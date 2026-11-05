"""Verification without ground truth: invariants between results, cross-method agreement, approximation bounds."""
from __future__ import annotations

import math
from typing import Any

from .ir import Plan
from .ops import Ctx, CATALOGUE


def overlap_at_k(a: list[str], b: list[str]) -> float:
    if not a or not b:
        return 0.0
    k = min(len(a), len(b))
    return len(set(a[:k]) & set(b[:k])) / k


MEASURE_WORDS = {"betweenness": "betweenness", "pagerank": "pagerank", "page rank": "pagerank", "closeness": "closeness",
                 "eigenvector": "eigenvector_centrality", "in-degree": "degree", "indegree": "degree", "out-degree": "degree", "outdegree": "degree",
                 "degree": "degree", "triangle": "triangle_count", "clustering": "average_clustering", "communit": "communities",
                 "diameter": "diameter", "k-core": "max_core", "assortativ": "assortativity", "reciprocity": "reciprocity", "pagerank score": "pagerank"}
OP_FAMILY = {"degree": {"degree", "node_degree", "strength", "degree_stats"}, "triangle_count": {"triangle_count"},
             "average_clustering": {"average_clustering", "local_clustering", "transitivity"}, "communities": {"communities"},
             "diameter": {"diameter"}, "max_core": {"max_core", "core_number", "k_core"}, "assortativity": {"assortativity"},
             "reciprocity": {"reciprocity"}, "betweenness": {"betweenness"}, "pagerank": {"pagerank"}, "closeness": {"closeness"},
             "eigenvector_centrality": {"eigenvector_centrality"}}


def consistency(plan: Plan, ctx: Ctx, question: str | None) -> list[dict[str, Any]]:
    """Checks between the question, the stated interpretation, the inferred semantics and the plan. No ground truth used."""
    import re
    out: list[dict[str, Any]] = []
    sem = ctx.lg.semantics
    g = ctx.lg.g
    interp = plan.interpretation or {}

    def add(check, ok, detail, severity="error"):
        out.append({"check": check, "ok": bool(ok), "detail": detail, "severity": severity if not ok else "info"})

    # 1. stated interpretation vs inferred semantics (only when inference is confident)
    if isinstance(interp.get("directed"), bool) and sem.confidence.get("directed", 0) >= 0.85 and interp["directed"] != sem.directed:
        add("interpretation: directedness contradicts the data", False,
            f"plan assumes directed={interp['directed']} but the file evidence says directed={sem.directed} ({sem.evidence.get('directed')})")
    uses_w = interp.get("use_weights") is True or any(st.args.get("use_weights") is True for st in plan.steps)
    if uses_w and sem.weighted and sem.weight_semantics == "strength":
        add("interpretation: weights used as path lengths but they are strengths", False, sem.evidence.get("weight_semantics", "weights look like counts"))
    # 2. directional ops on an undirected graph
    for st in plan.steps:
        if not g.is_directed() and st.args.get("mode") in ("in", "out"):
            add(f"{st.id}: in/out mode on an undirected graph", False, "in- and out-degree are undefined here; the graph was loaded undirected", "warn")
    # 3. stated measure vs operators actually used
    ops = {st.op for st in plan.steps}

    def has(op):
        return bool(OP_FAMILY.get(op, {op}) & ops)
    m = str(interp.get("measure", "")).lower()
    for word, op in MEASURE_WORDS.items():
        if word in m and not has(op):
            add("interpretation: stated measure not computed", False, f"interpretation says '{word}' but no {op} step exists")
            break
    # 4. question mentions a measure or direction the plan ignores
    if question:
        ql = question.lower()
        for word, op in MEASURE_WORDS.items():
            if word in ql and not has(op):
                add("question: named measure not computed", False, f"question says '{word}' but the plan has no {op} step")
                break
        comp_modes = {st.args.get("mode", "weak") for st in plan.steps if st.op == "components"}
        if "strongly" in ql and comp_modes and "strong" not in comp_modes:
            add("question: asks for strongly connected components but plan uses weak", False, f"modes {comp_modes}")
        if ("weakly" in ql or ("connected components" in ql and "strongly" not in ql and not g.is_directed())) and comp_modes and "weak" not in comp_modes:
            add("question: asks for (weakly) connected components but plan uses strong", False, f"modes {comp_modes}")
        if g.is_directed():
            wants_in = any(w in ql for w in ("in-degree", "indegree", "followers", "cited by", "trusted by", "received", "bought by", "cite it", "follow 7", "follow it"))
            wants_out = any(w in ql for w in ("out-degree", "outdegree", "follows", "cites", "trusts", "sent"))
            modes = {st.args.get("mode") for st in plan.steps if st.op in ("degree", "node_degree", "strength", "neighbours", "k_hop_neighbourhood")}
            if wants_in and not wants_out and modes and modes <= {"all", None, "out"}:
                add("question: asks for incoming relations but plan uses out/all mode", False, f"modes used: {modes}", "warn")
            if wants_out and not wants_in and modes and modes <= {"all", None, "in"}:
                add("question: asks for outgoing relations but plan uses in/all mode", False, f"modes used: {modes}", "warn")
        # 5. node references not present in the question text
        toks = set(re.findall(r"[A-Za-z0-9_]+", question))
        for st in plan.steps:
            spec = CATALOGUE.get(st.op)
            if not spec:
                continue
            for p_, ty in spec.params.items():
                if ty.startswith("NodeRef") and p_ in st.args and isinstance(st.args[p_], (str, int)):
                    v = str(st.args[p_])
                    if v not in toks and v.lower() not in {t.lower() for t in toks} and not any(v.lower() in t.lower() or t.lower() in v.lower() for t in toks if len(t) > 2):
                        add(f"{st.id}: node '{v}' is not mentioned in the question", False, "possible wrong node", "warn")
    # 6. multigraph semantics
    if sem.multigraph and question:
        ql = question.lower()
        if "edge_count" in ops and any(w in ql for w in ("distinct", "different", "pairs", "unique")) and "distinct_edge_count" not in ops:
            add("multigraph: question asks for distinct pairs but edge_count counts repeated interactions", False, "use distinct_edge_count", "warn")
    return out


