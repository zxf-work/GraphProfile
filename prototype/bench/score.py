"""Deterministic checkers for each question family, plus plan and cost scoring."""
from __future__ import annotations

import json
import math
import re
from typing import Any


def _numbers_in(obj: Any) -> list[float]:
    """All numbers reachable in a JSON-like object (and numbers embedded in strings)."""
    out: list[float] = []
    def walk(x):
        if isinstance(x, bool):
            return
        if isinstance(x, (int, float)):
            if not (isinstance(x, float) and math.isnan(x)):
                out.append(float(x))
        elif isinstance(x, str):
            for m in re.findall(r"-?\d+(?:\.\d+)?(?:e-?\d+)?", x.replace(",", "")):
                try:
                    out.append(float(m))
                except ValueError:
                    pass
        elif isinstance(x, dict):
            for v in x.values():
                walk(v)
        elif isinstance(x, (list, tuple)):
            for v in x:
                walk(v)
    walk(obj)
    return out


def _close(a: float, b: float, tol: float) -> bool:
    if tol == 0:
        return abs(a - b) < 1e-9 or (abs(b) >= 1 and abs(a - b) <= 1e-6 * abs(b))
    return abs(a - b) <= tol * max(abs(b), 1e-12)


def _first_scalar(ans: Any) -> Any:
    """The scalar a system meant as its answer."""
    if isinstance(ans, dict):
        for k in ("value", "answer", "result", "count", "length", "distance", "hops", "diameter", "n", "k"):
            if k in ans and isinstance(ans[k], (int, float, str, bool)):
                return ans[k]
        if len(ans) == 1:
            return list(ans.values())[0]
    if isinstance(ans, list) and len(ans) == 1:
        return ans[0]
    return ans


def _is_inf(x: Any) -> bool:
    if x is None:
        return True
    if isinstance(x, dict):
        if x.get("reachable") is False or (x.get("path") == [] and x.get("length") in (None, "inf", "Infinity")):
            return True
        return _is_inf(_first_scalar({k: v for k, v in x.items() if v is not None}) if any(v is not None for v in x.values()) else None)
    if isinstance(x, float) and math.isinf(x):
        return True
    s = str(x).lower()
    return s in ("inf", "infinity", "unreachable", "none", "null", "no path", "false") or "unreach" in s or "no path" in s or "infinit" in s


def _node_list(ans: Any) -> list[str]:
    if isinstance(ans, dict):
        for k in ("nodes", "ranking", "top", "answer", "value", "list"):
            if k in ans and isinstance(ans[k], list):
                ans = ans[k]
                break
        else:
            if all(isinstance(v, (int, float)) for v in ans.values()) and len(ans) > 1:
                return [str(k) for k, _ in sorted(ans.items(), key=lambda kv: -kv[1])]
    if isinstance(ans, list):
        out = []
        for x in ans:
            if isinstance(x, dict):
                v = x.get("node", x.get("id", x.get("name", x.get("label"))))
                out.append(str(v) if v is not None else str(x))
            elif isinstance(x, (list, tuple)) and x:
                out.append(str(x[0]))
            else:
                out.append(str(x))
        return out
    if isinstance(ans, str):
        return [s.strip() for s in re.split(r"[,;\n]+", ans) if s.strip()]
    return []


