"""Operator catalogue: typed graph-analysis operators bound to igraph.

Each operator has a signature (param -> type), an output type, preconditions,
a cost class and optional approximate tiers. The catalogue is the only thing the
planner is allowed to compose. Types: Graph, NodeSet, NodeMap, Partition, Path,
Scalar, Table, Bool.
"""
from __future__ import annotations

import math
import random
from dataclasses import dataclass, field
from typing import Any, Callable, Optional

import igraph as ig
import numpy as np

from .graphs import LoadedGraph


@dataclass
class OpSpec:
    name: str
    doc: str
    params: dict[str, str]                 # param -> type description
    returns: str
    cost: str                              # "O(n+m)", "O(nm)", "O(n^2)", "O(k(n+m))", "O(1)"
    tiers: list[str] = field(default_factory=lambda: ["exact"])
    preconditions: list[str] = field(default_factory=list)
    directed_only: bool = False
    fn: Optional[Callable] = None


CATALOGUE: dict[str, OpSpec] = {}


def op(name: str, doc: str, params: dict[str, str], returns: str, cost: str, tiers=None, preconditions=None, directed_only=False):
    def deco(fn):
        CATALOGUE[name] = OpSpec(name=name, doc=doc, params=params, returns=returns, cost=cost,
                                 tiers=tiers or ["exact"], preconditions=preconditions or [],
                                 directed_only=directed_only, fn=fn)
        return fn
    return deco


# --------------------------------------------------------------------------- helpers
class Ctx:
    """Execution context: the loaded graph plus a registry of intermediate values."""
    def __init__(self, lg: LoadedGraph, seed: int = 1):
        self.lg = lg
        self.seed = seed
        self.values: dict[str, Any] = {"G": lg.g}
        self.log: list[dict[str, Any]] = []

    def node(self, ref: Any) -> int:
        """Resolve a node reference (label, int id, or 'node:<idx>') to a vertex index."""
        if isinstance(ref, int) and not self.lg.semantics.labelled:
            r = self.lg.resolve(str(ref))
            if r["matched"] is None:
                raise ValueError(f"node {ref} not found")
            return r["matched"]
        r = self.lg.resolve(ref)
        if r["matched"] is None:
            raise ValueError(f"node '{ref}' not found; candidates: {r['candidates']}")
        return r["matched"]

    def label(self, idx: int) -> str:
        return self.lg.labels[idx]


def _vid(ctx: Ctx, g: ig.Graph, ref: Any) -> int:
    """Vertex index of a node reference *in graph g* (which may be a subgraph with its own indexing)."""
    if g is ctx.lg.g:
        return ctx.node(ref)
    label = ctx.label(ctx.node(ref))
    idx = g.vs.find(name=label).index if label in set(g.vs["name"]) else None
    if idx is None:
        raise ValueError(f"node '{ref}' is not in this subgraph")
    return idx


def _lab(g: ig.Graph, idx: int) -> str:
    return g.vs[idx]["name"]


def _nodeset(ctx: Ctx, x: Any) -> list[str]:
    """Coerce a NodeSet reference, a Table with a node column, or a literal list to a list of labels."""
    v = ctx.values[x] if isinstance(x, str) and x in ctx.values else x
    if isinstance(v, dict) and "membership" in v:
        raise ValueError("a Partition is not a NodeSet; use largest_block or block_of")
    if isinstance(v, list):
        return [str(e["node"]) if isinstance(e, dict) and "node" in e else str(e) for e in v]
    if isinstance(v, dict):
        return [str(k) for k in v.keys()]
    raise ValueError(f"not a NodeSet: {x}")


def _graph(ctx: Ctx, g: Any) -> ig.Graph:
    if isinstance(g, ig.Graph):
        return g
    if isinstance(g, str) and g in ctx.values:
        return ctx.values[g]
    raise ValueError(f"not a graph: {g}")


def _nodemap(ctx: Ctx, values: list[float], g: ig.Graph) -> dict[str, float]:
    names = g.vs["name"]
    return {names[i]: float(v) for i, v in enumerate(values)}


# --------------------------------------------------------------------------- basic
@op("node_count", "Number of nodes.", {"graph": "Graph"}, "Scalar", "O(1)")
def node_count(ctx, graph="G"):
    return _graph(ctx, graph).vcount()


