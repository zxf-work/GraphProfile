"""Graph loading, semantics inference, profile card, and node-name resolution.

The profile card is the only description of the graph that an LLM ever sees.
"""
from __future__ import annotations

import csv
import hashlib
import os
import re
from dataclasses import dataclass, field, asdict
from difflib import get_close_matches
from typing import Any, Optional

import igraph as ig
import numpy as np


# --------------------------------------------------------------------------- raw file facts
@dataclass
class RawFacts:
    """What can be read from the file without interpreting it."""
    path: str
    n_lines: int
    header: Optional[list[str]]
    n_columns: int
    sample_rows: list[list[str]]
    ids_numeric: bool
    n_distinct_ids: int
    n_rows: int
    n_rows_dedup_directed: int       # distinct (u,v)
    n_rows_dedup_undirected: int     # distinct {u,v}
    n_self_loops: int
    reciprocity: float               # fraction of (u,v) rows whose (v,u) also appears
    weight_column_numeric: bool
    weight_values_distinct: int
    weight_min: Optional[float]
    weight_max: Optional[float]
    sha1: str


def _is_number(s: str) -> bool:
    try:
        float(s)
        return True
    except ValueError:
        return False


def read_raw(path: str) -> tuple[RawFacts, list[tuple[str, str, Optional[float]]]]:
    rows: list[tuple[str, str, Optional[float]]] = []
    header = None
    sample: list[list[str]] = []
    h = hashlib.sha1()
    n_lines = 0
    with open(path, "r", newline="") as f:
        first = f.readline()
        n_lines += 1
        h.update(first.encode())
        delim = "," if "," in first else None
        toks = first.strip().split(delim)
        if toks and not _is_number(toks[0]) and not _is_number(toks[-1]) and len(toks) >= 2 and toks[0].lower() in {"source", "src", "from", "u", "node1", "a"}:
            header = toks
        else:
            if len(toks) >= 2 and not first.startswith("#"):
                rows.append((toks[0], toks[1], float(toks[2]) if len(toks) > 2 and _is_number(toks[2]) else None))
                sample.append(toks)
        for line in f:
            n_lines += 1
            h.update(line.encode())
            if not line.strip() or line.startswith("#"):
                continue
            t = line.strip().split(delim)
            if len(t) < 2:
                continue
            rows.append((t[0], t[1], float(t[2]) if len(t) > 2 and _is_number(t[2]) else None))
            if len(sample) < 5:
                sample.append(t)
    ids = set()
    for u, v, _ in rows:
        ids.add(u); ids.add(v)
    ids_numeric = all(_is_number(x) for x in list(ids)[:1000])
    directed_set = set((u, v) for u, v, _ in rows)
    undirected_set = set(frozenset((u, v)) if u != v else frozenset((u,)) for u, v, _ in rows)
    self_loops = sum(1 for u, v, _ in rows if u == v)
    recip = sum(1 for (u, v) in directed_set if u != v and (v, u) in directed_set)
    non_loop = sum(1 for (u, v) in directed_set if u != v)
    reciprocity = recip / non_loop if non_loop else 0.0
    weights = [w for _, _, w in rows if w is not None]
    n_cols = len(sample[0]) if sample else (len(header) if header else 2)
    facts = RawFacts(
        path=path, n_lines=n_lines, header=header, n_columns=n_cols, sample_rows=sample,
        ids_numeric=ids_numeric, n_distinct_ids=len(ids), n_rows=len(rows),
        n_rows_dedup_directed=len(directed_set), n_rows_dedup_undirected=len(undirected_set),
        n_self_loops=self_loops, reciprocity=reciprocity,
        weight_column_numeric=bool(weights) and len(weights) == len(rows),
        weight_values_distinct=len(set(weights)) if weights else 0,
        weight_min=min(weights) if weights else None, weight_max=max(weights) if weights else None,
        sha1=h.hexdigest()[:12],
    )
    return facts, rows


# --------------------------------------------------------------------------- semantics
@dataclass
class Semantics:
    directed: bool
    weighted: bool
    weight_semantics: Optional[str]   # "strength" | "distance" | None
    multigraph: bool                  # duplicate rows kept as parallel edges
    labelled: bool
    confidence: dict[str, float] = field(default_factory=dict)
    evidence: dict[str, str] = field(default_factory=dict)
    needs_clarification: list[str] = field(default_factory=list)


