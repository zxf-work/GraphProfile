# Headline experiment results

Questions: 10. Systems: full, flat, codegen. Model: claude-opus-5-5. Results dir: results_scale.

## Answer accuracy by family (mean score, 0 to 1)

| family | n | full | flat | codegen |
|---|---|---|---|---|
| metric | 5 | 1.00 | 1.00 | 1.00 |
| topk | 4 | 1.00 | 1.00 | 1.00 |
| interpretation | 1 | 1.00 | 1.00 | 1.00 |
| ALL | 10 | 1.00 | 1.00 | 1.00 |

## Plan correctness (mean, 0 to 1; n/a for codegen)

| family | full | flat | codegen |
|---|---|---|---|
| metric | 1.00 | 0.90 | n/a |
| topk | 1.00 | 1.00 | n/a |
| interpretation | 1.00 | 1.00 | n/a |
| ALL | 1.00 | 0.95 | n/a |

## Cost per question (means)

| system | LLM calls | tokens | USD | seconds | errors/timeouts |
|---|---|---|---|---|---|
| full | 2.0 | 9403 | 0.047 | 140 | 0 |
| flat | 3.4 | 20346 | 0.054 | 214 | 0 |
| codegen | 1.0 | 3193 | 0.033 | 129 | 0 |

## Per-question scores

| qid | family | full | flat | codegen |
|---|---|---|---|---|
| X1 | metric | 1.0 | 1.0 | 1.0 |
| X2 | topk | 1.0 | 1.0 | 1.0 |
| X3 | metric | 1.0 | 1.0 | 1.0 |
| X4 | topk | 1.0 | 1.0 | 1.0 |
| X5 | metric | 1.0 | 1.0 | 1.0 |
| X6 | topk | 1.0 | 1.0 | 1.0 |
| X7 | metric | 1.0 | 1.0 | 1.0 |
| X8 | topk | 1.0 | 1.0 | 1.0 |
| X9 | interpretation | 1.0 | 1.0 | 1.0 |
| X10 | metric | 1.0 | 1.0 | 1.0 |

## Failure notes (score < 1)

