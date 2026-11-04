# Solution Architecture, Novelty, and the Fundamental Problem

Date: 2026-11-04. Third document in the series, after `text-to-result-toolkit-assessment.md` and `benchmark-analysis.md`.

## 1. The fundamental problem

Computation is solved: igraph, NetworkX, and NetworKit compute every profiling-class metric in seconds. Language is mostly solved: current models parse analytical questions well. What is not solved is the gap between them. Call it the **grounding gap**. It has three parts, and none is addressed by "LLM calls graph tools", which is the 2026 consensus design and therefore no longer a contribution.

1. **Analytical vocabulary is many-to-many with algorithms.** "Most important node" maps to degree, PageRank, betweenness, closeness, eigenvector, k-core number, and which one is right depends on what the edges mean. "Community" maps to Leiden, Louvain, label propagation, SBM, k-clique, each with parameters. "Similar", "bottleneck", "hub", "well-connected", "robust" behave the same way. In text-to-SQL the ambiguity lives in the schema; here it lives in the measure. A plan can be well-typed and executable and still answer the wrong question.
2. **Graph semantics are implicit in the data.** An edge list does not say whether it is directed, whether direction means "follows" or "cites", whether weights are distances or strengths, whether the graph is a projection of a bipartite one, whether duplicate rows are multi-edges or noise, or whether node IDs are stable identifiers or row numbers. Each of these silently changes every downstream number. Relational tables carry this in their schema; graphs do not.
3. **There is no ground truth at query time.** In text-to-SQL, execution either succeeds or fails and the user judges the rows. In analytics, a plan can execute cleanly and return a confident, wrong number, and the user has no way to tell. Approximate and sampled algorithms make this worse.

The research question, in one sentence: **can the mapping from an analytical question to a graph computation be made reliable when the vocabulary is ambiguous, the data's semantics are implicit, and no ground truth exists at query time?**

Four concrete sub-questions:

- RQ1 (interpretation): how much of a graph's semantics can be inferred from the data and its profile, how well, and when must the system ask instead?
- RQ2 (measure grounding): how should analytical vocabulary be mapped to a distribution over measures given the profile and domain, and when should the system clarify versus compute the plausible variants and reconcile them?
- RQ3 (verification): how much of a planner's error can be caught without ground truth, using invariants between metrics, agreement between independent methods, and error bounds on approximations?
- RQ4 (cost-aware tiering): can a cost model over profile features pick exact, parallel, approximate, or sampled tiers to meet a latency budget with a stated error?

The headline experiment: a typed, interpretation-aware, self-verifying plan beats both free code generation and flat tool-calling on the benchmark from `benchmark-analysis.md`, and the gain comes mostly from the interpretation and verification stages rather than from the executor.

## 2. What is new

Not new in 2026: an LLM selecting from a catalogue of graph algorithms (GraphTool-Instruction, GraphSeek, Analytics-Augmented Generation, Memgraph 3.3, TigerGraph CoPilot). A proposal whose only content is "agent over igraph" will not be a contribution.

New, as far as the survey could establish:

1. **A typed Graph Analysis IR as the target of translation.** Cypher and GQL are pattern-matching algebras; analytics questions are not patterns. The IR is a dataflow over graph-native types (Graph, NodeSet, NodeMap, EdgeMap, Partition, Path, Scalar, Distribution) whose operators are algorithms with declared preconditions, cost, and approximation tiers. It is inspectable and editable, like a SQL plan, and engine-independent. Nobody in the survey has this; code-generation systems produce opaque Python and tool-calling systems produce flat call lists with no types and no composition.
2. **Interpretation as a first-class, scored stage.** Graph-semantics inference plus a clarification policy, evaluated separately from answer accuracy. GTools scores tool and parameter choice; nothing scores interpretation, and no tool handles node-name resolution.
3. **Verification without ground truth.** Invariant checking across metrics (component sizes sum to n, triangle count consistent with transitivity, betweenness within bounds, diameter at least the double-sweep lower bound), cross-method agreement (two centralities should produce correlated top-k on most real graphs, and a disagreement is a signal to show both), and approximation error bounds surfaced to the user. This is the analytics analogue of execution feedback in text-to-SQL, which catches crashes but not wrong answers.
4. **Cost-aware tiering with a cost model over the profile card.** Query optimisation for analytics with approximation, in the spirit of approximate query processing in databases but for graph algorithms composed by a planner.
5. **The profile card as the grounding context**, the schema-linking analogue, and the one-call profile report as a product feature nothing in the NetworkX or igraph ecosystem provides.
6. **The benchmark** that scores answer, plan, and cost separately over real graphs at three scales.
7. Optionally, the partitioned multi-agent layer for exploratory "understand this graph" sessions, built on top of the plan machinery rather than instead of it.

## 3. Architecture