def infer_semantics(facts: RawFacts, description: str = "") -> Semantics:
    """Infer directedness, weights, multigraph-ness from the file alone.

    Returns a Semantics with confidences and the questions the system should ask
    when the evidence is weak. The description (if provided by the user) is used
    only as a weak textual hint.
    """
    ev: dict[str, str] = {}
    conf: dict[str, float] = {}
    ask: list[str] = []
    desc = description.lower()

    # directedness
    if facts.reciprocity > 0.98:
        directed, c = False, 0.95
        ev["directed"] = f"{facts.reciprocity:.1%} of rows have their reverse present: undirected graph stored in both directions"
    elif facts.reciprocity < 0.02:
        # either an undirected graph stored once, or a directed graph with no reciprocity.
        # Real directed networks almost always have some reciprocity, so lean undirected-stored-once,
        # but flag it.
        directed, c = False, 0.6
        ev["directed"] = f"only {facts.reciprocity:.1%} of rows are reciprocated: either undirected stored once (likely) or a directed graph with no mutual edges"
        ask.append("directed")
    else:
        directed, c = True, 0.85
        ev["directed"] = f"{facts.reciprocity:.1%} of rows are reciprocated: partial reciprocity is the signature of a directed graph"
    if "undirected" in desc or "symmetric" in desc:
        directed, c, ev["directed"] = False, max(c, 0.9), ev["directed"] + "; description says undirected"
        ask = [a for a in ask if a != "directed"]
    elif "directed" in desc or "trust" in desc or "follow" in desc or "cites" in desc:
        directed, c, ev["directed"] = True, max(c, 0.9), ev["directed"] + "; description says directed"
        ask = [a for a in ask if a != "directed"]
    conf["directed"] = c

    # weights
    weighted = facts.weight_column_numeric and facts.weight_values_distinct > 1
    wsem = None
    if weighted:
        conf["weighted"] = 0.9
        ev["weighted"] = f"third numeric column with {facts.weight_values_distinct} distinct values in [{facts.weight_min}, {facts.weight_max}]"
        if any(k in desc for k in ("distance", "cost", "length", "km", "time")):
            wsem, conf["weight_semantics"] = "distance", 0.85
        elif any(k in desc for k in ("count", "strength", "co-", "frequency", "number of", "times")):
            wsem, conf["weight_semantics"] = "strength", 0.85
        elif facts.weight_min is not None and facts.weight_min >= 1 and float(facts.weight_min).is_integer() and float(facts.weight_max).is_integer():
            wsem, conf["weight_semantics"] = "strength", 0.6
            ev["weight_semantics"] = "integer counts starting at 1 look like interaction counts (strength), not distances"
            ask.append("weight_semantics")
        else:
            wsem, conf["weight_semantics"] = None, 0.3
            ask.append("weight_semantics")
    else:
        conf["weighted"] = 0.95
        ev["weighted"] = "no numeric weight column" if facts.n_columns < 3 else "third column is constant"

    # multigraph
    dup = facts.n_rows - (facts.n_rows_dedup_directed if directed else facts.n_rows_dedup_undirected)
    multigraph = False
    if dup > 0:
        if not directed and facts.reciprocity > 0.98 and abs(dup - facts.n_rows / 2) < 0.01 * facts.n_rows:
            ev["multigraph"] = "duplicates are exactly the reverse copies of an undirected graph stored both ways; collapsed"
            conf["multigraph"] = 0.95
        else:
            ev["multigraph"] = f"{dup} duplicate rows ({dup / facts.n_rows:.1%}); treated as noise and collapsed"
            conf["multigraph"] = 0.7
            if dup / facts.n_rows > 0.05:
                ask.append("multigraph")
    else:
        conf["multigraph"] = 0.95
        ev["multigraph"] = "no duplicate rows"

    labelled = not facts.ids_numeric
    return Semantics(directed=directed, weighted=weighted, weight_semantics=wsem,
                     multigraph=multigraph, labelled=labelled, confidence=conf, evidence=ev,
                     needs_clarification=ask)


# --------------------------------------------------------------------------- loaded graph
@dataclass
class LoadedGraph:
    name: str
    g: ig.Graph
    facts: RawFacts
    semantics: Semantics
    labels: list[str]                       # index -> label
    index: dict[str, int]                   # label -> index
    lower_index: dict[str, int]
    strict: bool = False                    # exact-match-only node resolution (ablation)

    def resolve(self, name: Any, k: int = 3) -> dict[str, Any]:
        """Resolve a user-supplied node reference to a vertex index.

        Returns {"matched": idx or None, "method": ..., "candidates": [...]}.
        """
        s = str(name).strip()
        if s in self.index:
            return {"matched": self.index[s], "method": "exact", "candidates": [s]}
        if s.lower() in self.lower_index:
            return {"matched": self.lower_index[s.lower()], "method": "case-insensitive", "candidates": [self.labels[self.lower_index[s.lower()]]]}
        if self.strict or not self.semantics.labelled:
            # numeric ids: never fuzzy-match; strict mode (ablation): exact only
            return {"matched": None, "method": "none", "candidates": []}
        # token containment (e.g. "Jean Valjean" -> "Valjean")
        toks = [t for t in re.split(r"[\s,_-]+", s.lower()) if t]
        contained = [lab for lab in self.labels if any(t == lab.lower() or t in lab.lower().split() for t in toks)]
        if len(contained) == 1:
            return {"matched": self.index[contained[0]], "method": "token", "candidates": contained}
        close = get_close_matches(s, self.labels, n=k, cutoff=0.75)
        if len(close) == 1 or (close and len(contained) == 0):
            return {"matched": self.index[close[0]], "method": "fuzzy", "candidates": close}
        return {"matched": None, "method": "none", "candidates": (contained or close)[:k]}


