# Experiment 2: Harder Interpretation, Scale, and Fault Injection

Date: 2026-11-05. Follows `headline-experiment-results.md`. Code in `prototype/`; summaries in
`prototype/results_hard/summary.md`, `prototype/results_hard_haiku/summary.md`, `prototype/results_scale/summary.md`,
`prototype/results/fault_injection.md`.

## 1. What was run

Three additions, designed to fail rather than to pass:

1. **Hard interpretation set.** 27 questions over nine new graphs with the dataset description withheld. The
   semantics must be read from the file: a road network whose weights are distances in kilometres; a similarity
   graph with fractional weights under the uninformative name `weight`; an email log with repeated sender-recipient
   rows and epoch timestamps (a directed multigraph); an edge list whose first column is a running row number; a
   bipartite customer-product file; a follow network with partial reciprocity and no header; a trust network with
   integer strengths; a citation network with `citing,cited` columns; the Facebook graph stored in both directions.
2. **Scale tier.** 16 questions over synthetic preferential-attachment graphs with 10M edges (undirected), 12M edges
   (directed) and 50M edges. SNAP was not reachable from the environment, so the graphs are synthetic. Descriptions
   were given: this tier tests cost tiering, not interpretation. Code-generation scripts run under an 8 GB memory cap (13 GB for the 50M graph).
3. **Fault injection.** The full system's 60 correct plans from the base benchmark were each mutated one way at a
   time (flip directedness, swap the centrality measure, flip in/out mode, substitute a node, drop the
   largest-component restriction, swap weak/strong components or community method, use strength weights as path
   lengths), executed, and passed to the verifier. Measured: whether the mutation changed the answer, and whether the
   verifier flagged it. No LLM calls.

Changes to the prototype needed for this round: header-aware column selection, row-index detection, bipartite
detection, multigraph support, weight semantics from column names, a pandas/numpy loading path for large files,
and nine new operators (distinct edge count, edge multiplicity, simplify, bipartite test and projection, strength,
heaviest edges, strongest ties, weighted betweenness). The verifier gained consistency checks between the question,
the stated interpretation, the inferred semantics and the plan.

## 2. Hard interpretation set: results

Answer accuracy, mean over 27 questions:

| model | full | no_verify | no_interp | flat | codegen |
|---|---|---|---|---|---|
| Opus 5.5 | 1.00 | 0.98 | 0.98 | 0.94 | 1.00 |
| Haiku 5.5 | 0.98 | 0.98 | 0.98 | 0.98 | 0.96 |

Plan correctness (Opus): full 0.98, no_verify 0.98, no_interp 0.98, flat 0.83. Cost per question (Opus): full 2.0
calls and 9.8k tokens; flat 4.1 calls and 24.7k tokens with one timeout; codegen 1.0 call and 3.1k tokens.

**Every trap was caught by every system, with or without a dataset description.** The both-ways file was
collapsed to 88,234 friendships by all five systems. The row-index column was skipped by all five. The email log was
read as a directed multigraph and the repeated rows counted correctly. The follow network was recognised as directed
from 33% reciprocity. The one systematic miss was the similarity graph, where three systems computed the right
hop count but did not say whether they had used the fractional weights.

Before the three IR gaps found in this round were fixed, the plan-based systems scored 0.91 on this set. The
gaps were: no operator for the heaviest edge of a projected graph, no weighted betweenness, and no field references
inside list literals. Code generation had none of these failures. This repeats the run-1 pattern exactly.

### Why the interpretation stage still shows no effect

The no-interpretation ablation and the code-generation baseline both receive the raw file facts: header, sample
rows, row count, distinct directed and undirected pair counts, reciprocity of rows, and whether a weight column is
numeric. Given those numbers, both Opus and Haiku infer the semantics correctly on their own. The rule-based
inference layer adds nothing a frontier model does not already do from the same evidence.

This narrows the claim usefully. The value of the interpretation stage is in **computing and surfacing the right
file statistics**, which is cheap and deterministic. The inference rules on top are redundant with the model. What
remains untested is a model that does not receive those statistics, which is how most code-generation agents are
deployed today (they see a file path and a question). That is a one-line ablation for the next round.

## 3. Fault injection: what the verifier catches

