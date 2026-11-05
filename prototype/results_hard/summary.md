# Headline experiment results

Questions: 27. Systems: full, no_verify, no_interp, flat, codegen. Model: claude-opus-5-5. Results dir: results_hard.

## Answer accuracy by family (mean score, 0 to 1)

| family | n | full | no_verify | no_interp | flat | codegen |
|---|---|---|---|---|---|---|
| interpretation | 22 | 0.95 | 0.91 | 0.91 | 0.91 | 0.95 |
| metric | 3 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 |
| clarification | 1 | 1.00 | 0.50 | 0.50 | 0.50 | 1.00 |
| topk | 1 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 |
| ALL | 27 | 0.96 | 0.91 | 0.91 | 0.91 | 0.96 |

## Plan correctness (mean, 0 to 1; n/a for codegen)

| family | full | no_verify | no_interp | flat | codegen |
|---|---|---|---|---|---|
| interpretation | 1.00 | 1.00 | 1.00 | 0.84 | n/a |
| metric | 1.00 | 1.00 | 1.00 | 0.83 | n/a |
| clarification | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| topk | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| ALL | 1.00 | 1.00 | 1.00 | 0.85 | n/a |

## Cost per question (means)

| system | LLM calls | tokens | USD | seconds | errors/timeouts |
|---|---|---|---|---|---|
| full | 2.0 | 10088 | 0.054 | 19 | 0 |
| no_verify | 2.0 | 9622 | 0.045 | 18 | 1 |
| no_interp | 2.0 | 9792 | 0.051 | 19 | 0 |
| flat | 4.5 | 26978 | 0.075 | 49 | 2 |
| codegen | 1.0 | 3104 | 0.029 | 11 | 0 |

## Per-question scores

| qid | family | full | no_verify | no_interp | flat | codegen |
|---|---|---|---|---|---|---|
| H1 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H2 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 0.0 |
| H3 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H4 | clarification | 1.0 | 0.5 | 0.5 | 0.5 | 1.0 ? |
| H5 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H6 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H7 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H8 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H9 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H10 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H11 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H12 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H13 | interpretation | 0.0 | 0.0 | 0.0 ? | 0.0 | 1.0 |
| H14 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H15 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H16 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H17 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H18 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H19 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H20 | topk | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H21 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H22 | interpretation | 1.0 | 0.0 | 0.0 | 1.0 | 1.0 |
| H23 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H24 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H25 | interpretation | 1.0 | 1.0 | 1.0 | 0.0 | 1.0 |
| H26 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H27 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |

## Failure notes (score < 1)

- codegen H2 (interpretation): score 0.0: got "J49"
- no_verify H4 (clarification): score 0.5: value right but assumption not stated
- no_interp H4 (clarification): score 0.5: value right but assumption not stated
- flat H4 (clarification): score 0.5: value right but assumption not stated
- full H13 (interpretation): score 0.0: pair=False dist=False
- no_verify H13 (interpretation): score 0.0: pair=False dist=False
- no_interp H13 (interpretation): score 0.0: pair=False dist=False
- flat H13 (interpretation): score 0.0: error: max turns reached without a final answer [error: max turns reached without a final answer]
- no_verify H22 (interpretation): score 0.0: error: ValueError: node 's5.node[0]' not found; candidates: [] [error: ValueError: node 's5.node[0]' not found; candidates: []]
- no_interp H22 (interpretation): score 0.0: got "max leakage among trustees"
- flat H25 (interpretation): score 0.0: error: TimeoutExpired: Command '['claude', '-p', '--output-format', 'json', '--model', 'claude-opus-5-5', '--system-prompt', 'Y [error: TimeoutExpired: Command '['claude', '-p', '--output-format', 'json', '--model', 'claude-opus-5-5', ']
