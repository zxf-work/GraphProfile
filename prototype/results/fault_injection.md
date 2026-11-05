# Fault injection: what the verifier catches

| mutation | injected | changed the answer | caught (of harmful) | catch rate | flagged although harmless |
|---|---|---|---|---|---|
| drop_lcc | 4 | 0 | - | - | 1 |
| flip_directed | 60 | 35 | 35 | 100% | 25 |
| method_swap | 8 | 8 | 1 | 12% | 0 |
| mode_flip | 7 | 7 | 4 | 57% | 0 |
| swap_measure | 17 | 17 | 16 | 94% | 0 |
| use_weights | 3 | 3 | 3 | 100% | 0 |
| wrong_node | 22 | 17 | 17 | 100% | 5 |
| ALL | 121 | 87 | 76 | 87% | 31 |

## Caught by stage

- verifier: 71
- validator: 5

## Missed harmful mutations

- P1 method_swap
- P2 mode_flip
- P2 method_swap
- P3 swap_measure
- P3 method_swap
- C1 method_swap
- C2 method_swap
- C3 method_swap
- C4 method_swap
- N2 mode_flip
- CL3 mode_flip
