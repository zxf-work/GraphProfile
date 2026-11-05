"""Hard interpretation set: graphs whose semantics must be read from the data, questions without descriptions.

Writes bench/data_hard/*.{csv,txt}, bench/datasets_hard.json and bench/questions_hard.json.
"""
from __future__ import annotations

import csv
import json
import math
import os
import random
import sys

import igraph as ig

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
from t2r.graphs import load_graph  # noqa: E402
from t2r.ops import CATALOGUE, Ctx  # noqa: E402

D = os.path.join(HERE, "data_hard")
os.makedirs(D, exist_ok=True)
random.seed(11)

DS: dict[str, dict] = {}
Q: list[dict] = []


def add(qid, family, ds, question, checker, expected_ops, expected_assumptions=None):
    Q.append({"id": qid, "family": family, "dataset": ds, "file": DS[ds]["path"], "description": None,
              "question": question, "checker": checker, "expected_ops": expected_ops, "expected_assumptions": expected_assumptions or {}})


def G(name):
    d = DS[name]
    return load_graph(name, d["path"], "", override={k: d[k] for k in ("directed", "weighted", "multigraph") if k in d})


def run(name, op, **args):
    return CATALOGUE[op].fn(Ctx(G(name)), **args)


def topk(name, op, k, **args):
    m = run(name, op, **args)
    return [n for n, _ in sorted(m.items(), key=lambda kv: kv[1], reverse=True)[:k]]


# ------------------------------------------------------------------ 1. roads: weights are distances
pts = {f"J{i}": (random.random() * 100, random.random() * 100) for i in range(300)}
g = ig.Graph.Lattice([15, 20], circular=False)
names = list(pts)
rows = []
for e in g.es:
    a, b = names[e.source], names[e.target]
    (x1, y1), (x2, y2) = pts[a], pts[b]
    rows.append((a, b, round(math.hypot(x1 - x2, y1 - y2) / 10 + 0.5, 2)))
# add a few long shortcuts so hop-count and distance disagree
for _ in range(40):
    a, b = random.sample(names, 2)
    rows.append((a, b, round(random.uniform(20, 40), 2)))
with open(os.path.join(D, "roads.csv"), "w", newline="") as f:
    w = csv.writer(f); w.writerow(["from_junction", "to_junction", "distance_km"]); w.writerows(rows)
DS["roads"] = {"path": os.path.join(D, "roads.csv"), "directed": False, "weighted": True, "weight_semantics": "distance"}

# ------------------------------------------------------------------ 2. similarity: fractional weights under 'weight'
g = ig.Graph.Erdos_Renyi(n=120, m=600)
with open(os.path.join(D, "similarity.csv"), "w", newline="") as f:
    w = csv.writer(f); w.writerow(["source", "target", "weight"])
    for e in g.es:
        w.writerow([f"P{e.source}", f"P{e.target}", round(random.uniform(0.05, 0.99), 3)])
DS["similarity"] = {"path": os.path.join(D, "similarity.csv"), "directed": False, "weighted": True, "weight_semantics": None}

# ------------------------------------------------------------------ 3. emails: multigraph events, directed, epoch timestamps
people = [f"E{i}" for i in range(400)]
base = ig.Graph.Barabasi(n=400, m=3, directed=True)
ev = []
t0 = 1_600_000_000
for e in base.es:
    k = max(1, int(random.expovariate(0.4)))
    for _ in range(k):
        ev.append((people[e.source], people[e.target], t0 + random.randrange(0, 30_000_000)))
random.shuffle(ev)
with open(os.path.join(D, "emails.csv"), "w", newline="") as f:
    w = csv.writer(f); w.writerow(["sender", "recipient", "timestamp"]); w.writerows(ev)
DS["emails"] = {"path": os.path.join(D, "emails.csv"), "directed": True, "weighted": False, "multigraph": True}

# ------------------------------------------------------------------ 4. indexed edges: first column is a row number, no header
kz = ig.Graph.Famous("Zachary")
with open(os.path.join(D, "indexed_edges.txt"), "w") as f:
    for i, e in enumerate(kz.es):
        f.write(f"{i} {e.source} {e.target}\n")
