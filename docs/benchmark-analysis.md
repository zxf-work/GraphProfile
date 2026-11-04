# What Existing Graph-Reasoning Benchmarks Measure, and What a Text-to-Result Benchmark Would Need

Date: 2026-11-04. Companion to `text-to-result-toolkit-assessment.md`.

Access caveat: arXiv, OpenReview, Semantic Scholar, and proceedings sites were unreachable from this environment. Facts below come from reading benchmark repositories' source code and READMEs on GitHub (high confidence) and from search-engine excerpts of papers (medium confidence). Citation counts are approximate snapshots.

## 1. Three generations of benchmarks

### Generation 1 (2023-2026): can the model simulate an algorithm in its head?

NLGraph (NeurIPS 2023), GraphQA / Talk like a Graph (ICLR 2024), GraphInstruct / GraphWiz (KDD 2024), GraphOmni (ICLR 2026), GT-Bench (2026), GrAlgoBench (ICLR 2026).

| Property | Value |
|---|---|
| Graph delivery | Serialised into the prompt as an edge list or natural-language sentences |
| Graph source | Synthetic generators (Erdős-Rényi, Barabási-Albert, scale-free, trees) |
| Size | 5 to 60 nodes; GrAlgoBench pushes to 160 |
| Tasks | Textbook problems: connectivity, cycle, shortest path, topological sort, max flow, bipartite matching, Hamilton path, triangle count, diameter, MST, Euler path |
| Questions | Templated, fully specified, single turn |
| Answer | One value or yes/no, string-matched |
| State | Saturated. Code-executing agents report 97 to 99% on NLGraph and near 100% on GraphQA-style tasks |

What they actually measure: working memory and parsing. GraphOmni and GT-Bench show accuracy moves by tens of points when only the serialisation changes, which means a large share of the score is "did the model read the edge list correctly", not "does it understand graphs". These are useful as cognitive probes of LLMs. They are not a workload anyone has in practice, since nobody pastes a 30-node edge list into a chat to get a max flow.

### Generation 2 (2024-2025): can the model write code against a graph library?

ProGraph (NeurIPS 2024 D&B), GraphEval36K (NAACL 2025 Findings), GraphAlgorithm (2025, competition problems from Codeforces, AtCoder, CodeChef, Kattis).

What they measure: API recall and algorithm design. ProGraph's lesson (36% baseline, +11 to 32 points with documentation retrieval) is that models forget library signatures. GraphAlgorithm's lesson (Simple-RTC) is that models do fine once told to design the algorithm before coding. Both are closer to a text-to-result workload, but ProGraph is small (512 questions), uses GPT-based answer checking, and has 35 stars and about 13 citations. GraphAlgorithm is competitive programming, where the "graph" is a puzzle input, not a dataset.

### Generation 3 (2025-2026): can an agent operate tools over a real graph?

GTools / GraphTool-Instruction (KDD 2025), GraphChain's evaluation (NeurIPS 2025), Neo4j GDS Agent bench (2025), GABench (Aug 2026).

- GTools measures three things separately: graph extraction, tool-name selection, tool-parameter extraction. That decomposition is the right idea. The data is synthetic (2 to 100 nodes), templated, and the repo has 3 stars.
- GraphChain evaluates learned tool sequences on real graphs up to 200k nodes across five domains. It is a method paper's private evaluation, not a shared benchmark.
- GDS Agent bench is one graph (London Underground) and measures tool precision and recall.
- GABench is the closest thing to a text-to-result benchmark: 13 real datasets, 84 executable tools, 10,400 generated tasks across graph retrieval, graph theory, graph ML, and open-ended QA, run under three agent harnesses. Headline success rates are 34 to 38%. Weaknesses as far as can be verified: no code or data release found, the graph-ML tools return a fixed accuracy rather than training anything, open-ended QA is scored by an LLM judge, graph sizes are not published, and tasks come from a generation pipeline rather than from analysts.

## 2. Do they make sense?

For their authors' purpose, yes. Each generation was designed to expose a failure of the previous approach, and each paper states that explicitly:

- GraphWiz: prior graphs had fewer than 20 nodes and 100 edges.
- GraphArena: prior graphs were synthetic; need real graphs and a hallucination-versus-suboptimal breakdown.
- ProGraph: reasoning over graph text caps size at a few dozen nodes; experts write programs instead.
- GraphOmni and GT-Bench: fixing one serialisation misstates capability.
- Simple-RTC: benchmarks are easy once models design rather than replicate algorithms.
- GABench: prior work treats graph analysis as text QA with the graph in the prompt; agents must reach the data through tools, and harness and tool-call quality dominate.

For the purpose of judging a text-to-result analytics tool, they mostly do not. The mismatches:

1. **Wrong failure mode.** In a tool-backed system the algorithm never runs inside the model, so "can the model trace BFS" is irrelevant. The failures that matter are choosing the wrong algorithm, wrong parameters, wrong graph interpretation (directed, weighted, multigraph), wrong node, or a wrong explanation of a right number. Only GTools scores any of these, and only on synthetic data.
2. **Wrong workload.** No benchmark contains the questions analysts ask: profile this dataset, rank the top-k by centrality, find and describe communities, find outliers, compare two graphs, answer a multi-step compositional question, or ask for clarification when the question is under-specified. Every benchmark asks about one graph, one turn, fully specified. Path and reachability are the only workload every benchmark covers.
3. **Wrong scale.** Everything except GABench and GraphChain stays under 200 nodes. Real profiling targets start at tens of thousands of edges, where exact-versus-approximate choices and runtime become part of correctness.
4. **Wrong scoring.** Single-value exact match cannot score a ranking, a community partition, an approximate diameter, or a free-text profile. Nobody uses overlap@k, rank correlation, relative error, or partition similarity. GABench's answer for open-ended questions is an LLM judge, which is exactly what a verifiable-computation benchmark should avoid.
5. **Nothing on cost.** None reports tokens, latency, or tool-call count as a first-class metric alongside accuracy, although GABench has a token-efficiency table.