@op("edge_count", "Number of edges (undirected edges counted once; directed arcs counted once).", {"graph": "Graph"}, "Scalar", "O(1)")
def edge_count(ctx, graph="G"):
    return _graph(ctx, graph).ecount()


@op("distinct_edge_count", "Number of distinct node pairs connected (collapses parallel edges in a multigraph).", {"graph": "Graph"}, "Scalar", "O(m)")
def distinct_edge_count(ctx, graph="G"):
    g = _graph(ctx, graph)
    return len(set((e.source, e.target) if g.is_directed() else (min(e.source, e.target), max(e.source, e.target)) for e in g.es))


@op("edge_multiplicity", "Number of parallel edges (repeated interactions) between two nodes in a multigraph; 0 if none.", {"graph": "Graph", "source": "NodeRef", "target": "NodeRef"}, "Scalar", "O(deg)")
def edge_multiplicity(ctx, source, target, graph="G"):
    g = _graph(ctx, graph)
    return g.count_multiple(g.get_eids([(_vid(ctx, g, source), _vid(ctx, g, target))], error=False)) [0] if g.get_eid(_vid(ctx, g, source), _vid(ctx, g, target), error=False) >= 0 else 0


@op("simplify", "Collapse parallel edges and self-loops into a simple graph (parallel edges summed into a 'weight' attribute).", {"graph": "Graph"}, "Graph", "O(m)")
def simplify(ctx, graph="G"):
    g = _graph(ctx, graph).copy()
    if "weight" not in g.es.attributes():
        g.es["weight"] = 1.0
    g.simplify(multiple=True, loops=True, combine_edges={"weight": "sum"})
    return g


@op("is_bipartite", "Whether the graph is bipartite (two node types with edges only between types).", {"graph": "Graph"}, "Bool", "O(n+m)")
def is_bipartite(ctx, graph="G"):
    return bool(_graph(ctx, graph).is_bipartite())


@op("project_bipartite", "One-mode projection of a bipartite graph onto the side containing `side_node` (or side 'source'/'target' of the file). Two nodes of that side are linked when they share a neighbour; edge weight = number of shared neighbours.",
    {"graph": "Graph", "side": "str: 'source'|'target'|a node id on the wanted side"}, "Graph", "O(sum deg^2)")
def project_bipartite(ctx, side="source", graph="G"):
    g = _graph(ctx, graph)
    ok, types = g.is_bipartite(return_types=True)
    if not ok:
        raise ValueError("graph is not bipartite")
    if side in ("source", "target"):
        wanted = {u for u, _, _ in _rows_cache(ctx)} if side == "source" else {v for _, v, _ in _rows_cache(ctx)}
        # the type that covers the wanted ids
        t_true = sum(1 for i, t in enumerate(types) if t and g.vs[i]["name"] in wanted)
        t_false = sum(1 for i, t in enumerate(types) if not t and g.vs[i]["name"] in wanted)
        want_type = t_true >= t_false
    else:
        want_type = types[_vid(ctx, g, side)]
    p1, p2 = g.bipartite_projection(types=types, multiplicity=True)
    proj = p2 if want_type else p1
    return proj


def _rows_cache(ctx):
    if not hasattr(ctx, "_rows"):
        from .graphs import read_raw
        ctx._rows = read_raw(ctx.lg.facts.path)[1]
    return ctx._rows


@op("density", "Edge density: m / possible edges.", {"graph": "Graph"}, "Scalar", "O(1)")
def density(ctx, graph="G"):
    return _graph(ctx, graph).density()


@op("degree_stats", "Degree summary: min, median, mean, max, and a 10-bin histogram. mode in {all,in,out}.",
    {"graph": "Graph", "mode": "str: all|in|out (default all)"}, "Table", "O(n+m)")
def degree_stats(ctx, graph="G", mode="all"):
    g = _graph(ctx, graph)
    d = np.array(g.degree(mode=mode))
    hist, edges = np.histogram(d, bins=10)
    return {"min": int(d.min()), "median": float(np.median(d)), "mean": float(d.mean()), "max": int(d.max()),
            "std": float(d.std()), "histogram": [{"range": [float(edges[i]), float(edges[i + 1])], "count": int(hist[i])} for i in range(len(hist))]}


@op("degree", "Degree of every node as a NodeMap. mode in {all,in,out}.", {"graph": "Graph", "mode": "str"}, "NodeMap", "O(n+m)")
def degree(ctx, graph="G", mode="all"):
    g = _graph(ctx, graph)
    return _nodemap(ctx, g.degree(mode=mode), g)


