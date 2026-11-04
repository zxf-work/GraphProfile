"""Generate the benchmark questions with deterministic ground truth.

Ground truth is computed by igraph on graphs loaded with the semantics declared in datasets.json
(not inferred), and spot-checked against NetworkX in check_gold.py.
Each question: id, family, dataset, description (None = withheld), question, checker spec,
expected_ops (list of alternative op sets), expected_assumptions.
"""
from __future__ import annotations

import json
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
from t2r.graphs import load_graph  # noqa: E402
from t2r.ops import CATALOGUE, Ctx  # noqa: E402

DS = json.load(open(os.path.join(HERE, "datasets.json")))


def path_of(name: str) -> str:
    p = DS[name]["path"]
    return os.path.normpath(os.path.join(HERE, "..", p))


DESC = {
    "facebook": "SNAP ego-Facebook friendship network, undirected, each edge listed once",
    "enron": "SNAP email-Enron communication network, undirected; the file lists each edge in both directions",
    "epinions": "SNAP soc-Epinions1 who-trusts-whom network; a row 'u v' means u trusts v",
    "lesmis": "Les Miserables character co-appearance network; the weight is the number of chapters the two characters share",
    "karate": "Zachary karate club friendship network",
}

_graphs: dict[str, object] = {}


def G(name: str):
    if name not in _graphs:
        d = DS[name]
        lg = load_graph(name, path_of(name), "", override={"directed": d["directed"], "weighted": d["weighted"]})
        _graphs[name] = lg
    return _graphs[name]


def run(name: str, op: str, **args):
    ctx = Ctx(G(name))
    return CATALOGUE[op].fn(ctx, **args)


def topk(name, op, k, **args):
    m = run(name, op, **args)
    return [n for n, _ in sorted(m.items(), key=lambda kv: kv[1], reverse=True)[:k]]


Q: list[dict] = []


def add(qid, family, ds, question, checker, expected_ops, expected_assumptions=None, description=True):
    Q.append({"id": qid, "family": family, "dataset": ds, "file": path_of(ds),
              "description": DESC[ds] if description else None, "question": question,
              "checker": checker, "expected_ops": expected_ops, "expected_assumptions": expected_assumptions or {}})


