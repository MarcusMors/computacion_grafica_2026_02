# Start here: answering the Rubik questions

The questions are already written (see [`questions/`](questions/)). Nobody writes new ones except the facilitator.
Everybody answers, **in parallel**, each one in their own session.

## Before you start (once)

1. You accepted the GitHub invitation to the repository, and your LabCompiler folder is attached to it
   (the commands are in [`../README.md`](../README.md), section "Joining the repository"). Then `git pull`.
2. Start your Claude Code session **at the repository root**, so it reads `CLAUDE.md` by itself.
3. Make your branch: `git switch -c answers/<your-github-name>`.

## The order

Answer in this order, because each question depends on the previous ones:
**Q-001 -> Q-002 -> Q-003 -> Q-004 -> Q-005 -> Q-006 -> Q-008 -> Q-007**.
If you disagree with the recommendation, say so and why: a short reason is enough.

## What to tell your agent

**Marcus (facilitator)**

> Read CLAUDE.md and docs/rubik/ (README, GLOSSARY, questions). You are the facilitator of the rubik topic. Interview me on the
> open questions one at a time, in the order of docs/rubik/START_HERE.md. Explain each option in plain words with a small code or
> picture example, give your recommendation, and write my answer ONLY on my line. If my answers reveal a gap, propose a new
> question file. Do not touch anybody else's line. When all three have answered a question, tell me and prepare the ADR.

**CristianMellado and FabricioMessa**

> Read CLAUDE.md and docs/rubik/ (README, GLOSSARY, questions). You are NOT the facilitator: do not create question files or
> ADRs. Walk me through the open questions one at a time, in the order of docs/rubik/START_HERE.md. Explain each option in plain
> words, check it against the existing code in my_lib when relevant, say what you would pick and why, but let me decide. Write my
> answer ONLY on my own line (CristianMellado / FabricioMessa). Never edit another person's line.

## When you are done

Commit only your answer lines (`docs: my answers to Q-001..Q-003`), push your branch and open a pull request. The facilitator
merges when the three of you agree and turns the agreed answers into ADRs in `adr/`.
