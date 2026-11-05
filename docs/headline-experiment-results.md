# Headline Experiment: Results and What They Mean

Date: 2026-11-04. Code and raw data: `prototype/`. Summaries: `prototype/results/summary.md` (Opus 5.5, run 2),
`prototype/results/summary_run1_opus.md` (Opus 5.5, run 1, before fixes), `prototype/results_haiku/summary.md` (Haiku 5.5).

## 1. The claim under test

From `solution-architecture.md`: a typed, interpretation-aware, self-verifying plan beats both free code
generation and flat tool calling on analyst-style questions over real graphs, and the gain comes from the
interpretation and verification stages rather than the executor.

## 2. Setup

Five systems, same prompt inputs (file name, one-line dataset description where the question allows it, question), same model:

| system | interpretation | typed IR, validator, cost tiering | verifier |
|---|---|---|---|
| full | yes | yes | yes |
| no_verify | yes | yes | no |
| no_interp | no (raw file facts, strict node matching) | yes | yes |
| flat | no | no (tool-calling loop over the same 46 operators) | no |
| codegen | no | no (LLM writes a NetworkX or igraph script, one retry) | no |

Benchmark: 60 questions in 11 families over five real graphs (Facebook 4k nodes, Enron 37k, Epinions 76k directed,
Les Miserables weighted and labelled, karate club). Deterministic ground truth from igraph, cross-checked against
NetworkX on 16 questions. Three scores: answer (family checker), plan (expected operators and stated assumptions), cost.

## 3. Results

Answer accuracy, mean over 60 questions:

| model | full | no_verify | no_interp | flat | codegen |
|---|---|---|---|---|---|
| Opus 5.5, run 1 (before fixes) | 0.93 | 0.88 | 0.68 | 0.97 | 0.99 |
| Opus 5.5, run 2 | 1.00 | 1.00 | 1.00 | 1.00 | 0.99 |
| Haiku 5.5 | 1.00 | 1.00 | 1.00 | 0.93 | 0.99 |

Plan correctness (n/a for codegen): full 0.99, no_verify 0.99, no_interp 1.00, flat 0.93 (Opus); 0.98 / 0.98 / 0.98 / 0.95 (Haiku).

Cost per question (Opus 5.5): full 2.0 calls, 9k tokens, 16 s; flat 3.9 calls, 21k tokens, 25 s; codegen 1.1 calls, 3.4k tokens, 20 s.
With Haiku the flat agent needed 4.8 calls and 36k tokens, took 65 s, and timed out or ran out of turns on 3 questions.

Clarifications: the IR systems never asked one; flat and codegen asked on 2 to 3 questions each. The IR systems
instead computed the plausible variants and labelled them, which the checker accepts.

## 4. Reading the numbers honestly

**The headline claim is not confirmed by this experiment.** With a frontier model and a one-line dataset description,
every architecture answers essentially every question correctly. The benchmark saturates, so it cannot separate
the systems on answer accuracy. Three things did separate them:

1. **Robustness to a weaker model.** Flat tool calling fell to 0.93 with Haiku, with timeouts and turn exhaustion on
   compositional questions. The IR systems and code generation held at 0.99 to 1.00. A plan emitted in one shot is
   easier for a small model than a long tool-calling loop.
2. **Cost.** The IR systems use half the tokens of flat tool calling at the same accuracy. Code generation is cheapest
   of all, because one script replaces both planning and interpretation calls.
3. **Plan inspectability.** Only the IR systems produce a typed plan that can be scored, validated before execution,
   and cost-tiered. Flat tool calling scored 0.93 on plan correctness because it sometimes skipped stating
   assumptions or used extra tools; code generation cannot be scored at all.

Run 1 is the most informative run, and it argues against the claim in a different way. The 0.68 to 0.93 scores of the
IR systems were almost entirely bugs and expressiveness gaps in the prototype itself: node references resolved in the
wrong index space on subgraphs, no boolean operator for "is there a path", no field references, no set intersection,
single-step answers. The planner was right and the IR could not express what it wanted. Code generation had none of
these problems because Python already has all of that. This is the real cost of the IR approach: every missing
operator or type is a hard failure, where a code generator improvises.

## 5. Why the interpretation stage showed no effect

- Every question carried a one-line dataset description, which settles directedness and weight semantics for a
  frontier model without any inference. The no_interp ablation still received raw file facts including row
  reciprocity, and Opus inferred the semantics from those alone.
- The interpretation traps were too gentle. "Stored both ways" (Enron) and "weights are counts, not distances"
  (Les Miserables) were caught by every system, with or without a description.
- Node resolution was tested on five short labels. Code generation solved it with `difflib` in the script.

## 6. Why the verifier showed no effect

The verifier never fired on an error that changed an answer, because nothing it checks went wrong: partitions always
covered the graph, PageRank always summed to one, paths were always valid. Its one observable output was the
"two centralities disagree" warning, which the checker rewards equally whether it comes from the verifier or from the
planner's own variants. Verification pays off when the executor or planner is unreliable, and in this prototype neither was.

## 7. What this means for the research programme

The grounding gap is real in the literature but was not reachable with this benchmark. To make the claim testable:

1. **Withhold descriptions by default** and make the interpretation family larger and harsher: directed graphs stored
   with partial reciprocity, bipartite projections, weights that are distances in one file and strengths in another,
   duplicate rows that are real multi-edges, ID columns that are row numbers.
2. **Scale.** Nothing here exceeded 500k edges. At 10M to 50M edges, exact betweenness and diameter are infeasible,
   the cost model starts to matter, and code generation will time out or pick the wrong approximation.
3. **Compositional depth.** The compositional family had depth 2 to 3. Depth 4 to 6 questions with intermediate
   constraints are where flat tool calling ran out of turns with Haiku and where code generation starts to make
   silent logic errors.
4. **Adversarial verification cases.** Inject planner errors (wrong direction, wrong measure, wrong component) and
   measure the fraction the verifier catches. That is the RQ3 experiment, and it needs a fault-injection harness,
   not a clean benchmark.
5. **Weaker and cheaper models** as a first-class axis. The Haiku result is the only place the architecture earned
   its keep on accuracy, and the cheapest model that reaches a target accuracy is what a product will run.
6. **Report cost and inspectability as headline metrics** alongside accuracy. On this evidence they are where the
   architectures differ.

## 8. Reusable outcomes

- A working prototype of the full pipeline (loader, semantics inference, profile card, 46-operator catalogue, IR
  with validator and cost tiering, verifier, LLM planner and interpreter, two baselines) at `prototype/`.
- A 60-question benchmark with deterministic checkers for 11 families, a generator, and a runner that isolates every
  (system, question) pair in a subprocess and scores answer, plan and cost separately.
- Evidence that the IR must be at least as expressive as the questions: field references, set algebra, boolean
  predicates, multi-part answers, and subgraph-scoped node references were all required within the first 60 questions.
