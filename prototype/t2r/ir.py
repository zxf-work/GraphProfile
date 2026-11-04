"""Graph Analysis IR: a typed dataflow plan, its validator, cost model and executor."""
from __future__ import annotations

import math
import time
from typing import Any, Optional

from pydantic import BaseModel, Field, ValidationError

from .ops import CATALOGUE, Ctx
from .graphs import LoadedGraph


class Step(BaseModel):
    id: str
    op: str
    args: dict[str, Any] = Field(default_factory=dict)
    note: Optional[str] = None


class Answer(BaseModel):
    """Which step(s) hold the answer and how to present it."""
    step: str
    field: Optional[str] = None           # e.g. "count", "largest", "length"
    kind: str = "scalar"                  # scalar | list | table | bool | ranking


class Plan(BaseModel):
    interpretation: dict[str, Any] = Field(default_factory=dict)   # directed, weighted, use_weights, largest_component_only, node resolutions
    clarification: Optional[str] = None     # if set, the system asks instead of answering
    variants: list[str] = Field(default_factory=list)              # alternative measures computed and labelled
    steps: list[Step] = Field(default_factory=list)
    answer: Optional[Answer] = None
    rationale: Optional[str] = None


# cost model: rough operation counts from the profile card -----------------
def estimate_cost(op: str, args: dict[str, Any], n: int, m: int) -> float:
    spec = CATALOGUE[op]
    c = spec.cost
    tier = args.get("tier", "exact")
    k = float(args.get("k", 500) or 500)
    if c == "O(1)":
        return 1
    if c in ("O(n)", "O(m)", "O(n+m)", "O(deg)", "O(n log n)"):
        return n + m
    if c == "O(k*m)":
        return float(args.get("k", 2)) * m
    if c == "O(m^1.5)":
        return m ** 1.5
    if c == "O(k(n+m))":
        return 50 * (n + m)
    if c == "O(nm)":
        return (k * (n + m)) if tier == "approx" else n * m
    if c == "O(|S|(n+m))":
        return 50 * (n + m)
    return n * m


def validate(plan: Plan, lg: LoadedGraph, budget_ops: float = 2e9) -> list[str]:
    """Type-check and precondition-check a plan. Returns a list of problems (empty if valid)."""
    problems: list[str] = []
    types: dict[str, str] = {"G": "Graph"}
    n, m = lg.g.vcount(), lg.g.ecount()
    ids = set()
    for st in plan.steps:
        if st.id in ids:
            problems.append(f"duplicate step id {st.id}")
        ids.add(st.id)
        if st.op not in CATALOGUE:
            problems.append(f"step {st.id}: unknown op {st.op}")
            continue
        spec = CATALOGUE[st.op]
        for a in st.args:
            if a not in spec.params:
                problems.append(f"step {st.id}: op {st.op} has no parameter {a} (has {list(spec.params)})")
        for p, ty in spec.params.items():
            if p == "graph" and "graph" not in st.args:
                continue  # defaults to G
            base = ty.split(":")[0].split(" ")[0]
            v = st.args.get(p)
            if v is None:
                if base in ("Graph", "NodeMap", "Partition", "NodeSet") and p in ("map", "partition") or (p in ("node", "source", "target", "nodes") and base in ("NodeRef",)):
                    if p in ("map", "partition", "node", "source", "target"):
                        problems.append(f"step {st.id}: missing required argument {p}")
                continue
            if base in ("Graph", "NodeMap", "Partition", "NodeSet") and isinstance(v, str):
                if v not in types:
                    problems.append(f"step {st.id}: argument {p} refers to unknown step {v}")
                elif types[v] != base and not (base == "NodeSet" and types[v] in ("NodeSet",)):
                    problems.append(f"step {st.id}: argument {p} expects {base} but {v} is {types[v]}")
            if base == "NodeRef":
                r = lg.resolve(v)
                if r["matched"] is None:
                    problems.append(f"step {st.id}: node '{v}' not found (candidates {r['candidates']})")
        if spec.directed_only and not lg.g.is_directed():
            problems.append(f"step {st.id}: {st.op} requires a directed graph but the graph is undirected")
        if st.op == "components" and st.args.get("mode") == "strong" and not lg.g.is_directed():
            problems.append(f"step {st.id}: strong components are undefined on an undirected graph")
        if "tier" in st.args and st.args["tier"] not in spec.tiers:
            problems.append(f"step {st.id}: tier {st.args['tier']} not available for {st.op} (tiers {spec.tiers})")
        cost = estimate_cost(st.op, st.args, n, m)
        if cost > budget_ops and "approx" in spec.tiers and st.args.get("tier", "exact") == "exact":
            problems.append(f"step {st.id}: exact {st.op} estimated at {cost:.1e} operations exceeds budget {budget_ops:.0e}; use tier=approx")
        elif cost > budget_ops * 10:
            problems.append(f"step {st.id}: {st.op} estimated at {cost:.1e} operations, too expensive for this graph")
        types[st.id] = spec.returns
    if plan.answer is not None and plan.answer.step not in types:
        problems.append(f"answer refers to unknown step {plan.answer.step}")
    if plan.answer is None and plan.clarification is None:
        problems.append("plan has neither an answer step nor a clarification")
    return problems


