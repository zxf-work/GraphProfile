"""Run the headline experiment: every system on every benchmark question, each run isolated in a subprocess.

Usage:
  python run_experiment.py --systems full,no_verify,no_interp,flat,codegen --qids all --workers 3
  python run_experiment.py --score-only
Raw results land in results/raw/<system>/<qid>.json; the summary in results/summary.md.
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import time
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor, as_completed

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, "bench"))
from score import score_answer, score_plan  # noqa: E402

QUESTIONS = json.load(open(os.path.join(HERE, "bench", "questions.json")))
RESULTS = os.path.join(HERE, os.environ.get("T2R_RESULTS", "results"))
RAW = os.path.join(RESULTS, "raw")
SYSTEMS_ALL = ["full", "no_verify", "no_interp", "flat", "codegen"]


def run_one_subprocess(system: str, q: dict, timeout: int, model: str) -> dict:
    out = os.path.join(RAW, system, f"{q['id']}.json")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    cmd = [sys.executable, os.path.join(HERE, "run_one.py"), "--system", system, "--qid", q["id"], "--out", out, "--model", model]
    t0 = time.time()
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout, cwd=HERE)
        if p.returncode != 0 and not os.path.exists(out):
            json.dump({"system": system, "qid": q["id"], "error": f"runner failed rc={p.returncode}: {p.stderr[-800:]}", "seconds": time.time() - t0}, open(out, "w"))
    except subprocess.TimeoutExpired:
        json.dump({"system": system, "qid": q["id"], "error": f"timeout after {timeout}s", "seconds": timeout}, open(out, "w"))
    return json.load(open(out))


def load_graphs():
    from t2r.graphs import load_graph
    ds = json.load(open(os.path.join(HERE, "bench", "datasets.json")))
    graphs = {}
    for name, d in ds.items():
        graphs[name] = load_graph(name, os.path.normpath(os.path.join(HERE, d["path"])), "", override={"directed": d["directed"], "weighted": d["weighted"]})
    return graphs


def summarise(systems: list[str]) -> str:
    graphs = load_graphs()
    rows = []
    for q in QUESTIONS:
        for s in systems:
            p = os.path.join(RAW, s, f"{q['id']}.json")
            if not os.path.exists(p):
                continue
            r = json.load(open(p))
            a, note = score_answer(q, r, graphs)
            pl, pnote = score_plan(q, r)
            rows.append({"qid": q["id"], "family": q["family"], "dataset": q["dataset"], "system": s, "answer_score": a, "plan_score": pl,
                         "note": note, "plan_note": pnote, "calls": r.get("llm_calls", 0), "tokens": (r.get("tokens_in", 0) or 0) + (r.get("tokens_out", 0) or 0),
                         "cost": r.get("cost_usd", 0.0) or 0.0, "seconds": r.get("seconds", 0.0) or 0.0, "error": r.get("error"), "clarification": r.get("clarification")})
    json.dump(rows, open(os.path.join(RESULTS, "scores.json"), "w"), indent=1)
    fams = []
    for q in QUESTIONS:
        if q["family"] not in fams:
            fams.append(q["family"])
    by = defaultdict(list)
    for r in rows:
        by[(r["system"], r["family"])].append(r)
        by[(r["system"], "ALL")].append(r)
    md = ["# Headline experiment results\n", f"Questions: {len(QUESTIONS)}. Systems: {', '.join(systems)}. Model: {os.environ.get('T2R_MODEL', 'claude-opus-5-5')}. Results dir: {os.path.basename(RESULTS)}.\n",
          "## Answer accuracy by family (mean score, 0 to 1)\n", "| family | n | " + " | ".join(systems) + " |", "|---|---|" + "---|" * len(systems)]
    for f in fams + ["ALL"]:
        n = len(by[(systems[0], f)]) if by[(systems[0], f)] else len([q for q in QUESTIONS if q["family"] == f])
        cells = []
        for s in systems:
            xs = by[(s, f)]
            cells.append(f"{sum(x['answer_score'] for x in xs) / len(xs):.2f}" if xs else "-")
        md.append(f"| {f} | {n} | " + " | ".join(cells) + " |")
    md += ["\n## Plan correctness (mean, 0 to 1; n/a for codegen)\n", "| family | " + " | ".join(systems) + " |", "|---|" + "---|" * len(systems)]
    for f in fams + ["ALL"]:
        cells = []
        for s in systems:
            xs = [x for x in by[(s, f)] if x["plan_score"] is not None]
            cells.append(f"{sum(x['plan_score'] for x in xs) / len(xs):.2f}" if xs else "n/a")
        md.append(f"| {f} | " + " | ".join(cells) + " |")
    md += ["\n## Cost per question (means)\n", "| system | LLM calls | tokens | USD | seconds | errors/timeouts |", "|---|---|---|---|---|---|"]
    for s in systems:
        xs = by[(s, "ALL")]
        if not xs:
            continue
        md.append(f"| {s} | {sum(x['calls'] for x in xs) / len(xs):.1f} | {sum(x['tokens'] for x in xs) / len(xs):.0f} | {sum(x['cost'] for x in xs) / len(xs):.3f} | {sum(x['seconds'] for x in xs) / len(xs):.0f} | {sum(1 for x in xs if x['error'])} |")
    md += ["\n## Per-question scores\n", "| qid | family | " + " | ".join(systems) + " |", "|---|---|" + "---|" * len(systems)]
    for q in QUESTIONS:
        cells = []
        for s in systems:
            x = next((r for r in rows if r["qid"] == q["id"] and r["system"] == s), None)
            cells.append("-" if x is None else f"{x['answer_score']:.1f}" + (" ?" if x["clarification"] else ""))
        md.append(f"| {q['id']} | {q['family']} | " + " | ".join(cells) + " |")
    md += ["\n## Failure notes (score < 1)\n"]
    for r in rows:
        if r["answer_score"] < 1:
            md.append(f"- {r['system']} {r['qid']} ({r['family']}): score {r['answer_score']:.1f}: {r['note']}" + (f" [error: {r['error'][:100]}]" if r["error"] else ""))
    text = "\n".join(md) + "\n"
    os.makedirs(RESULTS, exist_ok=True); open(os.path.join(RESULTS, "summary.md"), "w").write(text)
    return text


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--systems", default=",".join(SYSTEMS_ALL))
    ap.add_argument("--qids", default="all")
    ap.add_argument("--workers", type=int, default=3)
    ap.add_argument("--timeout", type=int, default=1200)
    ap.add_argument("--model", default=os.environ.get("T2R_MODEL", "claude-opus-5-5"))
    ap.add_argument("--score-only", action="store_true")
    ap.add_argument("--force", action="store_true", help="re-run even if a result exists")
    a = ap.parse_args()
    systems = a.systems.split(",")
    if not a.score_only:
        qs = QUESTIONS if a.qids == "all" else [q for q in QUESTIONS if q["id"] in a.qids.split(",")]
        jobs = [(s, q) for q in qs for s in systems if a.force or not os.path.exists(os.path.join(RAW, s, f"{q['id']}.json"))]
        print(f"{len(jobs)} runs with {a.workers} workers", flush=True)
        t0 = time.time()
        with ThreadPoolExecutor(max_workers=a.workers) as ex:
            futs = {ex.submit(run_one_subprocess, s, q, a.timeout, a.model): (s, q) for s, q in jobs}
            for i, f in enumerate(as_completed(futs)):
                s, q = futs[f]
                r = f.result()
                print(f"[{i + 1}/{len(jobs)} {time.time() - t0:.0f}s] {s:10s} {q['id']:5s} ${r.get('cost_usd', 0) or 0:.3f} {r.get('seconds', 0) or 0:.0f}s {'ERR ' + str(r.get('error'))[:60] if r.get('error') else 'ok'}", flush=True)
    print(summarise(systems))


if __name__ == "__main__":
    main()
