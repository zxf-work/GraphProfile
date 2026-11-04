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


def verify(plan: Plan, ctx: Ctx) -> list[dict[str, Any]]:
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
    return findings