@op("strength", "Weighted degree (sum of edge weights, or number of parallel edges in a multigraph) per node as NodeMap.", {"graph": "Graph", "mode": "str"}, "NodeMap", "O(n+m)")
def strength(ctx, graph="G", mode="all"):
    g = _graph(ctx, graph)
    w = "weight" if "weight" in g.es.attributes() else None
    return _nodemap(ctx, g.strength(mode=mode, weights=w), g)


@op("node_degree", "Degree of one node. mode in {all,in,out}.", {"graph": "Graph", "node": "NodeRef", "mode": "str"}, "Scalar", "O(1)")
def node_degree(ctx, node, graph="G", mode="all"):
    g = _graph(ctx, graph)
    return g.degree(_vid(ctx, g, node), mode=mode)


@op("reciprocity", "Fraction of directed edges whose reverse also exists. Directed graphs only.",
    {"graph": "Graph"}, "Scalar", "O(m)", directed_only=True)
def reciprocity(ctx, graph="G"):
    return _graph(ctx, graph).reciprocity()


# --------------------------------------------------------------------------- components
@op("components", "Connected components. mode: weak (default) or strong (directed only). Returns count and sizes (descending).",
    {"graph": "Graph", "mode": "str: weak|strong"}, "Partition", "O(n+m)")
def components(ctx, graph="G", mode="weak"):
    g = _graph(ctx, graph)
    c = g.connected_components(mode=mode)
    sizes = sorted(c.sizes(), reverse=True)
    return {"count": len(c), "sizes": sizes[:20], "largest": sizes[0], "membership": c.membership, "_names": g.vs["name"], "_kind": "components", "_mode": mode}


@op("largest_component", "Induced subgraph of the largest (weakly) connected component.", {"graph": "Graph", "mode": "str: weak|strong"}, "Graph", "O(n+m)")
def largest_component(ctx, graph="G", mode="weak"):
    g = _graph(ctx, graph)
    c = g.connected_components(mode=mode)
    return c.giant()


@op("is_connected", "Whether the graph is (weakly) connected.", {"graph": "Graph"}, "Bool", "O(n+m)")
def is_connected(ctx, graph="G"):
    return _graph(ctx, graph).is_connected(mode="weak")


# --------------------------------------------------------------------------- distances
@op("diameter", "Longest shortest path. Infinite if disconnected unless largest_component_only. tier exact: all-pairs BFS O(nm); tier approx: double-sweep lower bound from k sources.",
    {"graph": "Graph", "largest_component_only": "bool (default true)", "tier": "exact|approx", "k": "int sources for approx (default 20)"},
    "Scalar", "O(nm)", tiers=["exact", "approx"])
def diameter(ctx, graph="G", largest_component_only=True, tier="exact", k=20):
    g = _graph(ctx, graph)
    if largest_component_only and not g.is_connected(mode="weak"):
        g = g.connected_components(mode="weak").giant()
    if tier == "exact":
        return int(g.diameter(directed=g.is_directed(), unconn=True))
    rng = random.Random(ctx.seed)
    best = 0
    for _ in range(k):
        s = rng.randrange(g.vcount())
        for _sweep in range(2):
            d = g.distances(source=s, mode="out")[0]
            far = max(range(len(d)), key=lambda i: d[i] if d[i] != math.inf else -1)
            best = max(best, int(d[far]) if d[far] != math.inf else best)
            s = far
    return best


@op("shortest_path", "Shortest path between two nodes (hop count, or weighted if use_weights and weights are distances). Returns length and node list.",
    {"graph": "Graph", "source": "NodeRef", "target": "NodeRef", "use_weights": "bool (default false)"}, "Path", "O(n+m)")
def shortest_path(ctx, source, target, graph="G", use_weights=False):
    g = _graph(ctx, graph)
    s, t = _vid(ctx, g, source), _vid(ctx, g, target)
    w = "weight" if (use_weights and "weight" in g.es.attributes()) else None
    paths = g.get_shortest_paths(s, to=t, weights=w, mode="out", output="vpath")
    p = paths[0]
    if not p:
        return {"length": math.inf, "path": [], "reachable": False}
    if w:
        length = sum(g.es[g.get_eid(p[i], p[i + 1])]["weight"] for i in range(len(p) - 1))
    else:
        length = len(p) - 1
    return {"length": length, "path": [_lab(g, i) for i in p], "reachable": True, "hops": len(p) - 1}


