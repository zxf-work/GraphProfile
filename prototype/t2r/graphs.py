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
    weight_column_name: Optional[str] = None
    columns_used: tuple[int, int, Optional[int]] = (0, 1, None)   # (source col, target col, weight col)
    row_index_column: bool = False          # first column is a running row number, not a node id
    weight_has_fractions: bool = False
    id_ranges_disjoint: bool = False        # source ids and target ids never overlap (bipartite file)


def _is_number(s: str) -> bool:
    try:
        float(s)
        return True
    except ValueError:
        return False


SRC_NAMES = {"source", "src", "from", "u", "node1", "a", "head", "user", "person", "from_id", "src_id", "i", "sender", "citing", "follower", "buyer", "parent", "from_junction", "customer"}
TGT_NAMES = {"target", "tgt", "to", "v", "node2", "b", "tail", "item", "to_id", "dst", "dst_id", "j", "recipient", "cited", "followed", "seller", "child", "to_junction", "product"}
NOT_WEIGHT = ("time", "date", "stamp", "id", "year", "label", "type", "name")
WEIGHT_NAMES = {"weight", "w", "count", "n", "distance", "dist", "km", "length", "cost", "time", "minutes", "strength", "value", "freq", "frequency", "times", "distance_km", "travel_time_min"}
INDEX_NAMES = {"id", "row", "idx", "index", "edge_id", "eid", "#"}


def _pick_columns(header: Optional[list[str]], raw: list[list[str]]) -> tuple[int, int, Optional[int], bool]:
    """Decide which columns are source, target, weight. Returns (s, t, w, row_index_detected)."""
    ncol = max(len(r) for r in raw) if raw else (len(header) if header else 2)
    if header:
        low = [c.strip().lower() for c in header]
        si = next((i for i, c in enumerate(low) if c in SRC_NAMES), None)
        ti = next((i for i, c in enumerate(low) if c in TGT_NAMES and i != si), None)
        wi = next((i for i, c in enumerate(low) if c in WEIGHT_NAMES and i not in (si, ti)), None)
        idx = next((i for i, c in enumerate(low) if c in INDEX_NAMES), None)
        if si is None or ti is None:
            cands = [i for i in range(len(low)) if i != idx]
            si = cands[0] if si is None else si
            ti = next(i for i in cands if i != si) if ti is None else ti
        if wi is None and ncol > 2:
            wi = next((i for i in range(ncol) if i not in (si, ti, idx) and i < len(low) and not any(k in low[i] for k in NOT_WEIGHT)), None)
        return si, ti, wi, idx is not None
    # no header: detect a running row index in column 0 when there are 3+ columns
    if ncol >= 3 and len(raw) >= 20:
        c0 = [r[0] for r in raw[:200] if len(r) >= 3]
        if all(_is_number(x) for x in c0):
            vals = [int(float(x)) for x in c0]
            if all(vals[i + 1] - vals[i] == 1 for i in range(len(vals) - 1)):
                wi = 3 if ncol >= 4 else None
                return 1, 2, wi, True
    return 0, 1, (2 if ncol > 2 else None), False


BIG_FILE_BYTES = 60_000_000