| mutation | injected | changed the answer | caught (of harmful) | catch rate | flagged although harmless |
|---|---|---|---|---|---|
| flip_directed | 60 | 35 | 35 | 100% | 25 |
| wrong_node | 22 | 17 | 17 | 100% | 5 |
| use_weights (strengths as distances) | 3 | 3 | 3 | 100% | 0 |
| swap_measure | 17 | 17 | 16 | 94% | 0 |
| mode_flip (in/out) | 7 | 7 | 4 | 57% | 0 |
| method_swap (weak/strong, community method) | 8 | 8 | 1 | 12% | 0 |
| drop_lcc | 4 | 0 | n/a | n/a | 1 |
| ALL | 121 | 87 | 76 | 87% | 31 |

Of the 76 catches, 5 came from the validator (the mutated plan no longer type-checked) and 71 from the verifier.
The catches come almost entirely from the **consistency checks** added this round, not from the numerical
invariants of round one: a stated directedness that contradicts the data, a node that the question never mentions,
a measure named in the question or interpretation that no step computes, strength weights used as distances. The
numerical invariants (partition covers the graph, PageRank sums to one, betweenness within bound) never fired,
because igraph does not produce such errors.

The 31 "harmless" flags are mostly flip_directed cases where the directedness was wrong but the particular answer
(for example an edge count on a graph stored once) happened not to change. They are correct detections of a wrong
interpretation, not false alarms in the usual sense.

The misses are informative. Swapping weak for strong components, or Leiden for label propagation, is invisible to a
verifier that has no ground truth and no question words to anchor on. Flipping in/out mode is missed when the
question names both directions. These are the cases where only a clarification to the user, or a second
independent computation, can help.

## 4. Scale tier: results

16 questions, Opus, one run per (system, question), single worker, 1800 s timeout per run. Ground truth for the
50M graph: exact counts, degrees, clustering and distance; diameter from 10 double sweeps; betweenness from 100
pivots (the reference ranking is itself an estimate, so X15 accepts overlap@10 >= 0.5).

| system | answer accuracy (16) | 10M/12M rows (10) | 50M rows (6) | LLM calls | tokens | USD | seconds | errors |
|---|---|---|---|---|---|---|---|---|
| full | 1.00 | 1.00 | 1.00 | 2.1 | 9852 | 0.050 | 293 | 0 |
| flat | 0.94 | 1.00 | 0.83 | 3.2 | 19099 | 0.053 | 312 | 0 |
| codegen | 0.97 | 1.00 | 0.92 | 1.0 | 3367 | 0.036 | 198 | 0 |

Per question on the 50M-edge graph (score, wall seconds including the ~5 minute load):

| qid | question | full | flat | codegen |
|---|---|---|---|---|
| X11 | nodes, edges, connected | 1.0, 352 s | 1.0, 492 s | 1.0, 257 s |
| X12 | top-10 degree | 1.0, 358 s | 1.0, 509 s | 1.0, 157 s |
| X13 | average clustering | 1.0, 394 s | 1.0, 480 s | 1.0, 251 s |
| X14 | diameter (rough) | 1.0, 603 s (approx, 20 sweeps) | 1.0, 718 s (approx, 20 sweeps) | 1.0, 294 s (iterated double sweep, plus an upper bound of 10) |
| X15 | top-10 betweenness (approx) | 1.0, 1216 s (125 pivots) | 0.0, 165 s (no tool called) | 0.5, 731 s (64 pivots) |
| X16 | hops between two nodes | 1.0, 362 s | 1.0, 496 s | 1.0, 185 s |

What the table shows:

- **Cost tiering worked without being asked.** On X15 the planner requested 500 pivots; the cost model's budget
  (1e10 edge visits) cut this to 125 before execution, the run finished in 20 minutes, and the top 10 overlapped the
  100-pivot reference at 0.5 (top 6 identical). On X14 the planner chose the approximate diameter tier itself.
  Nothing timed out.
- **The flat agent fabricated.** On X15 it returned no answer after a single LLM call and no tool call, with an
  explanation describing tool rejections ("won't accept fewer than 200 pivots", "500 pivots was also rejected",
  "top_k returned node 6 ahead of node 10") that never happened, and a "degree proxy" list it never computed (the
  listed nodes are not the top-degree nodes). The IR system cannot do this: an answer exists only if a validated plan
  executed. The scorer originally gave this run 0.5 because the invented proxy list happened to overlap the reference;
  variant credit now requires a primary answer, which changed only this row.