@op("distance", "Shortest-path distance (hops) from source to target; inf if unreachable.",
    {"graph": "Graph", "source": "NodeRef", "target": "NodeRef"}, "Scalar", "O(n+m)")
def distance(ctx, source, target, graph="G"):
    g = _graph(ctx, graph)
    d = g.distances(source=_vid(ctx, g, source), target=_vid(ctx, g, target), mode="out")[0][0]
    return d if d != math.inf else math.inf


@op("is_reachable", "Whether target can be reached from source (following edge direction in directed graphs).",
    {"graph": "Graph", "source": "NodeRef", "target": "NodeRef"}, "Bool", "O(n+m)")
def is_reachable(ctx, source, target, graph="G"):
    g = _graph(ctx, graph)
    return g.distances(source=_vid(ctx, g, source), target=_vid(ctx, g, target), mode="out")[0][0] != math.inf


@op("reachable_set", "All nodes reachable from source (following edge direction in directed graphs, within max_hops if given).",
    {"graph": "Graph", "source": "NodeRef", "max_hops": "int or null"}, "NodeSet", "O(n+m)")
def reachable_set(ctx, source, graph="G", max_hops=None):
    g = _graph(ctx, graph)
    s = _vid(ctx, g, source)
    order = max_hops if max_hops is not None else g.vcount()
    nb = g.neighborhood(s, order=order, mode="out")
    return [_lab(g, i) for i in nb if i != s]


@op("k_hop_neighbourhood", "Nodes within k hops of a node (excluding the node). mode in {all,in,out}.",
    {"graph": "Graph", "node": "NodeRef", "k": "int", "mode": "str"}, "NodeSet", "O(k*m)")
def k_hop_neighbourhood(ctx, node, k, graph="G", mode="all"):
    g = _graph(ctx, graph)
    s = _vid(ctx, g, node)
    nb = g.neighborhood(s, order=int(k), mode=mode)
    return [_lab(g, i) for i in nb if i != s]


@op("neighbours", "Direct neighbours of a node. mode in {all,in,out}.", {"graph": "Graph", "node": "NodeRef", "mode": "str"}, "NodeSet", "O(deg)")
def neighbours(ctx, node, graph="G", mode="all"):
    g = _graph(ctx, graph)
    return [_lab(g, i) for i in g.neighbors(_vid(ctx, g, node), mode=mode)]


# --------------------------------------------------------------------------- clustering
@op("triangle_count", "Total number of triangles in the graph (undirected interpretation).", {"graph": "Graph"}, "Scalar", "O(m^1.5)")
def triangle_count(ctx, graph="G"):
    g = _graph(ctx, graph)
    if g.is_directed():
        g = g.as_undirected(mode="collapse")
    if g.ecount() < 200_000:
        return int(len(g.cliques(min=3, max=3)))
    deg = np.array(g.degree(), dtype=float)
    lcc = np.nan_to_num(np.array(g.transitivity_local_undirected(mode="zero"), dtype=float))
    return int(round(float((lcc * deg * (deg - 1) / 2).sum() / 3)))


@op("average_clustering", "Average local clustering coefficient (nodes with degree<2 count as 0).", {"graph": "Graph"}, "Scalar", "O(m^1.5)")
def average_clustering(ctx, graph="G"):
    g = _graph(ctx, graph)
    if g.is_directed():
        g = g.as_undirected(mode="collapse")
    return g.transitivity_avglocal_undirected(mode="zero")


@op("transitivity", "Global clustering coefficient (3 x triangles / connected triples).", {"graph": "Graph"}, "Scalar", "O(m^1.5)")
def transitivity(ctx, graph="G"):
    g = _graph(ctx, graph)
    if g.is_directed():
        g = g.as_undirected(mode="collapse")
    return g.transitivity_undirected()


@op("local_clustering", "Local clustering coefficient per node as NodeMap.", {"graph": "Graph"}, "NodeMap", "O(m^1.5)")
def local_clustering(ctx, graph="G"):
    g = _graph(ctx, graph)
    u = g.as_undirected(mode="collapse") if g.is_directed() else g
    return _nodemap(ctx, u.transitivity_local_undirected(mode="zero"), g)


