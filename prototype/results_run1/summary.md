# Headline experiment results

Questions: 60. Systems: full, no_verify, no_interp, flat, codegen. Model: claude-opus-5-5.

## Answer accuracy by family (mean score, 0 to 1)

| family | n | full | no_verify | no_interp | flat | codegen |
|---|---|---|---|---|---|---|
| profile | 3 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 |
| metric | 14 | 1.00 | 1.00 | 0.79 | 1.00 | 1.00 |
| topk | 7 | 1.00 | 1.00 | 0.57 | 1.00 | 1.00 |
| community | 4 | 1.00 | 0.75 | 0.75 | 1.00 | 1.00 |
| path | 5 | 0.60 | 0.60 | 0.40 | 1.00 | 1.00 |
| neighbourhood | 4 | 1.00 | 0.75 | 0.75 | 0.75 | 1.00 |
| comparative | 2 | 0.50 | 0.50 | 0.50 | 1.00 | 1.00 |
| compositional | 5 | 0.80 | 0.60 | 0.40 | 1.00 | 1.00 |
| interpretation | 7 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 |
| clarification | 4 | 0.88 | 0.88 | 0.75 | 1.00 | 1.00 |
| resolution | 5 | 1.00 | 1.00 | 0.40 | 0.80 | 0.90 |
| ALL | 60 | 0.93 | 0.88 | 0.68 | 0.97 | 0.99 |

## Plan correctness (mean, 0 to 1; n/a for codegen)

| family | full | no_verify | no_interp | flat | codegen |
|---|---|---|---|---|---|
| profile | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| metric | 1.00 | 1.00 | 1.00 | 0.93 | n/a |
| topk | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| community | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| path | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| neighbourhood | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| comparative | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| compositional | 1.00 | 1.00 | 1.00 | 0.80 | n/a |
| interpretation | 1.00 | 1.00 | 1.00 | 0.79 | n/a |
| clarification | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| resolution | 1.00 | 1.00 | 1.00 | 1.00 | n/a |
| ALL | 1.00 | 1.00 | 1.00 | 0.94 | n/a |

## Cost per question (means)

| system | LLM calls | tokens | USD | seconds | errors/timeouts |
|---|---|---|---|---|---|
| full | 2.0 | 8537 | 0.048 | 17 | 1 |
| no_verify | 2.0 | 8278 | 0.040 | 17 | 3 |
| no_interp | 2.0 | 8554 | 0.048 | 19 | 3 |
| flat | 4.1 | 20764 | 0.066 | 27 | 1 |
| codegen | 1.1 | 3370 | 0.032 | 20 | 0 |

## Per-question scores

| qid | family | full | no_verify | no_interp | flat | codegen |
|---|---|---|---|---|---|---|
| P1 | profile | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| P2 | profile | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| P3 | profile | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S1 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S2 | metric | 1.0 | 1.0 | 0.0 | 1.0 | 1.0 |
| S3 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S4 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S5 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S6 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S7 | metric | 1.0 | 1.0 | 0.0 | 1.0 | 1.0 |
| S8 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S9 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S10 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S11 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S12 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| S13 | metric | 1.0 | 1.0 | 0.0 | 1.0 | 1.0 |
| S14 | metric | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| T1 | topk | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| T2 | topk | 1.0 | 1.0 | 0.0 | 1.0 | 1.0 |
| T3 | topk | 1.0 | 1.0 | 0.0 | 1.0 | 1.0 |
| T4 | topk | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| T5 | topk | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| T6 | topk | 1.0 | 1.0 | 0.0 | 1.0 | 1.0 |
| T7 | topk | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| C1 | community | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| C2 | community | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| C3 | community | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| C4 | community | 1.0 | 0.0 | 0.0 | 1.0 | 1.0 |
| PA1 | path | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| PA2 | path | 1.0 | 1.0 | 0.0 | 1.0 | 1.0 |
| PA3 | path | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 ? |
| PA4 | path | 0.0 | 0.0 | 0.0 | 1.0 | 1.0 |
| PA5 | path | 0.0 | 0.0 | 0.0 | 1.0 | 1.0 |
| N1 | neighbourhood | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| N2 | neighbourhood | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| N3 | neighbourhood | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| N4 | neighbourhood | 1.0 | 0.0 | 0.0 | 0.0 | 1.0 |
| CM1 | comparative | 0.0 | 0.0 | 0.0 | 1.0 | 1.0 |
| CM2 | comparative | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| CO1 | compositional | 1.0 | 0.0 | 0.0 | 1.0 | 1.0 |
| CO2 | compositional | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| CO3 | compositional | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| CO4 | compositional | 0.0 | 0.0 | 0.0 | 1.0 | 1.0 |
| CO5 | compositional | 1.0 | 1.0 | 0.0 | 1.0 | 1.0 |
| I1 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| I2 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| I3 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| I4 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| I5 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| I6 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 ? | 1.0 |
| I7 | interpretation | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| CL1 | clarification | 1.0 | 1.0 | 0.5 | 1.0 | 1.0 |
| CL2 | clarification | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 ? |
| CL3 | clarification | 0.5 | 0.5 | 0.5 | 1.0 | 1.0 |
| CL4 | clarification | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| NR1 | resolution | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 |
| NR2 | resolution | 1.0 | 1.0 | 0.0 | 1.0 | 1.0 |
| NR3 | resolution | 1.0 | 1.0 | 1.0 | 0.0 | 1.0 |
| NR4 | resolution | 1.0 | 1.0 | 0.0 | 1.0 | 1.0 |
| NR5 | resolution | 1.0 | 1.0 | 0.0 | 1.0 ? | 0.5 |

