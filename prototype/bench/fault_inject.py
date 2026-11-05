"""Fault injection for the verifier (RQ3).

Takes the full system's correct plans on the base benchmark, applies one mutation at a time,
executes the mutated plan, and records (a) whether the answer changed (harmful mutation) and
(b) whether the verifier raised an error or warning. No LLM calls.

Mutations:
  flip_directed    reload the graph with the opposite directedness, keep the plan
  swap_measure     betweenness <-> pagerank <-> degree <-> closeness (the stated measure in the interpretation is kept)
  mode_flip        in <-> out on degree-type ops
  wrong_node       replace one NodeRef with another random node
  drop_lcc         diameter largest_component_only True -> False
  method_swap      components weak<->strong (directed only), communities leiden -> label_propagation
  use_weights      shortest_path use_weights False -> True on a weighted graph

Usage: python bench/fault_inject.py [results_dir] -> writes <results_dir>/fault_injection.json and .md
"""
from __future__ import annotations

import copy
import json
import os
import random
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, ROOT)
from t2r.graphs import load_graph  # noqa: E402
from t2r.ir import Plan, validate, execute, extract_answer  # noqa: E402
from t2r.verify import verify  # noqa: E402

random.seed(3)
RES = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, "results")
Q = {q["id"]: q for q in json.load(open(os.path.join(HERE, "questions.json")))}
DS = json.load(open(os.path.join(HERE, "datasets.json")))

SWAP = {"betweenness": "pagerank", "pagerank": "degree", "degree": "closeness", "closeness": "betweenness"}


def graph_for(q, flip=False):
    d = DS[q["dataset"]]
    path = os.path.normpath(os.path.join(ROOT, d["path"]))
    ov = {"directed": (not d["directed"]) if flip else d["directed"], "weighted": d["weighted"]}
    return load_graph(q["dataset"], path, q.get("description") or "", override=ov)


def mutations(plan: Plan, lg):
    out = []
    p = copy.deepcopy(plan); out.append(("flip_directed", p))
    # swap_measure
    for st in plan.steps:
        if st.op in SWAP:
            p = copy.deepcopy(plan); s2 = next(s for s in p.steps if s.id == st.id); s2.op = SWAP[st.op]
            s2.args = {k: v for k, v in s2.args.items() if k in ("graph", "mode") and not (SWAP[st.op] != "degree" and k == "mode")}
            out.append(("swap_measure", p)); break
    for st in plan.steps:
        if st.args.get("mode") in ("in", "out"):
            p = copy.deepcopy(plan); s2 = next(s for s in p.steps if s.id == st.id); s2.args["mode"] = "out" if st.args["mode"] == "in" else "in"
            out.append(("mode_flip", p)); break
    for st in plan.steps:
        for key in ("node", "source", "target"):
            if key in st.args and isinstance(st.args[key], (str, int)):
                p = copy.deepcopy(plan); s2 = next(s for s in p.steps if s.id == st.id)
                s2.args[key] = random.choice(lg.labels)
                out.append(("wrong_node", p)); break
        else:
            continue
        break
    for st in plan.steps:
        if st.op == "diameter":
            p = copy.deepcopy(plan); s2 = next(s for s in p.steps if s.id == st.id); s2.args["largest_component_only"] = False
            out.append(("drop_lcc", p)); break
    for st in plan.steps:
        if st.op == "components" and lg.g.is_directed():
            p = copy.deepcopy(plan); s2 = next(s for s in p.steps if s.id == st.id); s2.args["mode"] = "strong" if st.args.get("mode", "weak") == "weak" else "weak"
            out.append(("method_swap", p)); break
        if st.op == "communities":
            p = copy.deepcopy(plan); s2 = next(s for s in p.steps if s.id == st.id); s2.args["method"] = "label_propagation"
            out.append(("method_swap", p)); break
    for st in plan.steps:
        if st.op == "shortest_path" and "weight" in lg.g.es.attributes():
            p = copy.deepcopy(plan); s2 = next(s for s in p.steps if s.id == st.id); s2.args["use_weights"] = True
            out.append(("use_weights", p)); break
    return out


