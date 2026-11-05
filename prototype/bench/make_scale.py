"""Scale tier: questions over synthetic power-law graphs with 10M, 12M (directed) and 50M edges.

Ground truth: exact where feasible (counts, degrees, PageRank, components, triangles, cores),
otherwise a high-effort approximation with a tolerance (diameter from many double sweeps, betweenness with
many pivots). Descriptions are given: this tier tests cost tiering, not interpretation.
"""
from __future__ import annotations

import json
import math
import os
import random
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
from t2r.graphs import load_graph  # noqa: E402
from t2r.ops import CATALOGUE, Ctx  # noqa: E402

B = os.path.join(HERE, "big")
DS = {
    "ba_10m": {"path": os.path.join(B, "ba_10m.txt"), "directed": False, "weighted": False, "skip_in_scoring": True},
    "ba_12m_directed": {"path": os.path.join(B, "ba_12m_directed.txt"), "directed": True, "weighted": False, "skip_in_scoring": True},
    "ba_50m": {"path": os.path.join(B, "ba_50m.txt"), "directed": False, "weighted": False, "skip_in_scoring": True},
}
DESC = {
    "ba_10m": "Synthetic undirected preferential-attachment network, 2M nodes, 10M edges, each edge listed once",
    "ba_12m_directed": "Synthetic directed preferential-attachment network, 3M nodes, 12M edges; a row 'u v' is an arc from u to v",
    "ba_50m": "Synthetic undirected preferential-attachment network, 10M nodes, 50M edges, each edge listed once",
}
json.dump(DS, open(os.path.join(HERE, "datasets_scale.json"), "w"), indent=1)
Q: list[dict] = []


def add(qid, family, ds, question, checker, expected_ops, expected_assumptions=None):
    Q.append({"id": qid, "family": family, "dataset": ds, "file": DS[ds]["path"], "description": DESC[ds],
              "question": question, "checker": checker, "expected_ops": expected_ops, "expected_assumptions": expected_assumptions or {}})


def timed(label, fn):
    t = time.time(); v = fn(); print(f"  {label}: {time.time() - t:.0f}s"); return v


# ---------------------------------------------------------------- 10M undirected
t = time.time(); lg = load_graph("ba_10m", DS["ba_10m"]["path"], "", override={"directed": False, "weighted": False}); print("loaded 10m", round(time.time() - t))
g = lg.g; ctx = Ctx(lg)
add("X1", "metric", "ba_10m", "How many nodes and edges are there?", {"type": "facts", "facts": {"n": g.vcount(), "m": g.ecount()}}, [["node_count", "edge_count"]])
deg = timed("degree", lambda: CATALOGUE["degree"].fn(ctx))
top10 = [n for n, _ in sorted(deg.items(), key=lambda kv: -kv[1])[:10]]
add("X2", "topk", "ba_10m", "Which 10 nodes have the highest degree?", {"type": "ranking", "value": top10, "min_overlap": 0.9}, [["degree", "top_k"]])
tri = timed("triangles", lambda: CATALOGUE["triangle_count"].fn(ctx))
add("X3", "metric", "ba_10m", "How many triangles does the graph contain?", {"type": "scalar", "value": tri, "tol": 0}, [["triangle_count"]])
pr = timed("pagerank", lambda: CATALOGUE["pagerank"].fn(ctx))
top_pr = [n for n, _ in sorted(pr.items(), key=lambda kv: -kv[1])[:10]]
add("X4", "topk", "ba_10m", "Who are the top 10 nodes by PageRank?", {"type": "ranking", "value": top_pr, "min_overlap": 0.8}, [["pagerank", "top_k"]])
diam = timed("diameter approx k=60", lambda: CATALOGUE["diameter"].fn(ctx, tier="approx", k=60))
add("X5", "metric", "ba_10m", "What is the diameter of the network (an estimate is acceptable)?", {"type": "scalar", "value": diam, "tol": 0.15}, [["diameter"]])
bc = timed("betweenness approx k=1500", lambda: CATALOGUE["betweenness"].fn(ctx, tier="approx", k=1500))
top_bc = [n for n, _ in sorted(bc.items(), key=lambda kv: -kv[1])[:10]]
add("X6", "topk", "ba_10m", "List the 10 nodes with the highest betweenness centrality (an approximation is acceptable).", {"type": "ranking", "value": top_bc, "min_overlap": 0.6}, [["betweenness", "top_k"]])
core = timed("max_core", lambda: CATALOGUE["max_core"].fn(ctx))
add("X7", "metric", "ba_10m", "What is the largest k for which a non-empty k-core exists?", {"type": "scalar", "value": core["k"], "tol": 0}, [["max_core"], ["core_number"]])
del lg, g, ctx, deg, pr, bc