def score_answer(q: dict, res: dict, graphs: dict | None = None) -> tuple[float, str]:
    """Return (score in [0,1], note)."""
    c = q["checker"]
    t = c["type"]
    ans = res.get("answer")
    clar = res.get("clarification")
    variants = res.get("variants") or []
    assumptions = res.get("assumptions") or {}
    if res.get("error") and ans is None and not clar:
        return 0.0, f"error: {res['error'][:120]}"

    if t == "scalar":
        gold = c["value"]
        gold = math.inf if gold == "inf" else gold
        tol = c.get("tol", 0)
        if gold == math.inf:
            v = _first_scalar(ans)
            if _is_inf(v):
                return 1.0, "unreachable correctly reported"
            if c.get("reverse") is not None and isinstance(v, (int, float)) and _close(float(v), c["reverse"], 0):
                return 0.0, "answered the reverse direction"
            return 0.0, f"expected unreachable, got {v}"
        v = _first_scalar(ans)
        if isinstance(v, bool):
            v = None
        if isinstance(v, (int, float)) and _close(float(v), float(gold), tol):
            if c.get("requires_assumption") and not assumptions.get(c["requires_assumption"]) and not clar:
                return 0.5, "value right but assumption not stated"
            return 1.0, "match"
        for alt in c.get("alt", []):
            if alt == "inf":
                if _is_inf(v) and (clar or assumptions.get("largest_component_only") is False):
                    return 1.0, "inf with disconnection stated"
            elif isinstance(v, (int, float)) and _close(float(v), float(alt), tol):
                return 1.0, "match (alternative form)"
        nums = _numbers_in(ans)
        if any(_close(x, float(gold), tol) for x in nums) and len(nums) <= 4:
            return 1.0, "match inside answer object"
        if clar and c.get("accept_clarification"):
            return 1.0, "clarification accepted"
        if any(isinstance(w, (int, float)) and isinstance(v, (int, float)) and _close(float(v), float(w), tol) for w in c.get("wrong", [])):
            return 0.0, f"known wrong interpretation ({v})"
        for var in variants:
            vn = _first_scalar(var.get("answer"))
            if isinstance(vn, (int, float)) and _close(float(vn), float(gold), tol):
                return 0.5, "correct only as a variant"
        return 0.0, f"got {json.dumps(ans, default=str)[:80]} expected {gold}"

    if t == "bool":
        v = _first_scalar(ans)
        if isinstance(v, str):
            v = v.strip().lower() in ("true", "yes")
        if isinstance(v, bool) and v == c["value"]:
            return 1.0, "match"
        return 0.0, f"got {v} expected {c['value']}"

    if t in ("facts", "facts_or_clarify"):
        tol = c.get("tol", 0.01)
        nums = _numbers_in(ans) + _numbers_in(variants) + ([] if t == "facts_or_clarify" else _numbers_in(res.get("explanation", "")))
        hits = {k: any(_close(x, float(v), tol if not float(v).is_integer() else 0) for x in nums) for k, v in c["facts"].items()}
        ok = sum(hits.values())
        for k, (lo, hi) in c.get("ranges", {}).items():
            if any(lo <= x <= hi for x in nums):
                ok += 1
            hits[k] = any(lo <= x <= hi for x in nums)
        total = len(hits)
        if t == "facts_or_clarify":
            if ok == total:
                return 1.0, "both values given"
            if clar:
                return 1.0, "clarification asked"
            v = _first_scalar(ans)
            if isinstance(v, (int, float)) and any(_close(float(v), float(s), 0) for s in c.get("single_ok", [])) and assumptions.get("mode") or (isinstance(v, (int, float)) and any(_close(float(v), float(s), 0) for s in c.get("single_ok", [])) and "total" in json.dumps(assumptions).lower()):
                return 0.5, "single value with mode stated"
            return ok / total * 0.5, f"partial {hits}"
        return ok / total, f"facts {hits}"

    if t == "ranking":
        got = _node_list(ans)
        gold = [str(x) for x in c["value"]]
        k = len(gold)
        ov = len(set(got[:k]) & set(gold)) / k if got else 0.0
        if ov >= c.get("min_overlap", 0.8):
            return 1.0, f"overlap@{k}={ov:.2f}"
        for var in variants:
            g2 = _node_list(var.get("answer"))
            ov2 = len(set(g2[:k]) & set(gold)) / k if g2 else 0
            if ov2 >= c.get("min_overlap", 0.8):
                return 0.5, f"correct only as a variant (overlap {ov2:.2f})"
        return 0.0, f"overlap@{k}={ov:.2f} got {got[:k]}"

    if t == "set":
        got = set(_node_list(ans))
        gold = set(str(x) for x in c["value"])
        j = len(got & gold) / len(got | gold) if (got | gold) else 0
        return (1.0 if j >= c.get("min_jaccard", 0.9) else 0.0), f"jaccard={j:.2f}"

    if t == "path":
        length = None
        if isinstance(ans, dict):
            length = ans.get("length", ans.get("hops", ans.get("distance")))
            path = ans.get("path")
        else:
            path = ans if isinstance(ans, list) else None
            if isinstance(ans, (int, float)):
                length = ans
        if path and isinstance(path, list) and length is None:
            length = len(path) - 1
        if length is not None and _close(float(length), float(c["length"]), 0):
            if path and graphs and c["dataset"] in graphs:
                lg = graphs[c["dataset"]]
                try:
                    idx = [lg.resolve(p)["matched"] for p in path]
                    valid = all(lg.g.are_adjacent(idx[i], idx[i + 1]) for i in range(len(idx) - 1)) and str(path[0]) == c["source"] and str(path[-1]) == c["target"]
                except Exception:
                    valid = False
                return (1.0 if valid else 0.5), ("valid path" if valid else "length right, path invalid")
            return 1.0, "length match"
        return 0.0, f"length {length} expected {c['length']}"

    if t == "choice":
        s = json.dumps(ans, default=str)
        toks = set(re.findall(r"[A-Za-z0-9]+", s))
        if str(c["value"]) in toks and (c.get("other") is None or str(c["other"]) not in toks or s.strip('"').strip().startswith(str(c["value"]))):
            return 1.0, "match"
        return 0.0, f"got {s[:60]}"

    if t == "pair_distance":
        s = json.dumps(ans, default=str)
        toks = set(re.findall(r"[A-Za-z0-9]+", s))
        nums = _numbers_in(ans)
        pair_ok = any(set(map(str, p)) <= toks for p in c["pairs"])
        dist_ok = any(_close(x, float(c["distance"]), 0) for x in nums)
        return (1.0 if pair_ok and dist_ok else 0.5 if (pair_ok or dist_ok) else 0.0), f"pair={pair_ok} dist={dist_ok}"

    if t == "clarify":
        got = _node_list(ans)[: c["k"]]
        acceptable = set(str(x) for x in c["acceptable"])
        in_acc = all(g in acceptable for g in got) and bool(got)
        measure_stated = bool(assumptions.get("measure"))
        if clar or len(variants) >= 1:
            if got and not in_acc:
                return 0.5, "asked or gave variants but primary answer outside acceptable set"
            return 1.0, "clarified or showed alternative measures"
        if in_acc and measure_stated:
            return 1.0, "single answer within acceptable set, measure stated"
        if in_acc:
            return 0.5, "single answer within acceptable set, measure not stated"
        return 0.0, f"answer {got} outside acceptable {sorted(acceptable)}"

    return 0.0, f"unknown checker {t}"