def main():
    rows = []
    for qid, q in Q.items():
        f = os.path.join(RES, "raw", "full", f"{qid}.json")
        if not os.path.exists(f):
            continue
        r = json.load(open(f))
        if not r.get("plan") or r.get("error") or r["plan"].get("clarification"):
            continue
        plan = Plan.model_validate(r["plan"])
        lg = graph_for(q)
        if validate(plan, lg):
            continue
        ctx0, _ = execute(plan, lg)
        base = json.dumps(extract_answer(plan, ctx0), default=str, sort_keys=True)
        base_f = verify(plan, ctx0, question=q["question"])
        base_bad = {x["check"] for x in base_f if not x["ok"]}
        for name, mp in mutations(plan, lg):
            lg2 = graph_for(q, flip=True) if name == "flip_directed" else lg
            probs = validate(mp, lg2)
            if probs:
                rows.append({"qid": qid, "mutation": name, "harmful": True, "caught": True, "stage": "validator", "detail": probs[0][:120]})
                continue
            try:
                ctx, _ = execute(mp, lg2)
            except Exception as e:  # noqa: BLE001
                rows.append({"qid": qid, "mutation": name, "harmful": True, "caught": True, "stage": "executor", "detail": f"{type(e).__name__}: {e}"[:120]})
                continue
            ans = json.dumps(extract_answer(mp, ctx), default=str, sort_keys=True)
            harmful = ans != base
            fs = verify(mp, ctx, question=q["question"])
            new_bad = [x for x in fs if not x["ok"] and x["check"] not in base_bad]
            rows.append({"qid": qid, "mutation": name, "harmful": harmful, "caught": bool(new_bad), "stage": "verifier" if new_bad else "none",
                         "detail": "; ".join(f"{x['severity']}: {x['check']}" for x in new_bad)[:160]})
    json.dump(rows, open(os.path.join(RES, "fault_injection.json"), "w"), indent=1)
    # summary
    from collections import defaultdict
    by = defaultdict(lambda: {"n": 0, "harmful": 0, "caught_harmful": 0, "false_alarm": 0})
    for r in rows:
        b = by[r["mutation"]]; b["n"] += 1
        if r["harmful"]:
            b["harmful"] += 1; b["caught_harmful"] += int(r["caught"])
        elif r["caught"]:
            b["false_alarm"] += 1
    tot = {"n": sum(b["n"] for b in by.values()), "harmful": sum(b["harmful"] for b in by.values()), "caught_harmful": sum(b["caught_harmful"] for b in by.values()), "false_alarm": sum(b["false_alarm"] for b in by.values())}
    md = ["# Fault injection: what the verifier catches\n", "| mutation | injected | changed the answer | caught (of harmful) | catch rate | flagged although harmless |", "|---|---|---|---|---|---|"]
    for k, b in sorted(by.items()):
        md.append(f"| {k} | {b['n']} | {b['harmful']} | {b['caught_harmful']} | {b['caught_harmful'] / b['harmful']:.0%} | {b['false_alarm']} |" if b["harmful"] else f"| {k} | {b['n']} | 0 | - | - | {b['false_alarm']} |")
    md.append(f"| ALL | {tot['n']} | {tot['harmful']} | {tot['caught_harmful']} | {tot['caught_harmful'] / max(tot['harmful'], 1):.0%} | {tot['false_alarm']} |")
    md += ["\n## Caught by stage\n"]
    st = defaultdict(int)
    for r in rows:
        if r["harmful"] and r["caught"]:
            st[r["stage"]] += 1
    md += [f"- {k}: {v}" for k, v in st.items()]
    md += ["\n## Missed harmful mutations\n"] + [f"- {r['qid']} {r['mutation']}" for r in rows if r["harmful"] and not r["caught"]]
    open(os.path.join(RES, "fault_injection.md"), "w").write("\n".join(md) + "\n")
    print("\n".join(md))


if __name__ == "__main__":
    main()
