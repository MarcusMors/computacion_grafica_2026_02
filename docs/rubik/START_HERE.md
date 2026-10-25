# Start here: answering the Rubik questions

The questions are already written (see [`questions/`](questions/)); there are 12. Nobody writes new ones except the facilitator
(Marcus's agent). Every one of you answers **all 12**, in two steps so that nobody has to dig deep into 12 at once:

1. **Lead:** you study your 4 questions in depth and answer them.
2. **Review:** you give your own answer to the other 8, helped by what the lead wrote.

**No question is locked.** Every question file keeps all three answer lines (Marcus, CristianMellado, FabricioMessa), so anybody
can read and answer any question at any time, not only their own four; "lead" only says who studies it first. The one rule
stays: write on **your own line only**. Keep the blank line between the answer lines: it lets two people edit the same file in
parallel without a merge conflict.

## Before you start (once)

1. You accepted the GitHub invitation to the repository, and your LabCompiler folder is attached to it
   (the commands are in [`../README.md`](../README.md), section "Joining the repository").
2. Get the questions. If the questions pull request is already merged: `git pull`. If not yet:
   `git fetch origin` and then `git switch -c answers/<your-github-name> origin/q/rubik-questions`.
3. Start your Claude Code session **at the repository root**, so it reads `CLAUDE.md` by itself.
4. Your branch for step 1 is `answers/<your-github-name>` (for step 2: `answers/<your-github-name>-review`).

## The order

Each question depends on the ones before it, so answer in this order:

| # | question | in one line |
|---|----------|-------------|
| 1 | Q-001 | 3x3x3 only, or any size? |
| 2 | Q-008 | what is in version 1 (course requirements + extras) |
| 3 | Q-010 | does the middle layer turn (M, E, S)? |
| 4 | Q-002 | how we store the state of the cube |
| 5 | Q-003 | how a move is represented and typed |
| 6 | Q-011 | where the solution comes from (solver or not) |
| 7 | Q-004 | how the cube is drawn (meshes and figures) |
| 8 | Q-009 | how a turned cubie is drawn (rotations vs `Transform`) |
| 9 | Q-005 | how one layer turn is animated |
| 10 | Q-012 | does the solving animation show middle-layer turns |
| 11 | Q-006 | keyboard and camera controls |
| 12 | Q-007 | who writes which part |

If you disagree with a recommendation, say so and why: a short reason is enough.

## Who leads what (agreed by Marcus: first, second and last third; Cristian and Fabricio can swap if they prefer)

| lead | questions | theme |
|------|-----------|-------|
| Marcus | Q-001, Q-008, Q-010, Q-002 | scope and the state |
| CristianMellado | Q-003, Q-011, Q-004, Q-009 | moves, solver, drawing |
| FabricioMessa | Q-005, Q-012, Q-006, Q-007 | animation, solution display, controls, split of work |

## Step 1: lead (all three in parallel)

- Work through your 4 questions in the order above, with your agent, and write your answer on **your own line**: option letter
  and a one-line reason.
- A question may depend on one that someone else leads. Do not wait: assume the recommended option of that upstream question and
  say so in your reason (for example "B, assuming Q-002 = C"). If the upstream answer ends up different, the facilitator flags yours.
- Commit only your answer lines (`docs: my answers to Q-003, Q-011, Q-004, Q-009`), push your branch, open a pull request titled
  `answers: <name> lead`. One partner approves; the facilitator merges.

## Step 2: review (after the three step-1 pull requests are merged)

- `git pull`, new branch `answers/<your-name>-review`. (If you already have a view on a question you do not lead, you may answer
  it earlier; the review step is simply when every lead's reasons are available.)
- On each of the **8 questions you did not lead**, read the lead's line and reason (your agent summarises it), then write your own
  line: `agree with <name>: <letter>`, or a different letter and why. The decision is yours, not the lead's.
- Pull request titled `answers: <name> review`.

## Step 3: facilitator

Marcus's agent lists, per question, who agrees and who does not. Disagreements are talked through in the pull request or a call;
whoever changes their mind edits their own line. When all three agree, the agent writes the ADR in `adr/` and marks the question decided.

## What to tell your agent

You are computer scientists: you know data structures, algorithms and linear algebra. What is new is how graphics organises data
(meshes, GPU buffers, transform matrices). The prompts below say so, so your agent does not over-explain the first and does not skip the second.

**Marcus (facilitator)**

> Read CLAUDE.md and docs/rubik/ (README, GLOSSARY, START_HERE, questions). You are the facilitator of the rubik topic, and I lead
> Q-001, Q-008, Q-010 and Q-002 (then I review the other 8). My partners and I are computer scientists: do not explain basic data
> structures, but explain graphics-specific ideas (mesh, transform, GPU buffer) in plain words with a small code or picture example.
> Walk me through my questions one at a time, give your recommendation, and write my answer ONLY on my line. If my answers reveal a
> gap, propose a new question file. Do not touch anybody else's line. When all three have answered a question, tell me and prepare the ADR.

**CristianMellado and FabricioMessa** (replace the question numbers with your own four from the table above)

> Read CLAUDE.md and docs/rubik/ (README, GLOSSARY, START_HERE, questions). You are NOT the facilitator: do not create question
> files or ADRs. I lead <your four questions> (step 1) and later review the other 8 (step 2). I am a computer scientist: do not
> explain basic data structures, but explain graphics-specific ideas (mesh, transform, GPU buffer) in plain words with a small code or
> picture example, and check the options against the real code in my_lib when relevant. Say what you would pick and why, but let me
> decide. Write my answer ONLY on my own line (CristianMellado / FabricioMessa). Never edit another person's line.