def auto_tier(plan: Plan, lg: LoadedGraph, budget_ops: float = 2e9) -> list[str]:
    """Optimiser pass: downgrade exact ops to approx when over budget. Returns notes."""
    notes = []
    n, m = lg.g.vcount(), lg.g.ecount()
    for st in plan.steps:
        if st.op in CATALOGUE and "approx" in CATALOGUE[st.op].tiers and st.args.get("tier", "exact") == "exact":
            if estimate_cost(st.op, st.args, n, m) > budget_ops:
                st.args["tier"] = "approx"
                notes.append(f"{st.id}: {st.op} switched to approximate tier (graph too large for exact within budget)")
    return notes


def execute(plan: Plan, lg: LoadedGraph, seed: int = 1, cache: Optional[dict] = None) -> tuple[Ctx, list[dict[str, Any]]]:
    ctx = Ctx(lg, seed=seed)
    trace = []
    for st in plan.steps:
        spec = CATALOGUE[st.op]
        key = (lg.facts.sha1, st.op, repr(sorted(st.args.items())))
        t0 = time.time()
        cached = cache is not None and key in cache and all(not (isinstance(v, str) and v in ctx.values and v != "G") for v in st.args.values())
        if cached:
            val = cache[key]
        else:
            val = spec.fn(ctx, **st.args)
            if cache is not None and not cached and all(not (isinstance(v, str) and v in ctx.values and v != "G") for v in st.args.values()):
                cache[key] = val
        ctx.values[st.id] = val
        trace.append({"step": st.id, "op": st.op, "args": st.args, "seconds": round(time.time() - t0, 3), "cached": cached, "summary": summarise(val)})
    return ctx, trace


def summarise(val: Any, limit: int = 12) -> Any:
    """Compact, JSON-safe view of a value for the LLM and the trace."""
    import igraph as ig
    if isinstance(val, ig.Graph):
        return {"type": "Graph", "n": val.vcount(), "m": val.ecount()}
    if isinstance(val, float):
        return None if math.isnan(val) else (val if val != math.inf else "inf")
    if isinstance(val, dict):
        out = {}
        for k, v in val.items():
            if k == "membership" or k == "matrix":
                continue
            if isinstance(v, list) and len(v) > limit:
                out[k] = v[:limit] + [f"... ({len(v)} total)"]
            else:
                out[k] = summarise(v, limit) if isinstance(v, (dict, list, float)) else v
        if "membership" in val:
            out["_note"] = "membership omitted"
        if len(out) > 40:  # NodeMap
            items = list(val.items())
            return {"type": "NodeMap", "n": len(val), "sample": dict(items[:5])}
        return out
    if isinstance(val, list):
        if len(val) > limit:
            return [summarise(x, limit) for x in val[:limit]] + [f"... ({len(val)} total)"]
        return [summarise(x, limit) for x in val]
    return val


def extract_answer(plan: Plan, ctx: Ctx) -> Any:
    if plan.answer is None:
        return None
    val = ctx.values.get(plan.answer.step)
    if plan.answer.field and isinstance(val, dict):
        val = val.get(plan.answer.field)
    return summarise(val, limit=100) if not isinstance(val, (int, float, str, bool)) else (val if val != math.inf else "inf")