def read_raw_fast(path: str) -> tuple[RawFacts, Any]:
    """Large-file path: pandas load, numpy statistics, sampled reciprocity. Returns (facts, (u, v, w) arrays)."""
    import pandas as pd
    h = hashlib.sha1()
    with open(path, "rb") as f:
        h.update(f.read(1 << 20)); h.update(str(os.path.getsize(path)).encode())
    with open(path, "r") as f:
        first = f.readline()
    delim = "," if "," in first else r"\s+"
    toks = [x.strip() for x in (first.strip().split(",") if delim == "," else first.split())]
    header = toks if (len(toks) >= 2 and not all(_is_number(x) for x in toks[:2]) and not first.startswith("#")) else None
    # Read in chunks: a single frame for a 50M-row file peaks at several GB; chunks bound the peak to one chunk plus the column arrays.
    us, vs, ws = [], [], []
    raw_sample = None; cols = None; si = ti = wi = row_index = None
    for df in pd.read_csv(path, sep=delim, header=0 if header else None, comment="#", engine="c", dtype=str if header else None, chunksize=5_000_000):
        if raw_sample is None:
            raw_sample = df.head(200).astype(str).values.tolist()
            si, ti, wi, row_index = _pick_columns(header, raw_sample)
            cols = list(df.columns)
        us.append(df[cols[si]].to_numpy()); vs.append(df[cols[ti]].to_numpy())
        if wi is not None and wi < len(cols):
            ws.append(df[cols[wi]])
        del df
    u = np.concatenate(us); v = np.concatenate(vs); del us, vs
    wcol = pd.concat(ws, ignore_index=True) if ws else None; del ws
    try:
        u = u.astype(np.int64); v = v.astype(np.int64); numeric = True
    except (ValueError, TypeError):
        numeric = False
    w = None
    if wi is not None and wi < len(cols):
        try:
            w = pd.to_numeric(wcol, errors="coerce").to_numpy(dtype=float)
            if np.isnan(w).any():
                w = None
        except Exception:
            w = None
    n_rows = len(u)
    if numeric:
        N = int(max(u.max(), v.max())) + 1
        # presence arrays instead of np.unique over the concatenation: O(N) bytes rather than a sorted 2m-element copy
        if N <= 50 * n_rows + 1000:
            seen_u = np.zeros(N, dtype=bool); seen_u[u] = True
            seen_v = np.zeros(N, dtype=bool); seen_v[v] = True
            n_ids = int((seen_u | seen_v).sum())
            srcs_tgts_disjoint = not bool((seen_u & seen_v).any())
            del seen_u, seen_v
        else:
            n_ids = len(np.unique(np.concatenate([u, v])))
            srcs_tgts_disjoint = len(np.intersect1d(np.unique(u), np.unique(v))) == 0
        self_loops = int((u == v).sum())
        lo = np.minimum(u, v); hi = np.maximum(u, v)
        n_dedup_u = len(np.unique(lo * N + hi))
        del lo, hi
        key_d = u * N + v
        uniq_d = np.unique(key_d)
        del key_d
        n_dedup_d = len(uniq_d)
        rng = np.random.default_rng(0)
        idx = rng.choice(n_rows, size=min(200_000, n_rows), replace=False)
        su, sv = u[idx], v[idx]
        nonloop = su != sv
        rev = sv[nonloop] * N + su[nonloop]
        pos = np.searchsorted(uniq_d, rev)
        pos[pos >= len(uniq_d)] = 0
        recip = float(np.mean(uniq_d[pos] == rev)) if nonloop.any() else 0.0
        del uniq_d, rev, pos
    else:
        n_ids = len(set(u) | set(v)); self_loops = int((u == v).sum()); recip = 0.0
        n_dedup_d = n_rows; n_dedup_u = n_rows; srcs_tgts_disjoint = False
    facts = RawFacts(
        path=path, n_lines=n_rows + (1 if header else 0), header=header, n_columns=len(cols), sample_rows=raw_sample[:5],
        ids_numeric=numeric, n_distinct_ids=n_ids, n_rows=n_rows, n_rows_dedup_directed=n_dedup_d, n_rows_dedup_undirected=n_dedup_u,
        n_self_loops=self_loops, reciprocity=recip, weight_column_numeric=w is not None,
        weight_values_distinct=int(len(np.unique(w))) if w is not None else 0,
        weight_min=float(w.min()) if w is not None else None, weight_max=float(w.max()) if w is not None else None,
        sha1=h.hexdigest()[:12], weight_column_name=(header[wi] if (header and wi is not None and wi < len(header)) else None),
        columns_used=(si, ti, wi), row_index_column=row_index,
        weight_has_fractions=bool(w is not None and (w != np.floor(w)).any()), id_ranges_disjoint=srcs_tgts_disjoint,
    )
    return facts, (u, v, w)