```
                         user question (CLI, chat, notebook, MCP client)
                                          |
   +--------------------------------------v-----------------------------------------+
   | 1. INTERPRETATION                                                               |
   |    graph loader -> semantics inference (directed? weighted? multigraph?         |
   |    bipartite? temporal? ID type?) -> profile card -> node-resolution index      |
   |    -> clarification policy (ask / assume-and-state / compute variants)          |
   +--------------------------------------+-----------------------------------------+
                                          | profile card + resolved question
   +--------------------------------------v-----------------------------------------+
   | 2. PLANNER (LLM)                                                                |
   |    emits a typed Analysis Plan (DAG) in the Graph Analysis IR.                  |
   |    Never sees the graph. Never writes code on the main path.                    |
   |    Measure grounding: candidate measures ranked by profile + domain prior.      |
   +--------------------------------------+-----------------------------------------+
                                          | plan
   +--------------------------------------v-----------------------------------------+
   | 3. VALIDATOR / OPTIMISER (deterministic)                                        |
   |    type-check, precondition check, cost model -> exact/parallel/approx/sampled  |
   |    tier per op, runtime estimate, insert verification ops, cache lookups.       |
   |    Plan is shown to the user and editable before execution.                     |
   +--------------------------------------+-----------------------------------------+
                                          | executable plan
   +--------------------------------------v-----------------------------------------+
   | 4. EXECUTOR                                                                     |
   |    op catalogue bound to engines: igraph (default), NetworkX (long tail,        |
   |    GPU/parallel via dispatch), NetworKit (large graphs, approximations),        |
   |    pybind11-wrapped C++ where measurably faster.                                |
   |    metric cache keyed by (graph hash, op, params, tier); provenance log.        |
   +--------------------------------------+-----------------------------------------+
                                          | results + provenance
   +--------------------------------------v-----------------------------------------+
   | 5. VERIFIER (deterministic)                                                     |
   |    invariants, cross-method agreement, approximation error bounds.             |
   |    inconsistency -> re-plan or surface to user with both results.               |
   +--------------------------------------+-----------------------------------------+
                                          | verified results
   +--------------------------------------v-----------------------------------------+
   | 6. INTERPRETER (LLM)                                                            |
   |    every claim bound to a result ID; answer + table/chart + plan trace +        |
   |    stated assumptions and error bars.                                           |
   +---------------------------------------------------------------------------------+

   Side paths:
   - Code-generation fallback: when no plan type-checks, LLM writes NetworkX code in a
     sandbox with retrieved docs and retry; successful code can be promoted into the
     catalogue after human review (GraphSeek pattern).
   - Exploratory layer: partition the graph (Leiden / METIS), one agent per region with
     the full stack above, shared board of questions and evidence, routing by profile-
     card similarity; for "what is interesting here" sessions only.
   - Surfaces: the same catalogue and plans are exposed over MCP so any agent harness
     can use them.
```

### 3.1 The Graph Analysis IR, by example

Question: "Within the largest community, who are the ten most central people, and how far is each from Alice?"

```
interpret:
  g0 = load("social.csv", directed=false, weighted=false, multigraph=collapse)
       # inferred: symmetric edge rows (99.7%), no weight column; stated to user
  alice = resolve("Alice") -> node 4821 (exact label match, 1 candidate)

plan:
  P   = communities(g0, method=leiden, resolution=1.0, seed=1)   : Partition
  S   = largest_block(P)                                         : NodeSet
  g1  = induced_subgraph(g0, S)                                  : Graph
  bc  = betweenness(g1, tier=approx(pivots=500), seed=1)         : NodeMap<float>
       # "most central" grounded to betweenness (social, undirected);
       # alternative PageRank computed for agreement check
  pr  = pagerank(g1)                                             : NodeMap<float>
  top = top_k(bc, 10)                                            : NodeSet(ordered)
  d   = distances(g0, source=alice, targets=top)                 : NodeMap<int>

verify:
  sum(sizes(P)) == n(g0)
  bc values within [0, (n1-1)(n1-2)/2]
  overlap@10(top_k(bc,10), top_k(pr,10)) >= 0.5   # else show both rankings
  alice in reachable(g0) for each target, else d = inf stated

answer:
  table(top, bc, pr, d) + plan trace + assumptions + approx error note
```

The IR is what makes the system inspectable, cacheable, verifiable, engine-independent, and benchmarkable at the plan level. It is also what the planner is trained or prompted to emit, which is a much smaller target than Python.

### 3.2 Why not just generate code

Code generation is the strongest baseline, and it should be kept as the fallback. It loses on the main path for four reasons: the planner must be grounded in library APIs it misremembers (ProGraph), the output is opaque to the validator and verifier, it cannot be cost-tiered or cached at the operator level, and it cannot be scored for plan correctness on the benchmark. The IR gives up generality for inspectability, and the fallback recovers the generality.

### 3.3 Why not a query language

Cypher, GQL, and Gremlin are pattern-matching languages with algorithms bolted on as procedures. Analytical questions are algorithm compositions with semantic choices, not patterns. Targeting a pattern language forces the planner to express "most central" as a procedure call anyway, while adding a whole grammar the user never wanted. The IR is the minimal formal target for analytics, and the user never sees it unless they want to edit the plan.

## 4. Build order

The order follows the research questions, so each milestone produces evidence, not just code.

1. Interpretation layer and profile card over igraph; measure inference accuracy on a labelled set of real graph files (RQ1).
2. IR, validator, executor, and twenty operators; the benchmark's templated families and checkers; baseline numbers for flat tool-calling and code generation.
3. Planner with measure grounding and clarification policy; compare against baselines (RQ2).
4. Verifier; measure the fraction of injected planner errors caught (RQ3).
5. Cost model and tiering on the 1M and 50M edge tiers (RQ4).
6. MCP surface, profile report, code-generation fallback, exploratory multi-agent layer.