def load_graph(name: str, path: str, description: str = "", override: Optional[dict] = None) -> LoadedGraph:
    """Load an edge list and build an igraph.Graph according to inferred (or overridden) semantics."""
    facts, rows = read_raw(path)
    sem = infer_semantics(facts, description)
    if override:
        for k, v in override.items():
            setattr(sem, k, v)
            sem.confidence[k] = 1.0
            sem.evidence[k] = "declared by user"
        sem.needs_clarification = [a for a in sem.needs_clarification if a not in override]
    # vertex index
    if facts.ids_numeric:
        ids = sorted({int(float(u)) for u, v, _ in rows} | {int(float(v)) for u, v, _ in rows})
        labels = [str(i) for i in ids]
    else:
        labels = sorted({u for u, v, _ in rows} | {v for u, v, _ in rows})
    index = {lab: i for i, lab in enumerate(labels)}
    def key(x: str) -> str:
        return str(int(float(x))) if facts.ids_numeric else x
    edges: dict[tuple[int, int], float] = {}
    for u, v, w in rows:
        a, b = index[key(u)], index[key(v)]
        if a == b:
            continue
        if not sem.directed and a > b:
            a, b = b, a
        if (a, b) in edges:
            if sem.multigraph:
                pass
            continue
        edges[(a, b)] = w if (w is not None and sem.weighted) else 1.0
    g = ig.Graph(n=len(labels), edges=list(edges.keys()), directed=sem.directed)
    g.vs["name"] = labels
    if sem.weighted:
        g.es["weight"] = list(edges.values())
    return LoadedGraph(name=name, g=g, facts=facts, semantics=sem, labels=labels, index=index,
                       lower_index={l.lower(): i for l, i in index.items()})


# --------------------------------------------------------------------------- profile card
def profile_card(lg: LoadedGraph, with_inference: bool = True) -> dict[str, Any]:
    """Compact, LLM-facing description of the graph. Cheap statistics only."""
    g = lg.g
    deg = np.array(g.degree())
    card: dict[str, Any] = {
        "name": lg.name,
        "file": os.path.basename(lg.facts.path),
        "n_nodes": g.vcount(),
        "n_edges": g.ecount(),
        "density": round(2 * g.ecount() / (g.vcount() * (g.vcount() - 1)) if not g.is_directed() else g.ecount() / (g.vcount() * (g.vcount() - 1)), 6),
        "degree": {"min": int(deg.min()), "median": float(np.median(deg)), "mean": round(float(deg.mean()), 2), "max": int(deg.max())},
        "node_id_type": "integer" if lg.facts.ids_numeric else "string label",
        "sample_node_ids": lg.labels[:5] + (lg.labels[-3:] if len(lg.labels) > 8 else []),
    }
    comps = g.connected_components(mode="weak")
    card["n_components_weak"] = len(comps)
    card["largest_component_fraction"] = round(max(comps.sizes()) / g.vcount(), 4)
    if g.is_directed():
        scc = g.connected_components(mode="strong")
        card["n_components_strong"] = len(scc)
        card["largest_scc_fraction"] = round(max(scc.sizes()) / g.vcount(), 4)
        card["reciprocity"] = round(g.reciprocity(), 4)
    if with_inference:
        s = lg.semantics
        card["semantics"] = {
            "directed": s.directed, "weighted": s.weighted, "weight_semantics": s.weight_semantics,
            "multigraph": s.multigraph, "labelled": s.labelled,
            "confidence": s.confidence, "evidence": s.evidence,
            "needs_clarification": s.needs_clarification,
        }
    return card


def raw_card(facts: RawFacts) -> dict[str, Any]:
    """What a system without an interpretation stage sees: file facts only."""
    return {
        "file": os.path.basename(facts.path), "n_lines": facts.n_lines, "header": facts.header,
        "n_columns": facts.n_columns, "sample_rows": facts.sample_rows,
        "n_distinct_ids": facts.n_distinct_ids, "ids_numeric": facts.ids_numeric,
    }