@op("assortativity", "Degree assortativity coefficient (Pearson correlation of degrees at edge ends).", {"graph": "Graph"}, "Scalar", "O(m)")
def assortativity(ctx, graph="G"):
    return _graph(ctx, graph).assortativity_degree(directed=_graph(ctx, graph).is_directed())


# --------------------------------------------------------------------------- cores
@op("core_number", "k-core number (coreness) of every node as NodeMap.", {"graph": "Graph"}, "NodeMap", "O(n+m)")
def core_number(ctx, graph="G"):
    g = _graph(ctx, graph)
    return _nodemap(ctx, g.coreness(mode="all"), g)


@op("max_core", "Maximum k such that a k-core exists (degeneracy) and the size of that core.", {"graph": "Graph"}, "Table", "O(n+m)")
def max_core(ctx, graph="G"):
    g = _graph(ctx, graph)
    c = g.coreness(mode="all")
    k = max(c)
    return {"k": int(k), "size": int(sum(1 for x in c if x == k))}


@op("k_core", "Induced subgraph of the k-core.", {"graph": "Graph", "k": "int"}, "Graph", "O(n+m)")
def k_core(ctx, k, graph="G"):
    g = _graph(ctx, graph)
    return g.k_core(int(k))


# --------------------------------------------------------------------------- centrality
@op("pagerank", "PageRank as NodeMap (directed if graph directed).", {"graph": "Graph", "damping": "float (default 0.85)"}, "NodeMap", "O(k(n+m))")
def pagerank(ctx, graph="G", damping=0.85):
    g = _graph(ctx, graph)
    return _nodemap(ctx, g.pagerank(damping=damping, directed=g.is_directed()), g)


@op("betweenness", "Betweenness centrality as NodeMap. tier exact O(nm); tier approx samples k pivot sources (default 500).",
    {"graph": "Graph", "tier": "exact|approx", "k": "int pivots for approx"}, "NodeMap", "O(nm)", tiers=["exact", "approx"])
def betweenness(ctx, graph="G", tier="exact", k=500):
    g = _graph(ctx, graph)
    if tier == "exact":
        return _nodemap(ctx, g.betweenness(directed=g.is_directed()), g)
    rng = random.Random(ctx.seed)
    pivots = rng.sample(range(g.vcount()), min(int(k), g.vcount()))
    vals = g.betweenness(directed=g.is_directed(), sources=pivots)
    scale = g.vcount() / len(pivots)
    return _nodemap(ctx, [v * scale for v in vals], g)


@op("closeness", "Closeness centrality as NodeMap (harmonic variant handles disconnection). tier approx samples k targets.",
    {"graph": "Graph", "tier": "exact|approx", "k": "int"}, "NodeMap", "O(nm)", tiers=["exact", "approx"])
def closeness(ctx, graph="G", tier="exact", k=500):
    g = _graph(ctx, graph)
    if tier == "exact":
        return _nodemap(ctx, g.harmonic_centrality(mode="out"), g)
    rng = random.Random(ctx.seed)
    targets = rng.sample(range(g.vcount()), min(int(k), g.vcount()))
    d = np.array(g.distances(source=None, target=targets, mode="out"))
    with np.errstate(divide="ignore"):
        h = np.where(d > 0, 1.0 / d, 0.0)
    return _nodemap(ctx, (h.sum(axis=1) * (g.vcount() - 1) / len(targets)).tolist(), g)


@op("eigenvector_centrality", "Eigenvector centrality as NodeMap.", {"graph": "Graph"}, "NodeMap", "O(k(n+m))")
def eigenvector_centrality(ctx, graph="G"):
    g = _graph(ctx, graph)
    return _nodemap(ctx, g.eigenvector_centrality(scale=True), g)


# --------------------------------------------------------------------------- communities
@op("communities", "Community detection. method in {leiden, louvain, label_propagation}. Returns count, sizes, modularity, membership. Undirected interpretation.",
    {"graph": "Graph", "method": "str", "resolution": "float (default 1.0)"}, "Partition", "O(k(n+m))")
