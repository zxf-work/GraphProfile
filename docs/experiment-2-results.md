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
   were given: this tier tests cost tiering, not interpretation. Code-generation scripts run under an 8 GB memory cap.
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

(filled in below once the runs complete)