# ---------------------------------------------------------------- 12M directed
t = time.time(); lg = load_graph("ba_12m_directed", DS["ba_12m_directed"]["path"], "", override={"directed": True, "weighted": False}); print("loaded 12m dir", round(time.time() - t))
g = lg.g; ctx = Ctx(lg)
indeg = timed("in-degree", lambda: CATALOGUE["degree"].fn(ctx, mode="in"))
top_in = [n for n, _ in sorted(indeg.items(), key=lambda kv: -kv[1])[:10]]
add("X8", "topk", "ba_12m_directed", "Which 10 nodes have the most incoming arcs?", {"type": "ranking", "value": top_in, "min_overlap": 0.9}, [["degree", "top_k"]], {"directed": True, "mode": "in"})
reach = timed("reachable from 2999999", lambda: len(CATALOGUE["reachable_set"].fn(ctx, source="2999999")))
add("X9", "interpretation", "ba_12m_directed", "How many nodes can node 2999999 reach by following arcs?", {"type": "scalar", "value": reach, "tol": 0}, [["reachable_set"], ["components"]], {"directed": True})
scc = timed("scc", lambda: CATALOGUE["components"].fn(ctx, mode="strong"))
add("X10", "metric", "ba_12m_directed", "How many strongly connected components are there?", {"type": "scalar", "value": scc["count"], "tol": 0}, [["components"]], {"directed": True, "mode": "strong"})
del lg, g, ctx, indeg, scc

# ---------------------------------------------------------------- 50M undirected
t = time.time(); lg = load_graph("ba_50m", DS["ba_50m"]["path"], "", override={"directed": False, "weighted": False}); print("loaded 50m", round(time.time() - t))
g = lg.g; ctx = Ctx(lg)
add("X11", "metric", "ba_50m", "How many nodes and edges are there, and is the graph connected?", {"type": "facts", "facts": {"n": g.vcount(), "m": g.ecount()}}, [["node_count", "edge_count"], ["is_connected"], ["components"]])
deg = timed("degree", lambda: CATALOGUE["degree"].fn(ctx))
top10 = [n for n, _ in sorted(deg.items(), key=lambda kv: -kv[1])[:10]]
add("X12", "topk", "ba_50m", "Which 10 nodes have the highest degree?", {"type": "ranking", "value": top10, "min_overlap": 0.9}, [["degree", "top_k"]])
del deg
acc = timed("avg clustering", lambda: CATALOGUE["average_clustering"].fn(ctx))
add("X13", "metric", "ba_50m", "What is the average clustering coefficient?", {"type": "scalar", "value": acc, "tol": 0.02}, [["average_clustering"]])
diam = timed("diameter approx k=30", lambda: CATALOGUE["diameter"].fn(ctx, tier="approx", k=30))
add("X14", "metric", "ba_50m", "Roughly what is the diameter of the network?", {"type": "scalar", "value": diam, "tol": 0.15}, [["diameter"]])
bc = timed("betweenness approx k=600", lambda: CATALOGUE["betweenness"].fn(ctx, tier="approx", k=600))
top_bc = [n for n, _ in sorted(bc.items(), key=lambda kv: -kv[1])[:10]]
add("X15", "topk", "ba_50m", "Which 10 nodes have the highest betweenness centrality? An approximation is fine.", {"type": "ranking", "value": top_bc, "min_overlap": 0.5}, [["betweenness", "top_k"]])
d0 = timed("distance", lambda: CATALOGUE["distance"].fn(ctx, source="0", target="9999999"))
add("X16", "path", "ba_50m", "How many hops separate node 0 and node 9999999?", {"type": "scalar", "value": d0, "tol": 0}, [["distance"], ["shortest_path"]])

json.dump(Q, open(os.path.join(HERE, "questions_scale.json"), "w"), indent=1, default=lambda x: "inf" if x == math.inf else str(x))
print("wrote", len(Q), "scale questions")
for q in Q:
    print(q["id"], q["dataset"], "|", q["question"][:60], "|", json.dumps(q["checker"], default=str)[:90])
