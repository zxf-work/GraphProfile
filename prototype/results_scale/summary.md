# Headline experiment results

Questions: 16. Systems: full, flat, codegen. Model: claude-opus-5-5. Results dir: results_scale.

## Answer accuracy by family (mean score, 0 to 1)

| family | n | full | flat | codegen |
|---|---|---|---|---|
| metric | 8 | 1.00 | 0.62 | 0.62 |
| topk | 6 | 1.00 | 0.67 | 0.67 |
| interpretation | 1 | 1.00 | 1.00 | 1.00 |
| path | 1 | 1.00 | 0.00 | 0.00 |
| ALL | 16 | 1.00 | 0.62 | 0.62 |

## Plan correctness (mean, 0 to 1; n/a for codegen)

| family | full | flat | codegen |
|---|---|---|---|
| metric | 1.00 | 0.75 | n/a |
| topk | 1.00 | 0.83 | n/a |
| interpretation | 1.00 | 1.00 | n/a |
| path | 1.00 | 0.50 | n/a |
| ALL | 1.00 | 0.78 | n/a |

## Cost per question (means)

| system | LLM calls | tokens | USD | seconds | errors/timeouts |
|---|---|---|---|---|---|
| full | 2.1 | 9852 | 0.050 | 293 | 0 |
| flat | 2.1 | 12717 | 0.034 | 164 | 6 |
| codegen | 0.6 | 1996 | 0.020 | 111 | 6 |

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
| X11 | metric | 1.0 | 0.0 | 0.0 |
| X12 | topk | 1.0 | 0.0 | 0.0 |
| X13 | metric | 1.0 | 0.0 | 0.0 |
| X14 | metric | 1.0 | 0.0 | 0.0 |
| X15 | topk | 1.0 | 0.0 | 0.0 |
| X16 | path | 1.0 | 0.0 | 0.0 |

## Failure notes (score < 1)

- flat X11 (metric): score 0.0: error: runner failed rc=-9:  [error: runner failed rc=-9: ]
- codegen X11 (metric): score 0.0: error: runner failed rc=-9:  [error: runner failed rc=-9: ]
- flat X12 (topk): score 0.0: error: runner failed rc=-9:  [error: runner failed rc=-9: ]
- codegen X12 (topk): score 0.0: error: runner failed rc=-9:  [error: runner failed rc=-9: ]
- flat X13 (metric): score 0.0: error: runner failed rc=-9:  [error: runner failed rc=-9: ]
- codegen X13 (metric): score 0.0: error: runner failed rc=-9:  [error: runner failed rc=-9: ]
- flat X14 (metric): score 0.0: error: runner failed rc=-9:  [error: runner failed rc=-9: ]
- codegen X14 (metric): score 0.0: error: runner failed rc=-9:  [error: runner failed rc=-9: ]
- flat X15 (topk): score 0.0: error: runner failed rc=-9:  [error: runner failed rc=-9: ]
- codegen X15 (topk): score 0.0: error: runner failed rc=-9:  [error: runner failed rc=-9: ]
- flat X16 (path): score 0.0: error: runner failed rc=-9:  [error: runner failed rc=-9: ]
- codegen X16 (path): score 0.0: error: runner failed rc=-9:  [error: runner failed rc=-9: ]