def communities(ctx, graph="G", method="leiden", resolution=1.0):
    g = _graph(ctx, graph)
    u = g.as_undirected(mode="collapse") if g.is_directed() else g
    w = "weight" if "weight" in u.es.attributes() else None
    random.seed(ctx.seed)
    ig.set_random_number_generator(random)
    if method == "leiden":
        cl = u.community_leiden(objective_function="modularity", weights=w, resolution=resolution, n_iterations=-1)
    elif method == "louvain":
        cl = u.community_multilevel(weights=w, resolution=resolution)
    elif method == "label_propagation":
        cl = u.community_label_propagation(weights=w)
    else:
        raise ValueError(f"unknown method {method}")
    sizes = sorted(cl.sizes(), reverse=True)
    return {"count": len(cl), "sizes": sizes[:20], "largest": sizes[0], "modularity": float(u.modularity(cl, weights=w)),
            "membership": cl.membership, "_names": g.vs["name"], "method": method, "_kind": "communities"}


@op("largest_block", "Node set of the largest block of a Partition (component or community).", {"partition": "Partition"}, "NodeSet", "O(n)")
def largest_block(ctx, partition):
    p = ctx.values[partition] if isinstance(partition, str) else partition
    mem = p["membership"]
    names = p.get("_names") or ctx.lg.labels
    counts: dict[int, int] = {}
    for c in mem:
        counts[c] = counts.get(c, 0) + 1
    big = max(counts, key=counts.get)
    return [names[i] for i, c in enumerate(mem) if c == big]


@op("block_of", "Node set of the block containing a given node.", {"partition": "Partition", "node": "NodeRef"}, "NodeSet", "O(n)")
def block_of(ctx, partition, node):
    p = ctx.values[partition] if isinstance(partition, str) else partition
    mem = p["membership"]
    names = p.get("_names") or ctx.lg.labels
    c = mem[names.index(ctx.label(ctx.node(node)))]
    return [names[i] for i, x in enumerate(mem) if x == c]


# --------------------------------------------------------------------------- set / map algebra
@op("induced_subgraph", "Subgraph induced by a NodeSet.", {"graph": "Graph", "nodes": "NodeSet"}, "Graph", "O(n+m)")
def induced_subgraph(ctx, nodes, graph="G"):
    g = _graph(ctx, graph)
    ns = _nodeset(ctx, nodes)
    idx = [_vid(ctx, g, x) for x in ns]
    return g.induced_subgraph(idx)


@op("top_k", "Top-k entries of a NodeMap by value (descending). Returns ordered list of {node, value}.",
    {"map": "NodeMap", "k": "int", "ascending": "bool"}, "Table", "O(n log n)")
def top_k(ctx, map, k=10, ascending=False):
    m = ctx.values[map] if isinstance(map, str) else map
    items = sorted(m.items(), key=lambda kv: kv[1], reverse=not ascending)[: int(k)]
    return [{"node": n, "value": v} for n, v in items]


@op("map_value", "Value of a NodeMap at one node.", {"map": "NodeMap", "node": "NodeRef"}, "Scalar", "O(1)")
def map_value(ctx, map, node):
    m = ctx.values[map] if isinstance(map, str) else map
    return m[ctx.label(ctx.node(node))]


@op("set_intersection", "Nodes in both NodeSets.", {"a": "NodeSet", "b": "NodeSet"}, "NodeSet", "O(n)")
def set_intersection(ctx, a, b):
    sb = set(_nodeset(ctx, b))
    return [x for x in _nodeset(ctx, a) if x in sb]


@op("set_union", "Nodes in either NodeSet.", {"a": "NodeSet", "b": "NodeSet"}, "NodeSet", "O(n)")
def set_union(ctx, a, b):
    seen = set(); out = []
    for x in _nodeset(ctx, a) + _nodeset(ctx, b):
        if x not in seen:
            seen.add(x); out.append(x)
    return out


@op("set_difference", "Nodes in a but not in b.", {"a": "NodeSet", "b": "NodeSet"}, "NodeSet", "O(n)")
def set_difference(ctx, a, b):
    sb = set(_nodeset(ctx, b))
    return [x for x in _nodeset(ctx, a) if x not in sb]


@op("argmax", "Node with the largest NodeMap value (optionally within a NodeSet). Returns {node, value}.",
    {"map": "NodeMap", "nodes": "NodeSet or null"}, "Table", "O(n)")
def argmax(ctx, map, nodes=None):
    m = ctx.values[map] if isinstance(map, str) else map
    keys = _nodeset(ctx, nodes) if nodes is not None else list(m.keys())
    best = max(keys, key=lambda k: m[k])
    return {"node": best, "value": m[best]}


