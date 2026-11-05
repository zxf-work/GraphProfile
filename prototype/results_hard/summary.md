# Headline experiment results

Questions: 27. Systems: full, no_verify, no_interp, flat, codegen. Model: claude-opus-5-5. Results dir: results_hard.

## Answer accuracy by family (mean score, 0 to 1)

| family | n | full | no_verify | no_interp | flat | codegen |
|---|---|---|---|---|---|---|
| interpretation | 22 | 1.00 | 1.00 | 1.00 | 0.95 | 1.00 |
| metric | 3 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 |
| clarification | 1 | 1.00 | 0.50 | 0.50 | 0.50 | 1.00 |
| topk | 1 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 |
| ALL | 27 | 1.00 | 0.98 | 0.98 | 0.94 | 1.00 |

## Plan correctness (mean, 0 to 1; n/a for codegen)

| family | full | no_verify | no_interp | flat | codegen |
|---|---|---|---|---|---|
| interpretation | 0.98 | 0.98 | 0.98 | 0.82 | n/a |
| metric | 1.00 | 1.00 | 1.00 | 0.83 | n/a |
| clarification | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| topk | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| ALL | 0.98 | 0.98 | 0.98 | 0.83 | n/a |

## Cost per question (means)

| system | LLM calls | tokens | USD | seconds | errors/timeouts |
|---|---|---|---|---|---|
| full | 2.0 | 9781 | 0.050 | 17 | 0 |
| no_verify | 2.0 | 9455 | 0.043 | 16 | 0 |
| no_interp | 2.0 | 9575 | 0.049 | 17 | 0 |
| flat | 4.1 | 24703 | 0.068 | 46 | 1 |
| codegen | 1.0 | 3104 | 0.029 | 11 | 0 |

## Per-question scores

| qid | family | full | no_verify | no_interp | flat | codegen |
|---|---|---|---|---|---|---|
| H1 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H2 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
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
| H13 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H14 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H15 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H16 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H17 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H18 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H19 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H20 | topk | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H21 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H22 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H23 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H24 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H25 | interpretation | 1.0 | 1.0 | 1.0 | 0.0 | 1.0 |
| H26 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| H27 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |

## Failure notes (score < 1)

- no_verify H4 (clarification): score 0.5: value right but assumption not stated
- no_interp H4 (clarification): score 0.5: value right but assumption not stated
- flat H4 (clarification): score 0.5: value right but assumption not stated
- flat H25 (interpretation): score 0.0: error: TimeoutExpired: Command '['claude', '-p', '--output-format', 'json', '--model', 'claude-opus-5-5', '--system-prompt', 'Y [error: TimeoutExpired: Command '['claude', '-p', '--output-format', 'json', '--model', 'claude-opus-5-5', ']