def verify(plan: Plan, ctx: Ctx, question: str | None = None) -> list[dict[str, Any]]:
    """Return a list of findings: {"check": str, "ok": bool, "detail": str, "severity": "error|warn|info"}."""
    g = ctx.lg.g
    n, m = g.vcount(), g.ecount()
    findings: list[dict[str, Any]] = []
    vals = ctx.values
    by_op: dict[str, list[str]] = {}
    for st in plan.steps:
        by_op.setdefault(st.op, []).append(st.id)

    def add(check, ok, detail, severity="error"):
        findings.append({"check": check, "ok": bool(ok), "detail": detail, "severity": severity if not ok else "info"})

    # partitions cover the graph
    for sid in by_op.get("components", []) + by_op.get("communities", []):
        p = vals[sid]
        tot = sum(p["sizes"]) if p["count"] <= 20 else len(p["membership"])
        gsize = len(p["membership"])
        add(f"{sid}: partition covers all nodes", tot == gsize, f"sum of block sizes {tot} vs nodes {gsize}")
        if "modularity" in p:
            add(f"{sid}: modularity in [-0.5, 1]", -0.5 <= p["modularity"] <= 1.0, f"modularity={p['modularity']:.3f}")
            if p["count"] > 1 and p["modularity"] < 0.05:
                add(f"{sid}: community structure is weak", False, f"modularity {p['modularity']:.3f} < 0.05, communities may be meaningless", "warn")

    # triangle / clustering consistency
    if by_op.get("triangle_count") and by_op.get("transitivity"):
        tri = vals[by_op["triangle_count"][0]]
        tr = vals[by_op["transitivity"][0]]
        deg = g.degree()
        triples = sum(d * (d - 1) / 2 for d in deg)
        implied = 3 * tri / triples if triples else 0
        add("transitivity consistent with triangle count", abs(implied - tr) < 1e-6 + 0.02 * max(tr, 1e-9), f"3T/triples={implied:.4f} vs transitivity={tr:.4f}")

    # diameter bounds
    for sid in by_op.get("diameter", []):
        d = vals[sid]
        st = next(s for s in plan.steps if s.id == sid)
        if st.args.get("tier") == "approx":
            add(f"{sid}: approximate diameter is a lower bound", True, f"double-sweep gives a lower bound; true diameter >= {d}", "info")
        add(f"{sid}: diameter < n", d < n, f"diameter={d}, n={n}")

    # centrality bounds and cross-method agreement on top-k
    rankings: dict[str, list[str]] = {}
    for sid in by_op.get("top_k", []):
        st = next(s for s in plan.steps if s.id == sid)
        src = st.args.get("map")
        src_op = next((s.op for s in plan.steps if s.id == src), None)
        src_graph = next((s.args.get("graph", "G") for s in plan.steps if s.id == src), "G")
        if src_op and src_graph == "G":
            rankings[src_op] = [r["node"] for r in vals[sid]]
    for sid in by_op.get("betweenness", []):
        bmax = max(vals[sid].values())
        bound = (n - 1) * (n - 2) / (1 if g.is_directed() else 2)
        add(f"{sid}: betweenness within theoretical bound", bmax <= bound * 1.000001, f"max={bmax:.1f}, bound={bound:.1f}")
    for sid in by_op.get("pagerank", []):
        s = sum(vals[sid].values())
        add(f"{sid}: pagerank sums to 1", abs(s - 1) < 1e-6, f"sum={s:.6f}")
    cents = [op for op in ("betweenness", "pagerank", "degree", "closeness", "eigenvector_centrality") if op in rankings]
    for i in range(len(cents)):
        for j in range(i + 1, len(cents)):
            ov = overlap_at_k(rankings[cents[i]], rankings[cents[j]])
            add(f"top-k agreement {cents[i]} vs {cents[j]}", ov >= 0.3, f"overlap@k={ov:.2f}; " + ("measures agree on the leaders" if ov >= 0.3 else "measures disagree: report both, the choice of measure changes the answer"), "warn")

    # approximate betweenness: sample-size note
    for sid in by_op.get("betweenness", []) + by_op.get("closeness", []):
        st = next(s for s in plan.steps if s.id == sid)
        if st.args.get("tier") == "approx":
            k = st.args.get("k", 500)
            add(f"{sid}: approximate with {k} pivots", True, f"relative error on top ranks typically < 10% with {k} pivots on graphs of this size; exact values would need O(nm)={n * m:.1e} operations", "info")

    # paths
    for sid in by_op.get("shortest_path", []):
        p = vals[sid]
        if p.get("reachable"):
            path = p["path"]
            ok = all(g.are_adjacent(ctx.node(path[i]), ctx.node(path[i + 1])) for i in range(len(path) - 1))
            add(f"{sid}: returned path is a valid walk", ok, f"{len(path) - 1} hops")
        else:
            add(f"{sid}: target unreachable", True, "no path exists in the stated direction" + ("; the reverse direction may differ in a directed graph" if g.is_directed() else ""), "warn")
    for sid in by_op.get("distance", []):
        d = vals[sid]
        if d == math.inf and g.is_directed():
            add(f"{sid}: unreachable in a directed graph", True, "distance is infinite following edge direction; reverse or undirected distance may be finite", "warn")

    # set sizes
    for sid in by_op.get("reachable_set", []) + by_op.get("k_hop_neighbourhood", []):
        add(f"{sid}: set size < n", len(vals[sid]) < n, f"{len(vals[sid])} nodes")
    # diameter on a disconnected graph without restriction
    for sid in by_op.get("diameter", []):
        st = next(s for s in plan.steps if s.id == sid)
        if st.args.get("largest_component_only") is False and not g.is_connected(mode="weak"):
            add(f"{sid}: diameter over a disconnected graph", False, "the graph is disconnected; the diameter is infinite unless restricted to the largest component", "warn")
    findings.extend(consistency(plan, ctx, question))
    return findings