def score_plan(q: dict, res: dict) -> tuple[float | None, str]:
    """Plan correctness: expected ops present and expected assumptions matched. None when not applicable."""
    if res.get("system") == "codegen":
        return None, "n/a (opaque code)"
    ops = set(res.get("ops_used") or [])
    exp = q.get("expected_ops") or []
    ops_ok = any(set(alt) <= ops for alt in exp) if exp else True
    assum = res.get("assumptions") or {}
    exp_a = q.get("expected_assumptions") or {}
    a_ok = True
    for k, v in exp_a.items():
        if k == "mode":
            s = json.dumps(assum).lower() + " " + json.dumps(res.get("plan"), default=str).lower()
            if f'"{v}"' not in s and f"'{v}'" not in s and f"{v}-degree" not in s and f"{v}_degree" not in s and f"mode={v}" not in s:
                a_ok = False
        elif k in assum and isinstance(assum[k], bool):
            if assum[k] != v:
                a_ok = False
    if res.get("clarification") and not ops:
        return 1.0 if q["family"] in ("clarification", "interpretation") else 0.5, "clarified instead of planning"
    sc = (0.5 if ops_ok else 0.0) + (0.5 if a_ok else 0.0)
    return sc, f"ops_ok={ops_ok} assumptions_ok={a_ok}"