- **Code generation chose its own approximation and was good enough.** It wrote a 64-pivot sampled betweenness and
  an iterated double-sweep diameter with an explicit upper bound, both reasonable. Its betweenness ranking overlapped
  the reference at 0.4 (below the 0.5 bar; a computed degree-proxy variant at 0.6 earned half credit). Three sampled
  estimates with different seeds and pivot counts agree on the top 4 to 6 nodes and then diverge: the benchmark's
  reference for this question is only as sharp as its own sampling.
- **Wall time is dominated by loading, not reasoning.** Codegen's scripts load with pandas and igraph directly and
  are the fastest; the IR system pays for the profile card (degree statistics over 10M nodes) and the flat agent pays
  for one tool round trip per step. None of the systems is close to the 1800 s timeout except the betweenness runs.

Harness notes that matter for reproduction: the first 50M attempts were killed by the 15 GB memory cgroup. The
loader peaked at 14 GB (a Python list of 50M edge tuples); it now reads in chunks, builds the graph from numpy slices,
and peaks at 5 GB. The flat and codegen baselines were then killed because their file-facts step used the row-list
reader that the IR path had already stopped using; that was a harness bug, fixed and rerun, not a property of those
systems. SNAP was not reachable, so the graphs are synthetic preferential-attachment graphs, which are easy for
double-sweep diameter and have strongly hub-dominated betweenness; real graphs would spread the approximation error
more.

## 5. Conclusions and decision

The decision rule set before this round: if the full system beats code generation on the harder sets, the headline
claim is validated; if not, the product is code generation wrapped in the profile card, verifier and cost model, and
the benchmark remains the asset.

| set | model | full | no_verify | no_interp | flat | codegen |
|---|---|---|---|---|---|---|
| base (60) | Opus | 1.00 | 1.00 | 1.00 | 1.00 | 0.99 |
| base (60) | Haiku | 1.00 | 1.00 | 1.00 | 0.93 | 0.99 |
| hard (27) | Opus | 1.00 | 0.98 | 0.98 | 0.94 | 1.00 |
| hard (27) | Haiku | 0.98 | 0.98 | 0.98 | 0.98 | 0.96 |
| scale (16) | Opus | 1.00 | - | - | 0.94 | 0.97 |

On answer accuracy the full system does not beat code generation by a margin that two runs can distinguish: the
largest gap is three points on 16 questions, and the hard set is a tie. The headline claim as first stated ("a typed,
interpretation-aware, self-verifying plan is more accurate than code generation") is not supported by these
benchmarks, because every system reads the same file statistics and a strong model turns them into the right
semantics either way.

What the round did establish, and what is worth building on:

1. **Verification without ground truth is real and measurable.** The verifier flagged 87% of answer-changing plan
   mutations (76 of 87) using invariants and question/interpretation/plan consistency alone. Code generation has no
   equivalent: a wrong script that runs is indistinguishable from a right one.
2. **Cost tiering is the difference between an answer and a timeout at scale**, and the IR makes it automatic: the
   cost model overrode the planner's pivot count on the 50M graph and the run finished. Code generation got there by
   the model's own judgment this time; nothing guarantees it next time.
3. **Structured execution removes fabrication as a failure mode.** The flat agent invented a tool transcript and a
   proxy answer at 50M. A plan either validates and executes or returns a clarification.
4. **Interpretation inference adds nothing a strong model does not already do with the raw file statistics.** The
   value is in computing and surfacing those statistics (reciprocity, distinct pairs, headers, weight shape), not in
   the rule layer that names them. Keep the facts; drop the rules from the claim.
5. **The benchmark is still too easy.** Three sets of 60, 27 and 16 questions saturate for Opus. The questions that
   separated systems were the ones with contestable references (sampled betweenness) and the ones where an agent could
   drift (long tool chains with a weaker model: Haiku flat at 0.93 with timeouts).

Decision: build the product as the shared scaffold (profile card with file facts, cost model, verifier, clarification
protocol) around *both* executors, with the typed IR as the default path and code generation as the escape hatch for
questions the operator catalogue cannot express. The research claim moves from "IR beats codegen" to "verification and
cost control without ground truth", which is what the data supports, and the benchmark's next version has to target
what actually failed: contestable approximations, multi-step analyses whose intermediate choices change the answer,
weaker models, and an ablation of code generation without file facts (untested here; it is the one place the
interpretation stage could still show an effect).