## 3. What gets adopted, and why

Reuse is concentrated: NLGraph (roughly 300 to 535 citations, 149 stars) and GraphQA (125 to 483) dominate; GraphWiz and GraphArena follow; ProGraph, GTools, GraphEval36K, GT-Bench are barely reused. The adopted ones share five properties:

1. Cheap to run: a few thousand items, no sandbox, no library installs, answers checked by string or number match.
2. Data checked into GitHub or Hugging Face with a README results table. None has a hosted leaderboard; GraphArena's README table is the closest.
3. A seeded generator, so the benchmark doubles as a training-data source. Method papers need train/test splits, and NLGraph, GraphInstruct, GraphQA, and GT-Bench all provide generators.
4. A small number of tasks with obvious names, so results fit in one table.
5. An early mover in its generation. Later, more realistic benchmarks (GraphArena over NLGraph, ProGraph over GraphWiz) are used as held-out generalisation tests rather than as the main target.

The realistic-but-heavy benchmarks fail on point 1: they require code execution, GPT-based grading, or library setup, and their numbers are hard to compare across papers.

## 4. Why a new benchmark makes sense

The gap is specific and currently empty: **analyst-style questions over real graphs at multiple scales, with the graph never in the prompt, answers verified deterministically, and the plan scored separately from the answer.** GABench occupies the nearest cell but is unreleased, judge-scored for open questions, and generated rather than analyst-authored. GTools has the right scoring decomposition on the wrong data. Nothing scores profiling, ranking, community, comparison, composition, or clarification.

A benchmark is also what makes the toolkit itself credible. Without one, every design choice in the planner is argued by anecdote.

## 5. Design for adoption

Learn from section 3 and from the gaps in section 2:

**Workload (task families, each with a deterministic checker):**

| Family | Example question | Checker |
|---|---|---|
| Profile | "Describe this graph" | Set of required facts (n, m, density, components, degree stats) each checked numerically |
| Single metric | "What is the diameter?" | Exact, or relative error for approximate tiers |
| Top-k ranking | "Ten most central nodes by betweenness" | Overlap@k and rank correlation against exact |
| Community | "What communities exist and how big is the largest?" | Partition similarity (NMI or ARI) above a floor, modularity within tolerance |
| Path and reachability | "Shortest path from Alice to Bob" | Path validity and length |
| Neighbourhood | "Who is within two hops of X with degree above 50?" | Set equality |
| Comparative | "Is graph A more clustered than B?" | Boolean plus the two numbers |
| Compositional | "Largest community, its most central member, that member's distance to X" | Final value plus intermediate values |
| Interpretation | Questions where directed-vs-undirected or weighted-vs-not changes the answer | Correct only if the stated interpretation matches the file's declared semantics, or if the system asks |
| Clarification | Under-specified questions ("most important node") | Credit for asking a disambiguating question or for computing and labelling the plausible variants |
| Node resolution | Questions naming nodes by label, alias, or misspelling | Correct entity resolved |

**Scoring on three axes, reported separately:** answer correctness (family-specific checker above), plan correctness (algorithm chosen, parameters, graph interpretation, in the style of GTools' three metrics), and cost (tool calls, tokens, wall time). A single leaderboard number can be a weighted combination, but the axes must be visible.

**Data:** real graphs from SNAP, Network Repository, and KONECT at three scale tiers, roughly 10k, 1M, and 50M edges, with labelled nodes where available so node-resolution questions are possible. Ground truth computed once by igraph and NetworkX, cross-checked, and stored. No LLM judge anywhere.

**Delivery:** the graph is a file plus a small profile card; the agent reaches it only through tools. Ship a reference MCP tool server and a reference baseline agent so any harness (Claude Code, OpenClaw, LangChain, a bare function-calling loop) can run it in an afternoon. This is the point where ProGraph and GTools lost adoption.

**Question authoring:** a seeded generator for the templated families (this is what gets a benchmark reused as training data), plus a hand-written and paraphrased subset per family so the test split is not just templates. Publish the generator.

**Size and cost:** a core split of about 1,000 items that runs in under an hour on the small tier, and a full split for papers. Results table in the README from day one, with the reference agent's numbers.

**Positioning:** do not compete with GABench on graph ML or open-ended QA. Position as the analytics and profiling complement, with deterministic scoring and scale tiers, and evaluate your own toolkit on it in the same paper.

## 6. Risks

- The group behind ProGraph, GraphTeam, and GABench will likely release GABench's code and could extend it toward analytics. Moving first with a released, runnable artefact matters more than breadth.
- Deterministic checkers for communities and approximate metrics need tolerance thresholds that someone will dispute. Publish the thresholds and the sensitivity analysis.
- Clarification scoring is new and will need a small human-labelled set to validate.
- A benchmark tied to one toolkit looks self-serving. The MCP tool server must be library-neutral and the baseline agent must be trivial.