def main():
    # ---------------------------------------------------------------- profile
    fb = G("facebook").g; ep = G("epinions").g; lm = G("lesmis").g; ka = G("karate").g; en = G("enron").g
    add("P1", "profile", "facebook", "Give me a basic profile of this network.",
        {"type": "facts", "facts": {"n": fb.vcount(), "m": fb.ecount(), "components": 1, "max_degree": max(fb.degree())}},
        [["node_count", "edge_count"], ["degree_stats"], ["components"]])
    epc = run("epinions", "components", mode="weak")
    add("P2", "profile", "epinions", "Profile this graph: size, connectivity and degree distribution.",
        {"type": "facts", "facts": {"n": ep.vcount(), "m": ep.ecount(), "weak_components": epc["count"]}},
        [["components"]], {"directed": True})
    add("P3", "profile", "lesmis", "Summarise the structure of this network.",
        {"type": "facts", "facts": {"n": lm.vcount(), "m": lm.ecount(), "max_degree": max(lm.degree())}},
        [["node_count"], ["edge_count"], ["degree_stats"]])

    # ---------------------------------------------------------------- single metric
    add("S1", "metric", "facebook", "How many nodes and edges are there?",
        {"type": "facts", "facts": {"n": fb.vcount(), "m": fb.ecount()}}, [["node_count", "edge_count"]])
    add("S2", "metric", "facebook", "What is the diameter of the network?",
        {"type": "scalar", "value": run("facebook", "diameter"), "tol": 0}, [["diameter"]])
    add("S3", "metric", "facebook", "How many triangles does the graph contain?",
        {"type": "scalar", "value": run("facebook", "triangle_count"), "tol": 0}, [["triangle_count"]])
    add("S4", "metric", "facebook", "What is the average clustering coefficient?",
        {"type": "scalar", "value": run("facebook", "average_clustering"), "tol": 0.01}, [["average_clustering"]])
    scc = run("epinions", "components", mode="strong")
    add("S5", "metric", "epinions", "How many strongly connected components are there?",
        {"type": "scalar", "value": scc["count"], "tol": 0}, [["components"]], {"directed": True, "mode": "strong"})
    add("S6", "metric", "epinions", "What fraction of users is in the largest strongly connected component?",
        {"type": "scalar", "value": scc["largest"] / ep.vcount(), "tol": 0.01, "alt": [scc["largest"]]}, [["components"]], {"directed": True})
    add("S7", "metric", "karate", "What is the degree assortativity coefficient?",
        {"type": "scalar", "value": run("karate", "assortativity"), "tol": 0.01}, [["assortativity"]])
    add("S8", "metric", "enron", "What is the largest k for which the graph has a non-empty k-core?",
        {"type": "scalar", "value": run("enron", "max_core")["k"], "tol": 0}, [["max_core"], ["core_number"]])
    add("S9", "metric", "lesmis", "What is the global clustering coefficient (transitivity)?",
        {"type": "scalar", "value": run("lesmis", "transitivity"), "tol": 0.01}, [["transitivity"]])
    add("S10", "metric", "epinions", "What is the reciprocity of the trust relationships?",
        {"type": "scalar", "value": run("epinions", "reciprocity"), "tol": 0.02}, [["reciprocity"]], {"directed": True})
    add("S11", "metric", "facebook", "Is the network connected?",
        {"type": "bool", "value": True}, [["is_connected"], ["components"]])
    enc = run("enron", "components")
    add("S12", "metric", "enron", "How many connected components are there, and how large is the largest one?",
        {"type": "facts", "facts": {"count": enc["count"], "largest": enc["largest"]}}, [["components"]], {"directed": False})
    add("S13", "metric", "facebook", "What is the edge density?",
        {"type": "scalar", "value": fb.density(), "tol": 0.01}, [["density"]])
    add("S14", "metric", "lesmis", "How many triangles are there?",
        {"type": "scalar", "value": run("lesmis", "triangle_count"), "tol": 0}, [["triangle_count"]])

    # ---------------------------------------------------------------- top-k
    add("T1", "topk", "facebook", "Which 10 nodes have the highest degree?",
        {"type": "ranking", "value": topk("facebook", "degree", 10), "min_overlap": 0.9}, [["degree", "top_k"], ["degree_stats"]])
    add("T2", "topk", "facebook", "Who are the top 10 nodes by PageRank?",
        {"type": "ranking", "value": topk("facebook", "pagerank", 10), "min_overlap": 0.8}, [["pagerank", "top_k"]])
    add("T3", "topk", "facebook", "List the 10 nodes with the highest betweenness centrality.",
        {"type": "ranking", "value": topk("facebook", "betweenness", 10), "min_overlap": 0.7}, [["betweenness", "top_k"]])
    add("T4", "topk", "epinions", "Which 10 users are trusted by the most other users?",
        {"type": "ranking", "value": topk("epinions", "degree", 10, mode="in"), "min_overlap": 0.9}, [["degree", "top_k"]], {"directed": True, "mode": "in"})
    add("T5", "topk", "epinions", "Who are the top 10 users by PageRank?",
        {"type": "ranking", "value": topk("epinions", "pagerank", 10), "min_overlap": 0.8}, [["pagerank", "top_k"]], {"directed": True})
    add("T6", "topk", "lesmis", "Which five characters have the highest betweenness centrality?",
        {"type": "ranking", "value": topk("lesmis", "betweenness", 5), "min_overlap": 0.8}, [["betweenness", "top_k"]], {"use_weights": False})
    add("T7", "topk", "facebook", "Which 10 nodes have the highest closeness centrality?",
        {"type": "ranking", "value": topk("facebook", "closeness", 10), "min_overlap": 0.6}, [["closeness", "top_k"]])

    # ---------------------------------------------------------------- community
    cfb = run("facebook", "communities", method="leiden")
    add("C1", "community", "facebook", "How many communities are there, and what is the modularity of that partition?",
        {"type": "facts", "facts": {"modularity": cfb["modularity"]}, "tol": 0.04, "ranges": {"count": [cfb["count"] * 0.6, cfb["count"] * 1.6]}},
        [["communities"]])
    clm = run("lesmis", "communities", method="leiden")
    add("C2", "community", "lesmis", "Run community detection: how big is the largest community?",
        {"type": "scalar", "value": clm["largest"], "tol": 0.35}, [["communities"]])
    cka = run("karate", "communities", method="leiden")
    blk = run("karate", "block_of", partition=cka, node="0")
    add("C3", "community", "karate", "Detect communities; how many members are in the same community as member 0?",
        {"type": "scalar", "value": len(blk), "tol": 0.35}, [["communities", "block_of"], ["communities"]])
    cen = run("enron", "communities", method="leiden")
    add("C4", "community", "enron", "What modularity does the best community partition you can find achieve?",
        {"type": "scalar", "value": cen["modularity"], "tol": 0.08}, [["communities"]])

    # ---------------------------------------------------------------- paths
    sp = run("facebook", "shortest_path", source="0", target="4038")
    add("PA1", "path", "facebook", "What is the shortest path between node 0 and node 4038?",
        {"type": "path", "length": sp["length"], "dataset": "facebook", "source": "0", "target": "4038"}, [["shortest_path"], ["distance"]])
    d = run("lesmis", "distance", source="Gavroche", target="Cosette")
    add("PA2", "path", "lesmis", "How many steps separate Gavroche and Cosette?",
        {"type": "scalar", "value": d, "tol": 0}, [["distance"], ["shortest_path"]], {"use_weights": False})
    d1 = run("epinions", "distance", source="0", target="5000"); d2 = run("epinions", "distance", source="5000", target="0")
    add("PA3", "path", "epinions", "How many hops does it take to get from user 0 to user 5000 following trust edges?",
        {"type": "scalar", "value": d1, "tol": 0, "reverse": d2}, [["distance"], ["shortest_path"]], {"directed": True})
    r = run("epinions", "distance", source="100", target="7")
    add("PA4", "path", "epinions", "Can user 100 reach user 7 by following trust links?",
        {"type": "bool", "value": r != math.inf}, [["distance"], ["shortest_path"], ["reachable_set"]], {"directed": True})
    r2 = run("enron", "distance", source="0", target="36000")
    add("PA5", "path", "enron", "Is there a path between node 0 and node 36000?",
        {"type": "bool", "value": r2 != math.inf}, [["distance"], ["shortest_path"], ["components"], ["reachable_set"]])

    # ---------------------------------------------------------------- neighbourhood
    add("N1", "neighbourhood", "facebook", "How many nodes are within 2 hops of node 0?",
        {"type": "scalar", "value": len(run("facebook", "k_hop_neighbourhood", node="0", k=2)), "tol": 0}, [["k_hop_neighbourhood"]])
    add("N2", "neighbourhood", "epinions", "How many users does user 0 trust, and how many users trust user 0?",
        {"type": "facts", "facts": {"out": run("epinions", "node_degree", node="0", mode="out"), "in": run("epinions", "node_degree", node="0", mode="in")}},
        [["node_degree"], ["degree"], ["neighbours"]], {"directed": True})
    deg = run("facebook", "degree")
    nb = run("facebook", "neighbours", node="107")
    add("N3", "neighbourhood", "facebook", "How many of node 107's friends have more than 100 friends themselves?",
        {"type": "scalar", "value": sum(1 for x in nb if deg[x] > 100), "tol": 0}, [["neighbours", "filter_nodes"], ["neighbours", "degree"]])
    common = sorted(set(run("lesmis", "neighbours", node="Valjean")) & set(run("lesmis", "neighbours", node="Cosette")))
    add("N4", "neighbourhood", "lesmis", "Which characters appear together with both Valjean and Cosette?",
        {"type": "set", "value": common, "min_jaccard": 0.9}, [["neighbours"]])

    # ---------------------------------------------------------------- comparative (within one graph)
    bc = run("facebook", "betweenness")
    add("CM1", "comparative", "facebook", "Which of node 0 and node 107 has the higher betweenness centrality?",
        {"type": "choice", "value": "107" if bc["107"] > bc["0"] else "0", "other": "0" if bc["107"] > bc["0"] else "107"}, [["betweenness"]])
    core20 = run("facebook", "k_core", k=20)
    ctx = Ctx(G("facebook")); ctx.values["c"] = core20
    acc = CATALOGUE["average_clustering"].fn(ctx, graph="c"); acg = run("facebook", "average_clustering")
    add("CM2", "comparative", "facebook", "Is the average clustering coefficient of the 20-core higher than that of the whole graph?",
        {"type": "bool", "value": acc > acg, "facts": {"core": acc, "whole": acg}}, [["k_core", "average_clustering"]])

    # ---------------------------------------------------------------- compositional
    top10 = topk("facebook", "degree", 10)
    pd_ = run("facebook", "pairwise_distances", nodes=top10)
    mx = pd_["max_distance"]; pairs = [[top10[i], top10[j]] for i in range(10) for j in range(i + 1, 10) if pd_["matrix"][i][j] == mx]
    add("CO1", "compositional", "facebook", "Among the 10 highest-degree nodes, which two are farthest apart, and what is their distance?",
        {"type": "pair_distance", "distance": mx, "pairs": pairs}, [["degree", "top_k", "pairwise_distances"], ["degree", "top_k", "distance"]])
    lcc = run("enron", "largest_component")
    add("CO2", "compositional", "enron", "What is the average degree of the nodes in the largest connected component?",
        {"type": "scalar", "value": 2 * lcc.ecount() / lcc.vcount(), "tol": 0.01}, [["largest_component", "degree_stats"], ["largest_component", "degree", "map_stats"], ["components"]])
    ctx = Ctx(G("facebook")); ctx.values["c50"] = run("facebook", "k_core", k=50)
    add("CO3", "compositional", "facebook", "How many triangles are there inside the 50-core of the graph?",
        {"type": "scalar", "value": CATALOGUE["triangle_count"].fn(ctx, graph="c50"), "tol": 0}, [["k_core", "triangle_count"]])
    top20in = topk("epinions", "degree", 20, mode="in")
    sub = run("epinions", "induced_subgraph", nodes=top20in)
    add("CO4", "compositional", "epinions", "How many trust edges exist among the 20 users with the highest in-degree?",
        {"type": "scalar", "value": sub.ecount(), "tol": 0}, [["degree", "top_k", "induced_subgraph", "edge_count"]], {"directed": True, "mode": "in"})
    vn = run("lesmis", "neighbours", node="Valjean"); bl = run("lesmis", "betweenness")
    best = max(vn, key=lambda x: bl[x])
    add("CO5", "compositional", "lesmis", "Among the characters who appear directly with Valjean, who has the highest betweenness centrality in the whole network?",
        {"type": "choice", "value": best}, [["neighbours", "betweenness"], ["betweenness", "neighbours"]])

    # ---------------------------------------------------------------- interpretation
    add("I1", "interpretation", "enron", "How many edges does this network have?",
        {"type": "scalar", "value": en.ecount(), "tol": 0, "wrong": [G("enron").facts.n_rows]}, [["edge_count"]], {"directed": False})
    add("I2", "interpretation", "enron", "How many edges does this network have?",
        {"type": "scalar", "value": en.ecount(), "tol": 0, "wrong": [G("enron").facts.n_rows], "accept_clarification": True}, [["edge_count"]], {"directed": False}, description=False)
    reach = run("epinions", "reachable_set", source="1")
    add("I3", "interpretation", "epinions", "How many users can user 1 reach by following trust links?",
        {"type": "scalar", "value": len(reach), "tol": 0, "alt": [len(reach) + 1]}, [["reachable_set"], ["components"]], {"directed": True})
    add("I4", "interpretation", "epinions", "What is the in-degree of node 0?",
        {"type": "scalar", "value": run("epinions", "node_degree", node="0", mode="in"), "tol": 0, "accept_clarification": True}, [["node_degree"], ["degree"]], {"directed": True}, description=False)
    hops = run("lesmis", "distance", source="Valjean", target="Marius")
    add("I5", "interpretation", "lesmis", "What is the shortest path length from Valjean to Marius?",
        {"type": "scalar", "value": hops, "tol": 0, "accept_clarification": True}, [["distance"], ["shortest_path"]], {"use_weights": False})
    add("I6", "interpretation", "facebook", "What is the in-degree of node 0?",
        {"type": "scalar", "value": run("facebook", "node_degree", node="0"), "tol": 0, "accept_clarification": True}, [["node_degree"], ["degree"]], {"directed": False})
    add("I7", "interpretation", "karate", "How many edges does this network have?",
        {"type": "scalar", "value": ka.ecount(), "tol": 0, "accept_clarification": True}, [["edge_count"]], {"directed": False}, description=False)

    # ---------------------------------------------------------------- clarification
    def union_topk(name, k):
        s = set()
        for op in ("degree", "pagerank", "betweenness", "closeness", "eigenvector_centrality"):
            s |= set(topk(name, op, k))
        return sorted(s)
    add("CL1", "clarification", "karate", "Who are the 3 most important members?",
        {"type": "clarify", "acceptable": union_topk("karate", 3), "k": 3}, [["degree"], ["pagerank"], ["betweenness"], ["closeness"], ["eigenvector_centrality"]])
    dlcc = run("enron", "diameter", largest_component_only=True)
    add("CL2", "clarification", "enron", "What is the diameter?",
        {"type": "scalar", "value": dlcc, "tol": 0, "accept_clarification": True, "alt": ["inf"], "requires_assumption": "largest_component_only"}, [["diameter"]])
    add("CL3", "clarification", "epinions", "What is the degree of node 0?",
        {"type": "facts_or_clarify", "facts": {"in": run("epinions", "node_degree", node="0", mode="in"), "out": run("epinions", "node_degree", node="0", mode="out")},
         "single_ok": [run("epinions", "node_degree", node="0", mode="all")]}, [["node_degree"], ["degree"]], {"directed": True})
    add("CL4", "clarification", "lesmis", "Who is the most central character?",
        {"type": "clarify", "acceptable": union_topk("lesmis", 1), "k": 1}, [["degree"], ["pagerank"], ["betweenness"], ["closeness"], ["eigenvector_centrality"]])

    # ---------------------------------------------------------------- node resolution
    add("NR1", "resolution", "lesmis", "What is the degree of valjean?",
        {"type": "scalar", "value": run("lesmis", "node_degree", node="Valjean"), "tol": 0}, [["node_degree"], ["degree"]])
    add("NR2", "resolution", "lesmis", "How far is Marius from Cosete?",
        {"type": "scalar", "value": run("lesmis", "distance", source="Marius", target="Cosette"), "tol": 0}, [["distance"], ["shortest_path"]], {"use_weights": False})
    add("NR3", "resolution", "lesmis", "Who are the neighbours of Jean Valjean?",
        {"type": "set", "value": sorted(run("lesmis", "neighbours", node="Valjean")), "min_jaccard": 0.9}, [["neighbours"]])
    bg = bl["Gavroche"]; n = lm.vcount()
    add("NR4", "resolution", "lesmis", "What is the betweenness centrality of Gavroche?",
        {"type": "scalar", "value": bg, "tol": 0.01, "alt": [bg / ((n - 1) * (n - 2) / 2)]}, [["betweenness", "map_value"], ["betweenness"]])
    pr = run("lesmis", "pagerank")
    add("NR5", "resolution", "lesmis", "What is the PageRank score of Javer?",
        {"type": "scalar", "value": pr["Javert"], "tol": 0.02}, [["pagerank", "map_value"], ["pagerank"]])

    out = os.path.join(HERE, "questions.json")
    json.dump(Q, open(out, "w"), indent=1, default=lambda x: "inf" if x == math.inf else str(x))
    print(f"wrote {len(Q)} questions to {out}")
    from collections import Counter
    print(Counter(q["family"] for q in Q))


if __name__ == "__main__":
    main()
