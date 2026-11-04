# Text-to-Result Graph Analysis Toolkit: Codebase Assessment and Landscape Survey

Date: 2026-11-04

## 1. Summary

**Verdict: reconstruct, do not reuse the code. Reuse the ideas, the test data, and the benchmark numbers.**

- Both repositories are 2017 research harnesses, not libraries. Neither exposes a callable API; both are driven by interactive `stdin` prompts or by editing `main()` and recompiling.
- GraphProfile is correct (its results on the bundled Facebook graph match SNAP's published statistics) and still compiles today in sequential mode, but it is slower than off-the-shelf python-igraph by one to two orders of magnitude on the same graph. Its multithreaded (TBB) variants contain data races.
- GRabbit is a bidirectional-BFS / landmark-index experiment for point-to-point distance queries on SNAP-scale graphs. It has no build system, no API, no tests, and hardcodes experiment selection in `main.c`.
- The text-to-result layer you want does not exist in either repo. Both only accept a fixed command vocabulary (`cc`, `etri`, `adiam`, ...). That vocabulary, and the "profile a graph with a standard metric set" intent, is the reusable design asset.
- Recommended direction: a Python package where an LLM plans over a typed catalogue of graph-analysis tools, executed by an in-memory engine (igraph first, NetworkX as the fallback and for long-tail algorithms, with a path to GPU or C++ later). Details in section 5.

## 2. What GraphProfile is

Repository: `zxf-work/GraphProfile`, last commit 2017-11-29. 4,300 lines of C++14 on Boost Graph Library 1.63 (vendored, 658 MB in-tree) plus optional Intel TBB. Authors: Vincent Luong (first half of 2017, algorithms and analysis) and Boshen Cui (second half of 2017, TBB parallelisation).

### 2.1 Capabilities

Metrics (command name in parentheses):

| Metric | Command | Implementation |
|---|---|---|
| Vertex / edge count | `vert`, `edge` | Boost |
| Degree list | `degrees` | custom |
| Connected components (count and sizes up to 90% coverage) | `cc` | Boost `connected_components` |
| Exact triangle count | `etri` | custom neighbour-pair enumeration |
| Local / average clustering coefficient | `lcc` | custom |
| Approximate diameter (1000 random double-sweeps) | `adiam` | custom |
| Exact diameter (APSP, O(V^2) memory) | `ediam` | Boost Johnson / Floyd-Warshall |
| Exact diameter, low memory (V BFS runs) | `emdiam` | custom |
| Diameter upper and lower bound via spanning trees | `kdiam` | custom |
| PageRank, exact and approximate | `prank`, `aprank` | custom power iteration |
| Betweenness centrality | `bbc` (Boost Brandes), `bc` (custom O(V^3) space, crashes on large graphs), `abc` (approximate, single vertex) | mixed |
| Onion / k-core decomposition | `od` | custom, O(V) rescans per layer |
| Point-to-point distance | `dist` | BFS |
| Reachability of random pairs | inside suite tests | BFS visitor |
| Graph reduction (keep top-k neighbours, spanning-tree union, triangle-avoiding, percentage cutoff) | `reduce*` | custom, the project's research contribution |

Supporting assets worth keeping:

- `Graph Data/`: SNAP edge lists (facebook_combined, enron, epinions) plus tiny test graphs. Good regression fixtures.
- `GraphProfile Analysis/Manual.pdf` and the two analysis folders: a written spec of what "profiling a graph" meant to the team, and measured accuracy of each approximation.
- `benchmark_results.txt`: sequential vs TBB timings on a 56-core machine.

### 2.2 Verification done in this session

Built the sequential variant with g++ 13 against the vendored Boost. Compilation succeeded with no source changes (37 s). Ran it on `facebook_combined.txt` and compared with NetworkX 3.x and python-igraph on the same machine:

| Metric | GraphProfile (seq) | NetworkX | python-igraph |
|---|---|---|---|
| Triangles | 1,612,010 in 3 s | 1,612,010 in 0.32 s | 1,612,010 in <0.01 s |
| Avg clustering | 0.6055 in 4 s | 0.6055 in 2.1 s | 0.6055 in 0.02 s |
| Diameter | approx 8 in 105 s (1000 sweeps) | exact 8 in 27 s | exact 8 in 1.2 s |
| Betweenness, all vertices | not run (`bc` is O(V^3) space) | - | 2.3 s |

All values match SNAP's published statistics for this graph, so the implementations are correct. The 2017 code is slower because `boost::adjacency_list<setS, vecS>` has poor locality and the custom loops are naive (for example the onion decomposition rescans every vertex per peel, and `edge(u, v, g)` lookups dominate triangle counting).

### 2.3 Problems that block reuse

- **No API.** All logic is reachable only through `readCommand()`, which reads `std::cin`, prints to `std::cout`, and appends to files named after the input graph. Every algorithm function takes a `const Graph&` typedef'd to one specific Boost type, so the metric code cannot be lifted without the Boost type and the surrounding I/O.
- **Undirected only.** `common.h` fixes `boost::undirectedS`. Directed metrics (in/out degree, SCC, directed PageRank) are not available even though the README lists in/out degree.
- **Data races in the TBB code.** `approx_graph_diameter_multithread` writes shared `diameter` and `lowerdiam` from parallel iterations without atomics or reduction; `connected_comp_multithread` increments a shared `std::vector<unsigned>` from `parallel_for_each`; `exact_triangle_count_multithread` declares an unused mutex and builds neighbour vectors with `parallel_for_each` (correct but pointless). The published speedups are therefore partly unsafe.
- **Build system is non-portable.** Makefile hardcodes a University of Waterloo home-directory TBB path; TBB's API used here (`task_scheduler_init`, `tbb::mutex`) was removed in oneTBB 2021.
- **Vendored Boost 1.63** (2016) is 658 MB of the repo and far behind current Boost 1.86+.
- **No tests** beyond the `testall.in` stdin script. `suite_test.cpp` is a benchmarking driver, not a test.
- Node IDs must be dense integers starting at 0; the reader `add_edge`s by integer, so a file with ID 10,000,000 allocates ten million vertices. No labels, no attributes, no weights beyond a constant 1.

## 3. What GRabbit is

Repository: `zxf-work/GRabbit`, last commit 2017-07-25. 4,800 lines of C. README says only "One-hop, two-hop, ...". Authors: Xiaofei Zhang (zxf), Will.

### 3.1 Capabilities

- CSR-style adjacency construction from an edge list (`util.c`), for undirected (`Vertex`) and directed (`DVertex`) graphs, with vertex and edge counts read from `graph.cfg`.
- About twenty variants of bidirectional BFS for single-pair shortest distance (`bfs.c`, `dgbfs.c`): naive, degree-ordered frontier expansion (`drtbfs`), vertex-vs-edge frontier balancing (`vvbfs`, `eebfs`), k-hop limited, k-unvisited pruning, degree-reduced graph variants.
- Landmark BFS trees and landmark-based distance bounds (`bfstree.c`, `lmtest.c`); the last commit notes the undirected landmark test is buggy.
- Four-sweep diameter lower bound (`profile.c`, the only "profile" function).
- x-hop neighbourhood counting (`neighbor.c`).
- Experiment harness over SNAP graphs (`graph.cfg` lists LiveJournal, YouTube, web-BerkStan at 0.7 M to 5.4 M vertices, 6.6 M to 78 M edges).

### 3.2 Assessment

This is the raw material of a 2017 research paper on distance-query heuristics, not a toolkit. Experiment selection is by commenting and uncommenting calls in `findpath.c` and `main.c`; results go to `printf`. There is no Makefile, no header per module (functions are declared implicitly across files), `char gfile[16]` limits file names to 15 characters, and the queue is a fixed-size array. The one idea with lasting value, degree-aware bidirectional BFS for point-to-point queries on power-law graphs, is already what igraph and NetworKit do internally, and GRabbit's landmark indexing would only matter if the toolkit needs millions of distance queries per second, which a profiling tool does not.

**Reuse verdict: none of the code. Keep the repo as a reference for the experiment design.**

## 4. What neither repo has, that the goal needs

| Needed for text-to-result | GraphProfile | GRabbit |
|---|---|---|
| Natural-language front end | no (fixed commands) | no |
| Planner that maps a question to one or more algorithms | no | no |
| Callable library API (Python or C) | no | no |
| Machine-readable results (JSON) | no (prose to text files) | no |
| Directed, weighted, attributed, labelled graphs | undirected only, weight fixed to 1 | directed/undirected, no weights |
| Node-ID or name resolution | dense ints only | dense ints only |
| Community detection, assortativity, degree distribution stats, SCC, k-core numbers per node, eigenvector / closeness centrality | no | no |
| Caching of computed metrics across questions | no | no |
| Tests | no | no |
| Visualisation | no (a separate "GraphDisplay" is mentioned in a comment and not present) | no |

## 5. Recommended direction

### 5.1 Reconstruct as a Python package with a typed algorithm catalogue

Proposed shape (one package, four layers):

1. **Graph I/O and schema layer.** Load edge lists, CSV, GraphML, JSON, pandas DataFrames. Detect or ask: directed vs undirected, weighted, multigraph, node-ID type. Build a *profile card* (node and edge counts, density, directedness, attribute names and types, sample node IDs, degree summary). This card is the only thing about the graph the LLM ever sees.
2. **Algorithm catalogue.** 20 to 30 typed tools, each with a JSON schema, a one-paragraph description, preconditions (for example "requires connected graph", "exact variant is O(V*E)"), a complexity tier, and an exact and an approximate implementation where sensible. Initial set, mirroring the GraphProfile manual plus the gaps in section 4: basic stats, degree distribution, components (weak and strong), diameter and effective diameter, triangles, clustering, k-core / onion layers, PageRank, betweenness, closeness, eigenvector, communities (Leiden / Louvain), assortativity, shortest path and distance, reachability, k-hop neighbourhood, MST, top-k by any metric, and subgraph extraction.
3. **Planner.** LLM pipeline: classify intent, pick tools, extract parameters, resolve node references against the ID namespace, clarify when ambiguous, execute, then interpret the numbers. Code generation against NetworkX in a sandbox is the fallback for questions the catalogue cannot answer, with library docs retrieved into the prompt and an execution-error retry loop.
4. **Presentation.** Answer as natural-language summary plus a table or chart, with the plan shown (which algorithm, why, parameters, runtime, exact or approximate). Expose the same catalogue over the Model Context Protocol (MCP) so it works inside Claude Desktop, Cursor, and similar clients without a bespoke UI.

### 5.2 Engine choice

- **python-igraph 1.0** as the default CPU engine. In this session's measurement it was 10x to 100x faster than both NetworkX and the 2017 C++ code on profiling-class metrics, has Leiden / Louvain / Infomap, assortativity, k-core, and ships pip wheels on all platforms.
- **NetworkX** for the long-tail algorithms igraph lacks, for the code-generation fallback (it is the API LLMs know best), and as the path to parallel and GPU execution through its backend dispatch (nx-parallel, nx-cugraph).
- **NetworKit** worth evaluating later for graphs above roughly 50 M edges and for its approximation algorithms (ApproxBetweenness, EstimateDiameter). It also already has an automated profiling report, which is the closest existing thing to the "profile" half of your goal.
- **Your C/C++ code**: wrap via pybind11 only if a specific routine is measurably faster than igraph for a given metric. From the measurements above, none currently is. The GRabbit degree-ordered bidirectional BFS could become a specialised distance-query tool later if the toolkit needs millions of pair queries, but that is not a profiling workload.

### 5.3 What to carry over from the old repos

- The command vocabulary and the manual: they define the first draft of the tool catalogue and its documentation strings.
- The exact-vs-approximate pairing (adiam / ediam / emdiam, prank / aprank, bc / bbc / abc) as the pattern for scalability tiers.
- The graph-reduction experiments as a candidate "sampling / sketch the graph" tool for very large inputs.
- The SNAP fixtures and the benchmark table as regression and performance baselines.
- The four-sweep diameter lower bound in GRabbit's `profile.c` as a cheap diameter estimator for huge graphs.

### 5.4 Suggested first milestones

1. Package skeleton, loaders, profile card, and ten catalogue tools on igraph, each with a unit test on `facebook_combined.txt` and the two tiny fixtures.
2. Planner with typed tool calling over those ten tools, plus a 50-question evaluation set written against the fixtures (ground truth from igraph).
3. MCP server exposing the catalogue; CLI that prints plan and answer.
4. Remaining tools, approximate tiers, node-ID resolution, clarification, and the one-call profile report.
5. Code-generation fallback with sandboxing, then run against GABench or ProGraph subsets to measure against published systems.

## 6. Landscape survey

Method: about 35 web searches plus GitHub README reads. arXiv full text was not reachable from this environment, so paper numbers below come from proceedings pages, OpenReview, blogs, and GitHub, and are marked secondhand where they are.

### 6.1 Natural-language to graph analysis without a query language

Research systems, oldest to newest:

- **Graph-ToolFormer** (2023, https://arxiv.org/abs/2304.11116). First paper to frame "LLM emits calls to graph-reasoning tools" instead of reasoning in context. Preliminary results only.
- **NLGraph** (NeurIPS 2023, https://github.com/Arthur-Heng/NLGraph). Benchmark of 29k in-context graph problems with 5 to 35 nodes. LLMs only 37 to 58 points above random on connectivity, cycle, shortest path.
- **Talk like a Graph / GraphQA** (Google, ICLR 2024, https://research.google/blog/talk-like-a-graph-encoding-graphs-for-large-language-models/). Showed accuracy swings of 5 to 60 points depending purely on how the graph is serialised into text.
- **GraphWiz / GraphInstruct** (KDD 2024, https://github.com/Graph-Reasoner/Graph-Reasoning-LLM). Instruction-tuned LLaMA reasoning in context; models tap out at tens of nodes. An anti-pattern for your goal.
- **ProGraph + LLM4Graph** (NeurIPS 2024, https://github.com/BUPT-GAMMA/ProGraph). Benchmark where the LLM must write code against NetworkX and five other libraries. Best model at release scored about 36%; retrieving library documentation into the prompt or fine-tuning on it added 11 to 32 points. Lesson: LLMs misremember library APIs, so ground them with docs or typed schemas.
- **GraphTeam** (2024, revised Nov 2025, https://arxiv.org/abs/2410.18032v4). Five agents: normalise input, retrieve API docs and past experience, write Python, retry on execution error, fall back to pure reasoning. Reported +15 points over the best baseline; later work notes it fails outside its knowledge base.
- **GCoder** (2024-25, https://github.com/Graph-Reasoner/GCoder). Code-generating LLM trained with compiler feedback; +16 points over GPT-4o. Critics observe it mostly learns to call graph-library APIs, which supports the catalogue approach.
- **GraphArena** (ICLR 2025). GPT-4o hallucination on diameter rises from 16% at 5 nodes to above 80% at 30 nodes; letting models execute code cut hallucination sharply.
- **GraphTool-Instruction / GTools / GraphForge** (KDD 2025, https://dl.acm.org/doi/10.1145/3690624.3709238). Decomposes the task into graph extraction, tool-name selection, and tool-parameter extraction over 20 graph tasks. This is the published pipeline closest to your plan.
- **GraphAgent** (HKUDS, EMNLP 2025, https://github.com/HKUDS/GraphAgent). Generator, planner, and executor agents; mostly about building knowledge graphs from text, not computing metrics.
- **Simple-RTC** (Sept 2025, https://arxiv.org/abs/2509.24260v2). Argues "design the algorithm first, then write code" with a cheap model beats both reasoning-only and code-augmented systems.
- **GRRAF** (Sept 2025, https://arxiv.org/html/2509.12743). Zero-shot: LLM writes NetworkX code with error feedback and timeouts.
- **GraphOmni** (ICLR 2026, https://github.com/GAI-Community/GraphOmni). No serialisation or prompt scheme wins consistently for in-context reasoning.
- **Analytics-Augmented Generation** (Feb 2026, https://arxiv.org/html/2602.21604). Argues free code generation cannot deliver reliable graph analytics and proposes an algorithm-centric model where the LLM composes verifiable analytics modules. This is your design thesis stated as a paper.
- **GraphSeek** (Feb 2026, https://arxiv.org/html/2602.11052). The agent never sees raw graph content; a non-LLM executor runs tools from a library; LLM-synthesised new tools are gated behind human approval before promotion.
- **GraphSkill** (Mar 2026, https://github.com/fairyfali/graphskill). Documentation-guided hierarchical retrieval for graph coding.
- **GABench** (Aug 2026, https://arxiv.org/abs/2608.01684v1). 10,400 verifiable tasks for agents with an executable graph toolset. Finding: agents still struggle on complex tasks, harness design dominates, and "tool-call quality matters more than quantity". The benchmark to evaluate against.
- **Survey: Are LLMs Suitable for Graph Computation?** (June 2026, https://arxiv.org/html/2606.06865v1) and the curated list https://github.com/YuanchenBei/Awesome-Graphs-Meet-Agents.

Open-source prototypes with the same shape as your toolkit:

- **GraphQA by catio-tech** (https://github.com/catio-tech/graphqa). LangChain ReAct agent over an in-memory NetworkX graph with five tools (schema discovery, query, statistics, node analyser, algorithm selector). README shows an explicit question-to-algorithm table and prints the rationale and runtime. Experimental, 11 commits.
- **NxLU** (https://pypi.org/project/nxlu/). Registers as a NetworkX dispatch backend; profiles the graph, selects algorithms via zero-shot classification plus rule maps, runs them, and has an LLM explain the results. Clever entry point.
- **networkx-mcp-server** (https://github.com/brightlikethelight/networkx-mcp-server). MCP server exposing about 20 NetworkX operations including a PNG visualiser. Thin, but proves the MCP route.
- **gephi-ai** (https://github.com/poldham/gephi-ai). Gephi plugin plus MCP bridge with natural-language querying and undo snapshots.
- **LangChain and LlamaIndex** have no first-party NetworkX analytics tool. Their graph integrations are Cypher, Gremlin, and SPARQL QA chains.
- **Microsoft GraphRAG** (https://github.com/microsoft/graphrag) is the complement, not a competitor: it extracts a graph from documents and answers text questions over community summaries. Your toolkit takes a graph as input and returns metrics.

### 6.2 Commercial natural-language interfaces over graph databases

These all generate a query language underneath, but their UX conventions are what users will expect:

- **Neo4j Copilot and Aura Agent** (2024-25, https://www.neo4j.com/developer/genai-ecosystem/aura-agent): generated Cypher is shown and editable before execution; the agent shows its reasoning.
- **Memgraph Lab GraphChat** (https://memgraph.com/blog/memgraph-lab-3-3): every intermediate step is displayed, including the error-recovery loop. Version 3.3 moved from always generating Cypher to selecting from a catalogue of algorithm procedures. Docs are candid that generation fails when the schema lacks semantics.
- **TigerGraph CoPilot** (https://www.infoworld.com/article/2337106/how-tigergraph-copilot-enables-graph-augmented-ai.html): maps the question to the best pre-written query rather than generating one, returns text plus a visualisation.
- **Linkurious 4.3 Query AI** (https://linkurious.com/blog/linkurious-enterprise-4-3/): investigators inspect, edit, and save the generated query.
- **Graphistry / Louie.AI** (https://louie-py.readthedocs.io/): responses are typed elements (dataframes, charts, GPU graph renders) and reach data through MCP.
- **Kuzu** was archived in Oct 2025 and continues as LadybugDB (https://thedataquarry.com/blog/from-kuzu-to-ladybug/). The embedded graph-database space is in flux, which strengthens the case for a database-free design.
- No LLM feature was found for Cytoscape or Cosmograph.

Recurring patterns: chat panel beside a graph canvas; show the plan or query and let the user edit before running; show intermediate steps and error fixes; answer as summary plus table plus visual; schema grounding is the top failure cause; the newest products move from free-form generation toward a curated tool catalogue.

### 6.3 Compute libraries, state in late 2026

| Library | Status | Fit |
|---|---|---|
| NetworkX 3.5+ | Pure Python, BSD. Backend dispatch to nx-parallel, nx-cugraph, graphblas-algorithms. | Widest coverage, API LLMs know best, slowest. |
| python-igraph 1.0 (Sept 2025) | C core, GPL, pip wheels everywhere. | Fast, full profiling coverage incl. Leiden, assortativity, k-core. Recommended default. |
| NetworKit 11.2 (Nov 2025) | C++ OpenMP, MIT. | Large graphs, approximation algorithms, has a built-in `profiling.Profile` HTML report. |
| graph-tool | C++/Boost, no pip wheels, version unverified. | Fastest and has SBM inference; heavy install. |
| rustworkx 0.18 (2026) | Rust, Apache-2.0. | Fast paths and centralities; thin on communities and assortativity. |
| nx-cugraph (RAPIDS 26.08) | GPU backend for NetworkX, Linux only. | Centralities, communities, components, k-core, paths. Lacks diameter, closeness, assortativity, MST. |
| SNAP / snap.py | Last release Dec 2020. | Avoid. |
| Boost Graph Library | Shipped in Boost 1.89-1.92; C++ Alliance funded renewed maintenance in 2026. | Alive but no Python bindings; wrap only specific routines. |
| DuckDB DuckPGQ | Community extension, incomplete, and it is a query language. | Useful for edge-list ingest and filtering only. |

### 6.4 Design patterns that transfer from text-to-SQL and data agents

- Schema linking is the biggest single predictor of success in text-to-SQL; for graphs the "schema" is directedness, weights, multigraph-ness, node-ID namespace, and attribute names.
- Execution feedback catches crashes but not "runs but wrong"; verifiable typed tools close that gap.
- Enterprise text-to-SQL agents score only 6 to 31% on Spider 2.0 versus about 73% on BIRD; exploration and clarification, not syntax, is the bottleneck. PRACTIQ (NAACL 2025) formalises a clarify-then-answer protocol for ambiguous questions.
- Code actions (CodeAct, smolagents CodeAgent) beat JSON tool calls on complex multi-step tasks but require sandboxing; JSON tool calls win for fixed-schema systems. The 2026 graph papers converge on the hybrid: typed tools by default, code synthesis as an escape hatch.

## 7. Feature checklist for the design

Must have:

- Graph never enters the prompt; only the profile card and computed results do.
- Typed catalogue of 20 to 30 well-described tools with exact and approximate variants and stated preconditions.
- Planner pipeline: intent, tool, parameters, node-ID resolution, clarification, execute, interpret.
- Plan and rationale shown with every answer: algorithm, why, parameters, complexity tier, runtime, exact or approximate.
- One-call graph profile report (JSON and HTML). Only NetworKit has something comparable; nothing exists for NetworkX or igraph.
- Ambiguity handling for directed vs undirected, weighted, "which centrality", and fuzzy node names. No open-source tool found handles node resolution explicitly.
- MCP server over the same catalogue.
- Evaluation set with ground truth from igraph, then GABench and ProGraph subsets.

Should have:

- Scalability tiers selected from the profile card: exact, parallel, GPU, approximate or sampled, with the user told which was used.
- Metric cache keyed by graph hash and tool parameters so follow-up questions reuse earlier results.
- Visualisation of results (subgraph, distribution charts, community layout) as a typed output element.
- Code-generation fallback in a sandbox with library docs retrieved into the prompt, and a human-approval gate to promote generated code into the catalogue.
- Multiple output formats: table, JSON, CSV, GraphML export of subgraphs.

Nice to have:

- Graph-reduction or sampling tool for very large inputs, drawing on the GraphProfile reduction experiments.
- Session memory of resolved node aliases and prior clarifications.
- Pluggable engines so the same tool runs on igraph, NetworkX, NetworKit, or a pybind11-wrapped C++ routine.