DS["indexed"] = {"path": os.path.join(D, "indexed_edges.txt"), "directed": False, "weighted": False}

# ------------------------------------------------------------------ 5. purchases: bipartite user-item
users = [f"U{i}" for i in range(200)]; items = [f"I{i}" for i in range(80)]
pur = set()
for u in users:
    for it in random.sample(items, random.randint(2, 9)):
        pur.add((u, it))
with open(os.path.join(D, "purchases.csv"), "w", newline="") as f:
    w = csv.writer(f); w.writerow(["customer", "product"]); w.writerows(sorted(pur))
DS["purchases"] = {"path": os.path.join(D, "purchases.csv"), "directed": False, "weighted": False}

# ------------------------------------------------------------------ 6. follows: directed, partial reciprocity, no header
fb = ig.Graph.Barabasi(n=5000, m=4, directed=True)
edges = set((e.source, e.target) for e in fb.es)
for (a, b) in random.sample(sorted(edges), len(edges) // 5):
    edges.add((b, a))
with open(os.path.join(D, "follows.txt"), "w") as f:
    for a, b in sorted(edges):
        f.write(f"{a}\t{b}\n")
DS["follows"] = {"path": os.path.join(D, "follows.txt"), "directed": True, "weighted": False}

# ------------------------------------------------------------------ 7. trust scores: integer strengths 1-10
tg = ig.Graph.Barabasi(n=600, m=3, directed=True)
with open(os.path.join(D, "trust.csv"), "w", newline="") as f:
    w = csv.writer(f); w.writerow(["source", "target", "trust"])
    for e in tg.es:
        w.writerow([f"T{e.source}", f"T{e.target}", random.randint(1, 10)])
DS["trust"] = {"path": os.path.join(D, "trust.csv"), "directed": True, "weighted": True, "weight_semantics": "strength"}

# ------------------------------------------------------------------ 8. citations: directed with semantic column names
cg = ig.Graph.Barabasi(n=2000, m=5, directed=True)
with open(os.path.join(D, "citations.csv"), "w", newline="") as f:
    w = csv.writer(f); w.writerow(["citing", "cited"])
    for e in cg.es:
        w.writerow([f"paper{e.source}", f"paper{e.target}"])
DS["citations"] = {"path": os.path.join(D, "citations.csv"), "directed": True, "weighted": False}

# ------------------------------------------------------------------ 9. both-ways undirected, no header (facebook stored twice)
fbp = "/home/user/GraphProfile/Graph Data/facebook_combined.txt"
with open(fbp) as fi, open(os.path.join(D, "friends_bothways.txt"), "w") as fo:
    for line in fi:
        a, b = line.split()
        fo.write(f"{a} {b}\n{b} {a}\n")
DS["bothways"] = {"path": os.path.join(D, "friends_bothways.txt"), "directed": False, "weighted": False}

json.dump(DS, open(os.path.join(HERE, "datasets_hard.json"), "w"), indent=1)

# ================================================================== questions
# roads
rg = G("roads")
sp_w = run("roads", "shortest_path", source="J0", target="J299", use_weights=True)
sp_h = run("roads", "shortest_path", source="J0", target="J299", use_weights=False)
add("H1", "interpretation", "roads", "What is the shortest route from J0 to J299, and how long is it?",
    {"type": "scalar", "value": sp_w["length"], "tol": 0.01, "wrong": [sp_h["length"]]}, [["shortest_path"]], {"use_weights": True})
_bw = run("roads", "betweenness", use_weights=True)
add("H2", "interpretation", "roads", "Which junction is the most central in terms of lying on the most shortest routes?",
    {"type": "clarify", "acceptable": sorted({topk("roads", "betweenness", 1)[0], max(_bw, key=_bw.get)}), "k": 1}, [["betweenness"]])
add("H3", "metric", "roads", "How many junctions and road segments are there?",
    {"type": "facts", "facts": {"n": rg.g.vcount(), "m": rg.g.ecount()}}, [["node_count", "edge_count"]])

# similarity
sg = G("similarity")
d_hop = run("similarity", "distance", source="P0", target="P7")
add("H4", "clarification", "similarity", "What is the shortest path length between P0 and P7?",
    {"type": "scalar", "value": d_hop, "tol": 0, "accept_clarification": True, "requires_assumption": "use_weights"}, [["shortest_path"], ["distance"]])
add("H5", "metric", "similarity", "What is the average degree?",
    {"type": "scalar", "value": 2 * sg.g.ecount() / sg.g.vcount(), "tol": 0.01}, [["degree_stats"], ["degree", "map_stats"], ["edge_count", "node_count"]])

# emails (multigraph, directed)
eg = G("emails")
pairs = run("emails", "distinct_edge_count")
add("H6", "interpretation", "emails", "How many emails are in this dataset, and between how many distinct sender-recipient pairs?",
    {"type": "facts", "facts": {"emails": eg.g.ecount(), "pairs": pairs}}, [["edge_count", "distinct_edge_count"], ["edge_count"]], {"multigraph": True})
# pair with most emails
from collections import Counter
cnt = Counter((eg.g.vs[e.source]["name"], eg.g.vs[e.target]["name"]) for e in eg.g.es)
(bs, bt), bn = cnt.most_common(1)[0]
add("H7", "interpretation", "emails", f"How many emails did {bs} send to {bt}?",
    {"type": "scalar", "value": bn, "tol": 0, "wrong": [1]}, [["edge_multiplicity"]], {"multigraph": True, "directed": True})
sent = Counter(eg.g.vs[e.source]["name"] for e in eg.g.es)
top_sender = sent.most_common(1)[0][0]
out_distinct = Counter();
for (a, b) in cnt: out_distinct[a] += 1
top_contacts = out_distinct.most_common(1)[0][0]
add("H8", "interpretation", "emails", "Who sent the most emails?",
    {"type": "choice", "value": top_sender, "other": top_contacts if top_contacts != top_sender else None}, [["strength"], ["degree"]], {"directed": True})
rec = Counter(eg.g.vs[e.target]["name"] for e in eg.g.es)
add("H9", "interpretation", "emails", "Who received emails from the largest number of different people?",
    {"type": "choice", "value": topk("emails", "degree", 1, mode="in")[0] if False else max(set(b for _, b in cnt), key=lambda x: sum(1 for (a, b) in cnt if b == x))}, [["degree"], ["simplify", "degree"]], {"directed": True, "mode": "in"})

# indexed
ig_ = G("indexed")
add("H10", "interpretation", "indexed", "How many nodes and edges does this graph have?",
    {"type": "facts", "facts": {"n": 34, "m": 78}}, [["node_count", "edge_count"]])
add("H11", "interpretation", "indexed", "Which node has the highest degree?",
    {"type": "choice", "value": topk("indexed", "degree", 1)[0]}, [["degree", "top_k"], ["degree", "argmax"]])

# purchases (bipartite)
pg = G("purchases")
item_pop = Counter(it for _, it in pur)
add("H12", "interpretation", "purchases", "Which product was bought by the most customers?",
    {"type": "choice", "value": item_pop.most_common(1)[0][0]}, [["degree"]])
# projection: pair of customers sharing most products
cust_items = {}
for u, it in pur:
    cust_items.setdefault(u, set()).add(it)
best = (0, None)
ul = sorted(cust_items)
for i in range(len(ul)):
    for j in range(i + 1, len(ul)):
        c = len(cust_items[ul[i]] & cust_items[ul[j]])
        if c > best[0]:
            best = (c, (ul[i], ul[j]))
add("H13", "interpretation", "purchases", "Which two customers have the most products in common, and how many?",
    {"type": "pair_distance", "distance": best[0], "pairs": [list(best[1])]}, [["project_bipartite"], ["neighbours"]])
add("H14", "metric", "purchases", "Is this a bipartite graph?", {"type": "bool", "value": True}, [["is_bipartite"]])
_ctx = Ctx(G("purchases")); _ctx.values["proj"] = CATALOGUE["project_bipartite"].fn(_ctx, side="source")
_pc = CATALOGUE["communities"].fn(_ctx, graph="proj")
add("H15", "interpretation", "purchases", "How many communities of customers are there, if you connect customers who bought the same product?",
    {"type": "scalar", "value": _pc["count"], "tol": 0.5, "accept_clarification": True, "wrong": [run("purchases", "communities")["count"]]}, [["project_bipartite", "communities"], ["communities"]])

# follows (directed, partial reciprocity, no header)
fg = G("follows")
add("H16", "interpretation", "follows", "How many edges does this graph have?",
    {"type": "scalar", "value": fg.g.ecount(), "tol": 0, "wrong": [run("follows", "distinct_edge_count") if False else len(set(frozenset(e) for e in edges))], "accept_clarification": True}, [["edge_count"]], {"directed": True})
add("H17", "interpretation", "follows", "How many accounts does 7 follow, and how many follow 7?",
    {"type": "facts", "facts": {"out": run("follows", "node_degree", node="7", mode="out"), "in": run("follows", "node_degree", node="7", mode="in")}}, [["node_degree"], ["degree"]], {"directed": True})
r1 = run("follows", "distance", source="4999", target="0"); r2 = run("follows", "distance", source="0", target="4999")
add("H18", "interpretation", "follows", "Can account 0 reach account 4999 by following follow-links?",
    {"type": "bool", "value": r2 != math.inf}, [["is_reachable"], ["distance"], ["reachable_set"]], {"directed": True})
add("H19", "interpretation", "follows", "What fraction of follow relationships are mutual?",
    {"type": "scalar", "value": fg.g.reciprocity(), "tol": 0.02}, [["reciprocity"]], {"directed": True})
add("H20", "topk", "follows", "Which 5 accounts have the most followers?",
    {"type": "ranking", "value": topk("follows", "degree", 5, mode="in"), "min_overlap": 0.8}, [["degree", "top_k"]], {"directed": True, "mode": "in"})

# trust
tg_ = G("trust")
d_t = run("trust", "distance", source="T5", target="T0")
add("H21", "interpretation", "trust", "How many steps does it take to get from T5 to T0 through trust links?",
    {"type": "scalar", "value": d_t, "tol": 0, "accept_clarification": True}, [["distance"], ["shortest_path"]], {"use_weights": False, "directed": True})
truster = max(tg_.g.vs, key=lambda v: v.outdegree())["name"]
strongest = max(((tg_.g.vs[e.target]["name"], e["weight"]) for e in tg_.g.es if tg_.g.vs[e.source]["name"] == truster), key=lambda x: x[1])
add("H22", "interpretation", "trust", f"Whom does {truster} trust the most?",
    {"type": "choice", "value": strongest[0]}, [["neighbours"], ["strength"]], {"directed": True, "weighted": True})

# citations
add("H23", "interpretation", "citations", "Which paper is cited the most?",
    {"type": "choice", "value": topk("citations", "degree", 1, mode="in")[0]}, [["degree"]], {"directed": True, "mode": "in"})
add("H24", "interpretation", "citations", "How many papers does paper1999 cite, and how many cite it?",
    {"type": "facts", "facts": {"cites": run("citations", "node_degree", node="paper1999", mode="out"), "cited_by": run("citations", "node_degree", node="paper1999", mode="in")}}, [["node_degree"], ["degree"]], {"directed": True})
add("H25", "interpretation", "citations", "How many papers are cited by nothing?",
    {"type": "scalar", "value": sum(1 for d in cg.indegree() if d == 0), "tol": 0}, [["degree", "filter_nodes", "set_size"], ["degree"]], {"directed": True, "mode": "in"})

# both ways
bg = G("bothways")
add("H26", "interpretation", "bothways", "How many friendships are in this network?",
    {"type": "scalar", "value": 88234, "tol": 0, "wrong": [176468], "accept_clarification": True}, [["edge_count"]], {"directed": False})
add("H27", "interpretation", "bothways", "What is the average number of friends per person?",
    {"type": "scalar", "value": 2 * 88234 / 4039, "tol": 0.01, "wrong": [4 * 88234 / 4039]}, [["degree_stats"], ["degree", "map_stats"]], {"directed": False})

json.dump(Q, open(os.path.join(HERE, "questions_hard.json"), "w"), indent=1, default=lambda x: "inf" if x == math.inf else str(x))
print("wrote", len(Q), "hard questions;", len(DS), "datasets")
for q in Q:
    print(q["id"], q["dataset"], "|", q["question"][:60], "|", json.dumps(q["checker"], default=str)[:90])