def read_raw(path: str) -> tuple[RawFacts, list[tuple[str, str, Optional[float]]]]:
    rows: list[tuple[str, str, Optional[float]]] = []
    header = None
    sample: list[list[str]] = []
    h = hashlib.sha1()
    n_lines = 0
    raw: list[list[str]] = []
    with open(path, "r", newline="") as f:
        first = f.readline()
        n_lines += 1
        h.update(first.encode())
        delim = "," if "," in first else ("\t" if "\t" in first and not " " in first.strip() else None)
        toks = [x.strip() for x in first.strip().split(delim)]
        if toks and len(toks) >= 2 and not all(_is_number(x) for x in toks[:2]) and not first.startswith("#"):
            header = toks
        elif len(toks) >= 2 and not first.startswith("#"):
            raw.append(toks)
        for line in f:
            n_lines += 1
            h.update(line.encode())
            if not line.strip() or line.startswith("#"):
                continue
            t = [x.strip() for x in line.strip().split(delim)]
            if len(t) < 2:
                continue
            raw.append(t)
    si, ti, wi, row_index = _pick_columns(header, raw)
    for t in raw:
        if len(t) <= max(si, ti):
            continue
        w = float(t[wi]) if (wi is not None and len(t) > wi and _is_number(t[wi])) else None
        rows.append((t[si], t[ti], w))
        if len(sample) < 5:
            sample.append(t)
    srcs = set(u for u, _, _ in rows); tgts = set(v for _, v, _ in rows)
    ids = srcs | tgts
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
        weight_column_name=(header[wi].strip() if (header and wi is not None and wi < len(header)) else None),
        columns_used=(si, ti, wi), row_index_column=row_index,
        weight_has_fractions=any(not float(w).is_integer() for w in weights[:10000]) if weights else False,
        id_ranges_disjoint=bool(rows) and not (srcs & tgts),
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
    hdr_l = [c.lower() for c in facts.header] if facts.header else []
    DIRECTED_HDR = {"sender", "recipient", "citing", "cited", "follower", "followed", "buyer", "seller", "parent", "child", "trust", "rating", "payer", "payee", "caller", "callee"}
    hits = [c for c in hdr_l if c in DIRECTED_HDR]
    if hits and facts.reciprocity < 0.98:
        directed, c = True, max(c, 0.85)
        ev["directed"] = ev["directed"] + f"; column names {hits} denote an asymmetric relation"
        ask = [a for a in ask if a != "directed"]
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
        wname = (facts.weight_column_name or "").lower()
        ev["weighted"] = f"numeric column '{facts.weight_column_name or 'col ' + str(facts.columns_used[2])}' with {facts.weight_values_distinct} distinct values in [{facts.weight_min}, {facts.weight_max}]"
        if any(k in wname for k in ("dist", "km", "length", "cost", "time", "min", "sec", "hour", "metre", "meter", "mile")):
            wsem, conf["weight_semantics"] = "distance", 0.9
            ev["weight_semantics"] = f"column name '{facts.weight_column_name}' denotes a distance or cost"
        elif any(k in wname for k in ("count", "freq", "times", "strength", "n_", "num", "share", "co")):
            wsem, conf["weight_semantics"] = "strength", 0.9
            ev["weight_semantics"] = f"column name '{facts.weight_column_name}' denotes a count or strength"
        elif any(k in desc for k in ("distance", "cost", "length", "km", "time")):
            wsem, conf["weight_semantics"] = "distance", 0.85
        elif any(k in desc for k in ("count", "strength", "co-", "frequency", "number of", "times")):
            wsem, conf["weight_semantics"] = "strength", 0.85
        elif facts.weight_has_fractions:
            wsem, conf["weight_semantics"] = None, 0.4
            ev["weight_semantics"] = "fractional weights under an uninformative name: could be distances or similarities"
            ask.append("weight_semantics")
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
    hdr = " ".join(facts.header).lower() if facts.header else ""
    if dup > 0:
        if not directed and facts.reciprocity > 0.98 and abs(dup - facts.n_rows / 2) < 0.01 * facts.n_rows:
            ev["multigraph"] = "duplicates are exactly the reverse copies of an undirected graph stored both ways; collapsed"
            conf["multigraph"] = 0.95
        elif dup / facts.n_rows > 0.05 or any(k in hdr for k in ("time", "date", "event", "msg", "message", "email", "call", "transaction")):
            multigraph = True
            conf["multigraph"] = 0.8
            ev["multigraph"] = f"{dup} repeated rows ({dup / facts.n_rows:.1%}): rows look like events (repeated interactions), kept as parallel edges; use distinct_edge_count for unique pairs"
            if not any(k in hdr for k in ("time", "date", "event", "msg", "message", "email", "call", "transaction")):
                ask.append("multigraph")
        else:
            ev["multigraph"] = f"{dup} duplicate rows ({dup / facts.n_rows:.1%}); treated as noise and collapsed"
            conf["multigraph"] = 0.7
    else:
        conf["multigraph"] = 0.95
        ev["multigraph"] = "no duplicate rows"

    labelled = not facts.ids_numeric
    if facts.row_index_column:
        ev["columns"] = f"first column is a running row number; edges read from columns {facts.columns_used[0]} and {facts.columns_used[1]}"
        conf["columns"] = 0.9
    if facts.id_ranges_disjoint:
        ev["bipartite"] = "source ids and target ids never overlap: the file looks bipartite (two node types); one-mode questions need a projection"
        conf["bipartite"] = 0.85
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
    big = os.path.getsize(path) > BIG_FILE_BYTES
    if big:
        facts, arrays = read_raw_fast(path)
        rows = None
    else:
        facts, rows = read_raw(path)
    sem = infer_semantics(facts, description)
    if override:
        for k, v in override.items():
            setattr(sem, k, v)
            sem.confidence[k] = 1.0
            sem.evidence[k] = "declared by user"
        sem.needs_clarification = [a for a in sem.needs_clarification if a not in override]
    if big:
        return _build_big(name, facts, sem, arrays)
    # vertex index
    if facts.ids_numeric:
        ids = sorted({int(float(u)) for u, v, _ in rows} | {int(float(v)) for u, v, _ in rows})
        labels = [str(i) for i in ids]
    else:
        labels = sorted({u for u, v, _ in rows} | {v for u, v, _ in rows})
    index = {lab: i for i, lab in enumerate(labels)}
    def key(x: str) -> str:
        return str(int(float(x))) if facts.ids_numeric else x
    elist: list[tuple[int, int]] = []
    wlist: list[float] = []
    seen: set[tuple[int, int]] = set()
    for u, v, w in rows:
        a, b = index[key(u)], index[key(v)]
        if a == b:
            continue
        if not sem.directed and a > b:
            a, b = b, a
        if (a, b) in seen and not sem.multigraph:
            continue
        seen.add((a, b))
        elist.append((a, b))
        wlist.append(w if (w is not None and sem.weighted) else 1.0)
    g = ig.Graph(n=len(labels), edges=elist, directed=sem.directed)
    g.vs["name"] = labels
    if sem.weighted:
        g.es["weight"] = wlist
    return LoadedGraph(name=name, g=g, facts=facts, semantics=sem, labels=labels, index=index,
                       lower_index={l.lower(): i for l, i in index.items()})


