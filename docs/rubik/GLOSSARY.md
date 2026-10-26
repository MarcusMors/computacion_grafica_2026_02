# Glossary: Rubik's cube

Use these words in code, comments, commits and questions. If two words mean the same, we pick one and write the other as "also called".

| term | meaning |
|------|---------|
| **cube** | the whole puzzle. In this project a 3x3x3 unless Q-001 says otherwise |
| **cubie** | one small piece. 27 positions: 8 **corners**, 12 **edges**, 6 **centers**, and 1 invisible core |
| **sticker** (also called **facelet**) | one colored square on a cubie. 54 on a 3x3x3 (6 faces x 9) |
| **face** | one of the 6 sides: **U** up, **D** down, **L** left, **R** right, **F** front, **B** back |
| **layer** | the 9 cubies that turn together around one axis (3 layers per axis). The two outer ones are **face layers**; the one between them is the **middle layer** (slice) |
| **move** | one layer turn written with the standard notation: `R` = turn face R a quarter turn **clockwise as seen looking at that face from outside**; `R'` = counter-clockwise ("R prime"); `R2` = half turn. 18 moves = 6 faces x 3 amounts |
| **slice move** | `M`, `E`, `S`: turns the middle layer of one axis. `M` turns like `L`, `E` like `D`, `S` like `F` (course requirement, Q-010). With them there are 27 moves |
| **whole-cube turn** | `x`, `y`, `z`: all three layers of an axis turn together, `x` like `R`, `y` like `U`, `z` like `F`. The puzzle is the same, only held differently |
| **solution** | a sequence of moves that takes a state to a solved state |
| **sequence** | moves written in order, e.g. `R U R' U'` |
| **state** | where every cubie is and how it is twisted: everything needed to draw the cube |
| **solved state** | each face shows one color. The standard color scheme: U white, D yellow, F green, B blue, L orange, R red |
| **scramble** | a sequence of random moves applied to the solved state (so the state is always legal) |
| **orientation** | how a cubie is twisted in place: a corner has 3 possible, an edge 2 |
| **permutation** | which cubie sits in which position |
| **legal state** | reachable from the solved state by moves. Peeling off and re-sticking stickers can make an illegal one |
| **turn animation** | the motion of one layer from 0 to 90 degrees (or 180) over a short time |
| **world axes in our program** | OpenGL convention: +X right, +Y up, camera looks down -Z. So **R = +X, L = -X, U = +Y, D = -Y, F = +Z (towards the camera), B = -Z**. Careful: `Frame::forward` in `my_lib` is -Z (where the camera looks), the opposite of the cube's **F** |

## Move letters, spelled out (for readers who are not cubers)

A **letter** names one layer of the cube. The **move** is "turn that layer a quarter turn **clockwise, looking at it from
outside the cube**". A `'` after the letter ("prime") means counter-clockwise; a `2` means a half turn (the direction does
not matter then). A sequence such as `R U R' U2` is read left to right.

Our axes (see "world axes" above): X to the right, Y up, Z towards the viewer.

| letter | stands for | the layer it turns | where (our axes) | direction: clockwise seen from... |
|--------|------------|--------------------|------------------|-----------------------------------|
| `R` | **R**ight | right face layer | x = +1 | the right |
| `L` | **L**eft | left face layer | x = -1 | the left |
| `U` | **U**p | top face layer | y = +1 | above |
| `D` | **D**own | bottom face layer | y = -1 | below |
| `F` | **F**ront | face layer towards the viewer | z = +1 | the front |
| `B` | **B**ack | face layer away from the viewer | z = -1 | behind |
| `M` | **M**iddle | the layer between L and R | x = 0 | the left (turns like `L`) |
| `E` | **E**quator | the layer between U and D | y = 0 | below (turns like `D`) |
| `S` | **S**tanding | the layer between F and B | z = 0 | the front (turns like `F`) |
| `x` `y` `z` | (lower case) | the whole cube, all three layers of one axis | x / y / z | the right / above / the front (like `R` / `U` / `F`) |

- A letter is a **face or layer**, and writing it in a sequence makes it an **action**: `R` means "turn the Right layer".
  `D` is the Down layer, not the direction "downwards".
- Because every letter looks at its own layer from outside, `R` and `L` (and `U`/`D`, `F`/`B`) turn in **opposite** directions in
  space. That is why `L` can feel backwards: it is clockwise as seen from the left.
- `M`, `E`, `S` sit between a pair of faces and copy the direction of `L`, `D`, `F`.
- **quarter turn**: 90 degrees. **half turn**: 180 degrees. `R'` = `R R R`; `R2` = `R R`.
- Looking at the R face from outside, the X axis points at you (the "dot in a circle" of an electrical diagram); looking at the
  L face from outside it points away (the "cross in a circle"). That is the whole reason their directions are mirrored.