@op("filter_nodes", "Subset of a NodeSet (or all nodes) whose NodeMap value satisfies op in {>,>=,<,<=,==} threshold.",
    {"map": "NodeMap", "op": "str", "threshold": "number", "nodes": "NodeSet or null"}, "NodeSet", "O(n)")
def filter_nodes(ctx, map, op, threshold, nodes=None):
    m = ctx.values[map] if isinstance(map, str) else map
    ns = _nodeset(ctx, nodes) if nodes is not None else list(m.keys())
    f = {">": lambda a, b: a > b, ">=": lambda a, b: a >= b, "<": lambda a, b: a < b, "<=": lambda a, b: a <= b, "==": lambda a, b: a == b}[op]
    return [n for n in ns if f(m[n], threshold)]


@op("restrict_map", "Restrict a NodeMap to a NodeSet.", {"map": "NodeMap", "nodes": "NodeSet"}, "NodeMap", "O(n)")
def restrict_map(ctx, map, nodes):
    m = ctx.values[map] if isinstance(map, str) else map
    ns = _nodeset(ctx, nodes)
    return {n: m[n] for n in ns if n in m}


@op("map_stats", "min, max, mean, median of a NodeMap.", {"map": "NodeMap"}, "Table", "O(n)")
def map_stats(ctx, map):
    m = ctx.values[map] if isinstance(map, str) else map
    v = np.array(list(m.values()), dtype=float)
    return {"min": float(v.min()), "max": float(v.max()), "mean": float(v.mean()), "median": float(np.median(v)), "n": int(len(v))}


@op("set_size", "Number of elements in a NodeSet.", {"nodes": "NodeSet"}, "Scalar", "O(1)")
def set_size(ctx, nodes):
    return len(_nodeset(ctx, nodes))


@op("pairwise_distances", "Distances between all pairs in a NodeSet; returns the pair with the largest distance and the full matrix.",
    {"graph": "Graph", "nodes": "NodeSet"}, "Table", "O(|S|(n+m))")
def pairwise_distances(ctx, nodes, graph="G"):
    g = _graph(ctx, graph)
    ns = _nodeset(ctx, nodes)
    idx = [_vid(ctx, g, x) for x in ns]
    d = g.distances(source=idx, target=idx, mode="out")
    best = (-1, None, None)
    for i, a in enumerate(ns):
        for j, b in enumerate(ns):
            if i < j and d[i][j] != math.inf and d[i][j] > best[0]:
                best = (d[i][j], a, b)
    return {"max_distance": best[0], "pair": [best[1], best[2]], "matrix": [[(x if x != math.inf else None) for x in row] for row in d]}


@op("compare", "Compare two scalars (step refs, 'step.field' refs, or literals), optionally labelled: returns {a, b, a_greater, greater: label, ratio}.",
    {"a": "Scalar", "b": "Scalar", "label_a": "str or null", "label_b": "str or null"}, "Table", "O(1)")
def compare(ctx, a, b, label_a=None, label_b=None):
    av = ctx.values[a] if isinstance(a, str) and a in ctx.values else a
    bv = ctx.values[b] if isinstance(b, str) and b in ctx.values else b
    av = float(av); bv = float(bv)
    la = label_a if label_a is not None else (a if isinstance(a, str) else "a")
    lb = label_b if label_b is not None else (b if isinstance(b, str) else "b")
    return {"a": av, "b": bv, "label_a": la, "label_b": lb, "a_greater": av > bv, "greater": la if av > bv else lb, "ratio": (av / bv) if bv else None}


@op("is_finite", "True if a scalar is a finite number (e.g. a distance that is not inf).", {"value": "Scalar"}, "Bool", "O(1)")
def is_finite(ctx, value):
    v = ctx.values[value] if isinstance(value, str) and value in ctx.values else value
    try:
        return math.isfinite(float(v))
    except (TypeError, ValueError):
        return False


def catalogue_text(include_tiers: bool = True) -> str:
    lines = []
    for s in CATALOGUE.values():
        p = ", ".join(f"{k}: {v}" for k, v in s.params.items())
        t = f" tiers={s.tiers}" if include_tiers and len(s.tiers) > 1 else ""
        d = " [directed graphs only]" if s.directed_only else ""
        lines.append(f"- {s.name}({p}) -> {s.returns}  cost {s.cost}{t}{d}\n    {s.doc}")
    return "\n".join(lines)