class _RangeIndex:
    """label -> index mapping for contiguous integer ids 0..N-1 without a 10M-entry dict."""

    def __init__(self, n: int):
        self.n = n

    def _key(self, k: Any) -> Optional[int]:
        try:
            i = int(k)
        except (TypeError, ValueError):
            return None
        return i if 0 <= i < self.n and str(i) == str(k).strip() else None

    def __contains__(self, k: Any) -> bool:
        return self._key(k) is not None

    def __getitem__(self, k: Any) -> int:
        i = self._key(k)
        if i is None:
            raise KeyError(k)
        return i

    def __len__(self) -> int:
        return self.n


def _build_big(name: str, facts: RawFacts, sem: Semantics, arrays) -> LoadedGraph:
    u, v, w = arrays
    del arrays
    if not facts.ids_numeric:
        raise ValueError("large files must have integer node ids")
    nmax = int(max(u.max(), v.max())); nmin = int(min(u.min(), v.min()))
    contiguous = nmin == 0 and nmax + 1 == facts.n_distinct_ids
    if contiguous:
        N = nmax + 1
        a, b = u, v
        del u, v
    else:
        ids, inv = np.unique(np.concatenate([u, v]), return_inverse=True)
        N = len(ids)
        a = inv[: len(u)].astype(np.int64); b = inv[len(u):].astype(np.int64)
        del inv, u, v
    if not sem.directed:
        lo = np.minimum(a, b); hi = np.maximum(a, b); a, b = lo, hi
        del lo, hi
    mask = a != b
    if not mask.all():
        a, b = a[mask], b[mask]
        ww = w[mask] if w is not None else None
    else:
        ww = w
    del mask, w
    n_dedup = facts.n_rows_dedup_directed if sem.directed else facts.n_rows_dedup_undirected
    if not sem.multigraph and n_dedup < facts.n_rows:
        key = a.astype(np.int64) * N + b
        _, first = np.unique(key, return_index=True)
        del key
        a, b = a[first], b[first]
        ww = ww[first] if ww is not None else None
        del first
    # igraph accepts a numpy (m, 2) array directly; a Python list of 50M tuples would need ~6 GB on its own
    dt = np.int32 if N < 2**31 else np.int64
    edges = np.stack([a.astype(dt, copy=False), b.astype(dt, copy=False)], axis=1)
    del a, b
    # add edges in slices: one-shot construction from a 50M-row array transiently needs ~3x the final graph size
    g = ig.Graph(n=N, directed=sem.directed)
    for i in range(0, len(edges), 2_000_000):
        g.add_edges(edges[i:i + 2_000_000])
    del edges
    if contiguous:
        labels = np.arange(N).astype(str).tolist()
        index: Any = _RangeIndex(N)
    else:
        labels = ids.astype(str).tolist()
        index = {lab: i for i, lab in enumerate(labels)}
    g.vs["name"] = labels
    if sem.weighted and ww is not None:
        g.es["weight"] = ww.tolist()
    return LoadedGraph(name=name, g=g, facts=facts, semantics=sem, labels=labels, index=index, lower_index=index)


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
    if g.ecount() and g.ecount() < 5_000_000:
        card["is_bipartite"] = bool(g.is_bipartite())
    card["size_class"] = "small" if g.ecount() < 1_000_000 else ("large" if g.ecount() < 20_000_000 else "very large")
    if lg.semantics.multigraph:
        card["n_edges_distinct_pairs"] = len(set((min(e.source, e.target), max(e.source, e.target)) if not g.is_directed() else (e.source, e.target) for e in g.es))
    if lg.facts.header:
        card["columns"] = {"header": lg.facts.header, "source_col": lg.facts.columns_used[0], "target_col": lg.facts.columns_used[1], "weight_col": lg.facts.columns_used[2]}
    elif lg.facts.row_index_column:
        card["columns"] = {"note": "first column is a row index; edges read from columns 1 and 2"}
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
        "n_rows": facts.n_rows, "n_distinct_directed_pairs": facts.n_rows_dedup_directed, "n_distinct_undirected_pairs": facts.n_rows_dedup_undirected,
        "reciprocity_of_rows": round(facts.reciprocity, 4), "weight_column_numeric": facts.weight_column_numeric,
    }
