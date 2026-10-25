# Glossary: Rubik's cube

Use these words in code, comments, commits and questions. If two words mean the same, we pick one and write the other as "also called".

| term | meaning |
|------|---------|
| **cube** | the whole puzzle. In this project a 3x3x3 unless Q-001 says otherwise |
| **cubie** | one small piece. 27 positions: 8 **corners**, 12 **edges**, 6 **centers**, and 1 invisible core |
| **sticker** (also called **facelet**) | one colored square on a cubie. 54 on a 3x3x3 (6 faces x 9) |
| **face** | one of the 6 sides: **U** up, **D** down, **L** left, **R** right, **F** front, **B** back |
| **layer** | the 9 cubies that turn together around one axis (3 layers per axis). A face layer is the outer one; the **middle layer** (slice) is the other |
| **move** | one layer turn written with the standard notation: `R` = turn face R a quarter turn **clockwise as seen looking at that face from outside**; `R'` = counter-clockwise ("R prime"); `R2` = half turn. 18 moves = 6 faces x 3 amounts |
| **sequence** | moves written in order, e.g. `R U R' U'` |
| **state** | where every cubie is and how it is twisted: everything needed to draw the cube |
| **solved state** | each face shows one color. The standard color scheme: U white, D yellow, F green, B blue, L orange, R red |
| **scramble** | a sequence of random moves applied to the solved state (so the state is always legal) |
| **orientation** | how a cubie is twisted in place: a corner has 3 possible, an edge 2 |
| **permutation** | which cubie sits in which position |
| **legal state** | reachable from the solved state by moves. Peeling off and re-sticking stickers can make an illegal one |
| **turn animation** | the motion of one layer from 0 to 90 degrees (or 180) over a short time |
| **world axes in our program** | OpenGL convention: +X right, +Y up, camera looks down -Z. So **R = +X, L = -X, U = +Y, D = -Y, F = +Z (towards the camera), B = -Z** |