## Failure notes (score < 1)

- no_interp S2 (metric): score 0.0: got 17 expected 8
- no_interp S7 (metric): score 0.0: got -0.5687686833898792 expected -0.47561309768461435
- no_interp S13 (metric): score 0.0: got 0.0054099817517196435 expected 0.010819963503439287
- no_interp T2 (topk): score 0.0: overlap@10=0.00 got ['1911', '3434', '2655', '1902', '1888', '2649', '1907', '3971', '2654', '1910']
- no_interp T3 (topk): score 0.0: overlap@10=0.40 got ['1684', '1912', '1718', '563', '1405', '1656', '1086', '567', '3437', '119']
- no_interp T6 (topk): score 0.0: overlap@5=0.60 got ['Fantine', 'Valjean', 'Javert', 'Thenardier', 'Marius']
- no_verify C4 (community): score 0.0: error: TypeError: unsupported operand type(s) for /: 'str' and 'str' [error: TypeError: unsupported operand type(s) for /: 'str' and 'str']
- no_interp C4 (community): score 0.0: error: TypeError: unsupported operand type(s) for /: 'str' and 'str' [error: TypeError: unsupported operand type(s) for /: 'str' and 'str']
- no_interp PA2 (path): score 0.0: got "inf" expected 2
- full PA4 (path): score 0.0: got 2 expected True
- no_verify PA4 (path): score 0.0: got 2 expected True
- no_interp PA4 (path): score 0.0: got 2 expected True
- full PA5 (path): score 0.0: got 4 expected True
- no_verify PA5 (path): score 0.0: got 4 expected True
- no_interp PA5 (path): score 0.0: got 4 expected True
- no_verify N4 (neighbourhood): score 0.0: jaccard=0.00
- no_interp N4 (neighbourhood): score 0.0: jaccard=0.00
- flat N4 (neighbourhood): score 0.0: error: max turns reached without a final answer [error: max turns reached without a final answer]
- full CM1 (comparative): score 0.0: got {"a": 1192496.1130793886, "b": 3916560.1444407436, "a_greate
- no_verify CM1 (comparative): score 0.0: got {"a": 1192496.1130793886, "b": 3916560.1444407436, "a_greate
- no_interp CM1 (comparative): score 0.0: got {"a": 0.0, "b": 3963.904624030397, "a_greater": false, "rati
- no_verify CO1 (compositional): score 0.0: error: invalid plan after repair: step s3: argument nodes expects NodeSet but s2 is Table [error: invalid plan after repair: step s3: argument nodes expects NodeSet but s2 is Table]
- no_interp CO1 (compositional): score 0.0: error: TypeError: '>=' not supported between instances of 'float' and 'dict' [error: TypeError: '>=' not supported between instances of 'float' and 'dict']
- full CO4 (compositional): score 0.0: error: TypeError: '>=' not supported between instances of 'float' and 'dict' [error: TypeError: '>=' not supported between instances of 'float' and 'dict']
- no_verify CO4 (compositional): score 0.0: error: ValueError: node 'from' not found; candidates: [] [error: ValueError: node 'from' not found; candidates: []]
- no_interp CO4 (compositional): score 0.0: error: TypeError: '>=' not supported between instances of 'float' and 'dict' [error: TypeError: '>=' not supported between instances of 'float' and 'dict']
- no_interp CO5 (compositional): score 0.0: got [{"node": "Fantine", "value": 359.04166666666663}, {"node": 
- no_interp CL1 (clarification): score 0.5: asked or gave variants but primary answer outside acceptable set
- full CL3 (clarification): score 0.5: single value with mode stated
- no_verify CL3 (clarification): score 0.5: single value with mode stated
- no_interp CL3 (clarification): score 0.5: single value with mode stated
- no_interp NR2 (resolution): score 0.0: got "inf" expected 1
- flat NR3 (resolution): score 0.0: jaccard=0.85
- no_interp NR4 (resolution): score 0.0: got 136.0920634920635 expected 470.5706319136659
- no_interp NR5 (resolution): score 0.0: got 0.015412221317836617 expected 0.030302735905813585
- codegen NR5 (resolution): score 0.5: correct only as a variant
