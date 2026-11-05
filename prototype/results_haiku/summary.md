# Headline experiment results

Questions: 60. Systems: full, no_verify, no_interp, flat, codegen. Model: claude-haiku-5-5. Results dir: results_haiku.

## Answer accuracy by family (mean score, 0 to 1)

| family | n | full | no_verify | no_interp | flat | codegen |
|---|---|---|---|---|---|---|
| profile | 3 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 |
| metric | 14 | 1.00 | 1.00 | 1.00 | 0.93 | 1.00 |
| topk | 7 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 |
| community | 4 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 |
| path | 5 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 |
| neighbourhood | 4 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 |
| comparative | 2 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 |
| compositional | 5 | 1.00 | 1.00 | 1.00 | 0.60 | 1.00 |
| interpretation | 7 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 |
| clarification | 4 | 1.00 | 1.00 | 1.00 | 0.88 | 1.00 |
| resolution | 5 | 1.00 | 1.00 | 1.00 | 0.80 | 0.90 |
| ALL | 60 | 1.00 | 1.00 | 1.00 | 0.93 | 0.99 |

## Plan correctness (mean, 0 to 1; n/a for codegen)

| family | full | no_verify | no_interp | flat | codegen |
|---|---|---|---|---|---|
| profile | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| metric | 1.00 | 1.00 | 1.00 | 0.93 | n/a |
| topk | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| community | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| path | 0.80 | 0.80 | 0.80 | 0.80 | n/a |
| neighbourhood | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| comparative | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| compositional | 1.00 | 1.00 | 1.00 | 0.80 | n/a |
| interpretation | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| clarification | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| resolution | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| ALL | 0.98 | 0.98 | 0.98 | 0.95 | n/a |

## Cost per question (means)

| system | LLM calls | tokens | USD | seconds | errors/timeouts |
|---|---|---|---|---|---|
| full | 2.0 | 9383 | 0.001 | 15 | 0 |
| no_verify | 2.0 | 9321 | 0.001 | 15 | 0 |
| no_interp | 2.0 | 9247 | 0.001 | 16 | 0 |
| flat | 4.8 | 35850 | 0.008 | 65 | 3 |
| codegen | 1.1 | 3912 | 0.001 | 16 | 0 |

## Per-question scores

| qid | family | full | no_verify | no_interp | flat | codegen |
|---|---|---|---|---|---|---|
| P1 | profile | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| P2 | profile | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| P3 | profile | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S1 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S2 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S3 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S4 | metric | 1.0 | 1.0 | 1.0 | 0.0 | 1.0 |
| S5 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S6 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S7 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S8 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S9 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S10 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S11 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S12 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S13 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S14 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| T1 | topk | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| T2 | topk | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| T3 | topk | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| T4 | topk | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| T5 | topk | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| T6 | topk | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| T7 | topk | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| C1 | community | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| C2 | community | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| C3 | community | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| C4 | community | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| PA1 | path | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| PA2 | path | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| PA3 | path | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 ? |
| PA4 | path | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| PA5 | path | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| N1 | neighbourhood | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| N2 | neighbourhood | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| N3 | neighbourhood | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| N4 | neighbourhood | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| CM1 | comparative | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| CM2 | comparative | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| CO1 | compositional | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| CO2 | compositional | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| CO3 | compositional | 1.0 | 1.0 | 1.0 | 0.0 | 1.0 |
| CO4 | compositional | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| CO5 | compositional | 1.0 | 1.0 | 1.0 | 0.0 | 1.0 |
| I1 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| I2 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| I3 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| I4 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| I5 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| I6 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| I7 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| CL1 | clarification | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| CL2 | clarification | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| CL3 | clarification | 1.0 | 1.0 | 1.0 | 0.5 | 1.0 ? |
| CL4 | clarification | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| NR1 | resolution | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| NR2 | resolution | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 ? |
| NR3 | resolution | 1.0 | 1.0 | 1.0 | 0.0 ? | 1.0 |
| NR4 | resolution | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| NR5 | resolution | 1.0 | 1.0 | 1.0 | 1.0 ? | 0.5 |

## Failure notes (score < 1)

- flat S4 (metric): score 0.0: error: TimeoutExpired: Command '['claude', '-p', '--output-format', 'json', '--model', 'claude-haiku-5-5', '--system-prompt', ' [error: TimeoutExpired: Command '['claude', '-p', '--output-format', 'json', '--model', 'claude-haiku-5-5', ]
- flat CO3 (compositional): score 0.0: error: TimeoutExpired: Command '['claude', '-p', '--output-format', 'json', '--model', 'claude-haiku-5-5', '--system-prompt', ' [error: TimeoutExpired: Command '['claude', '-p', '--output-format', 'json', '--model', 'claude-haiku-5-5', ]
- flat CO5 (compositional): score 0.0: error: max turns reached without a final answer [error: max turns reached without a final answer]
- flat CL3 (clarification): score 0.5: single value with mode stated
- flat NR3 (resolution): score 0.0: jaccard=0.69
- codegen NR5 (resolution): score 0.5: correct only as a variant
