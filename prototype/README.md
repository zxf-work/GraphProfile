# Text-to-result graph analysis: prototype and headline experiment

This prototype tests the claim in `../docs/solution-architecture.md`: a typed, interpretation-aware,
self-verifying plan beats both free code generation and flat tool calling on analyst-style questions over
real graphs, and the gain comes from the interpretation and verification stages rather than the executor.

## Layout

```
t2r/graphs.py    loaders, raw file facts, semantics inference, profile card, node-name resolution
t2r/ops.py       operator catalogue (40 typed operators on igraph, with cost classes and approximate tiers)
t2r/ir.py        plan IR (pydantic), validator, cost model / auto-tiering, executor, answer extraction
t2r/verify.py    verification without ground truth: invariants, cross-method agreement, approximation notes
t2r/llm.py       LLM client (Anthropic SDK, or `claude -p` headless inside a Claude Code session)
t2r/systems.py   the five systems compared: full, no_verify, no_interp, flat, codegen
bench/datasets.json     dataset manifest with declared semantics (ground truth for interpretation)
bench/make_questions.py 60 questions in 11 families with igraph ground truth
bench/check_gold.py     NetworkX cross-check of the ground truth
bench/score.py          deterministic checkers per family, plan score, cost
run_experiment.py       runs every (system, question) pair in an isolated subprocess, writes results/summary.md
```

## Systems

| system | interpretation stage | typed IR + validator + cost tiering | verifier | notes |
|---|---|---|---|---|
| full | yes | yes | yes | the proposal |
| no_verify | yes | yes | no | ablation |
| no_interp | no (raw file facts, strict node matching) | yes | yes | ablation |
| flat | no (may call a `file_facts` tool) | no (tool-calling loop over the same operators) | no | conventional agent |
| codegen | no | no (LLM writes a NetworkX/igraph script, run in a subprocess, one retry) | no | strongest conventional baseline |

All systems receive the same user message (file name, optional one-line dataset description, question) and the same model.

## Benchmark

Datasets: SNAP ego-Facebook (4k nodes, undirected, stored once), email-Enron (37k nodes, undirected, stored both ways),
soc-Epinions1 (76k nodes, directed), Les Miserables (77 labelled nodes, weighted by co-appearance count), Zachary karate club.

Families: profile, single metric, top-k, community, path, neighbourhood, comparative, compositional,
interpretation (directed/undirected, stored-both-ways, weights as strengths), clarification (under-specified
"most important"), node resolution (case, misspelling, partial names).

Scoring: answer score per family checker (tolerances, overlap@k, Jaccard, path validity, clarification credit);
plan score (expected operators present and expected assumptions stated; n/a for opaque code); cost
(LLM calls, tokens, USD, wall time).

## Running

```bash
pip install python-igraph networkx numpy pydantic
python bench/make_questions.py          # ground truth (about 1 minute)
python run_experiment.py --workers 3    # all systems, all questions
python run_experiment.py --score-only   # re-score existing raw results
```

Set `ANTHROPIC_API_KEY` to use the SDK backend; inside a Claude Code session the `claude -p` backend is used automatically.
`T2R_MODEL` overrides the model (default `claude-opus-5-5`).

## Limitations of this prototype

- One graph per question (no cross-graph comparison yet).
- Ground truth is igraph, cross-checked against NetworkX on a subset; community questions use tolerances because partitions are seed-dependent.
- The verifier covers a dozen invariants, not a general theory.
- No caching across questions, no MCP surface, no visualisation.
