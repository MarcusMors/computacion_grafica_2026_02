# How we work together (humans and their AI agents)

Several people, each with their own AI agent session, share this repository. The agents do **not** share memory, so
everything that matters (questions, answers, decisions, vocabulary) is written down as a file here and travels through git.

## Topics

| topic   | folder                | status |
|---------|-----------------------|--------|
| porting | [`porting/`](porting/) | LabCompiler on Windows, Linux and macOS (one person) |
| rubik   | [`rubik/`](rubik/)     | Rubik's cube group project (three people) |

Each topic folder has the same shape:

```
README.md       what the topic is, its status, who is the facilitator
GLOSSARY.md     the words we use, defined once (use them in code and in docs)
questions/      Q-001-short-title.md   open questions, one file each
adr/            0001-short-title.md    accepted decisions ("architecture decision records"), one file each
```

Templates: [`templates/QUESTION.md`](templates/QUESTION.md), [`templates/ADR.md`](templates/ADR.md).
**One file per question and per decision** keeps merges free of conflicts.

## Roles

- **Facilitator** (one per topic; for now Marcus): his agent runs the "grilling" interview, writes the question files, and
  turns agreed answers into ADRs. Nobody else creates questions on that topic; if you have a new one, tell the facilitator
  or open a pull request that adds only your question file after asking.
- **Answerers** (everyone, including the facilitator): you answer in your own line under "Answers". Your agent may help
  you think, summarise options and check the code, but the answer is yours.

## The loop

1. The facilitator's agent writes `questions/Q-NNN-...md` (context, options, a recommendation) on a branch `q/NNN-short-title`
   and opens a pull request titled `question: Q-NNN ...`.
2. Each person answers on their own line (option letter + one-line reason), by editing the file on their own branch
   (`answers/<name>`) and opening a PR, or by commenting. The answer lines are separated by a blank line ON PURPOSE: git reports
   a conflict when two people edit lines that touch each other. Discussion happens in PR comments.
3. When every answer is in and you agree, the facilitator's agent writes `adr/NNNN-...md` (status `accepted`), sets the
   question to `decided`, and the PR is merged. Disagreement stays open: talk, then update the answers.
4. Code pull requests link the question or ADR they implement. If the code needs something not decided yet, stop and
   ask a question instead of inventing.

Statuses: question = `open` -> `answered` (everyone replied) -> `decided` (ADR written); ADR = `proposed` -> `accepted`
-> `superseded by ADR-XXXX`.

## Git rules

- Branch names: `<your-name>/<what>` for code, `q/NNN-title` for questions. One concern per pull request.
- Nobody pushes straight to `main`; one partner approves each pull request.
- Commits use Conventional Commits (`feat:`, `fix:`, `docs:`, `refactor:`, `build:`).
- Line endings are LF everywhere (`.gitattributes`); `.bat` / `.ps1` are CRLF.
- Never commit binaries, compilers, `bin/`, `build/`, or the portable tool folders (see `.gitignore`).

## Joining the repository (partners who already have the LabCompiler folder)

The repository holds **only our code** (ADR-0005 in `porting/adr`): GLFW, the other libraries, the compilers and Boost come from the LabCompiler zip, so a bare `git clone` cannot build. Attach your zip folder instead:

Inside your existing LabCompiler folder (this keeps your files and only shows what differs):

```
git init
git remote add origin https://github.com/MarcusMors/computacion_grafica_2026_02.git
git fetch origin
git reset origin/main
git status
```

Then create your branch (`git switch -c <your-name>/<what>`) and start your agent session **at the repository root**, so it
reads `CLAUDE.md` automatically.

## GitHub settings (one time, by the owner)

1. Settings -> Collaborators: add each partner.
2. Settings -> Branches -> add a rule for `main`: require a pull request, 1 approval, no force pushes.
3. Optional, later: GitHub Actions would need a step that downloads the third-party pieces first (ADR-0005).

> **The repository is public.** Everything committed is visible to everyone, including AI-assistance notes and the
> names of files. Check what you are about to commit.

## Agent skills (so every agent works the same way)

Install the same ones (links as collected by the owner; if one cannot be downloaded, say so in a pull request):

| skill | use it for |
|-------|-----------|
| [grill-with-docs](https://www.skills.sh/mattpocock/skills/grill-with-docs) | interview the humans and write the answers into these docs (facilitator) |
| [domain-modeling](https://www.skills.sh/mattpocock/skills/domain-modeling) | glossary and decision records |
| [codebase-design](https://www.skills.sh/mattpocock/skills/codebase-design) | module and interface shape |
| [research](https://www.skills.sh/mattpocock/skills/research) | checking facts before deciding |
| [architecture-designer](https://jeffallan.github.io/claude-skills/skills/api-architecture/architecture-designer/) | larger structural decisions |
| [feature-forge](https://jeffallan.github.io/claude-skills/skills/workflow/feature-forge/) | turning a wish into requirements |
| [the-fool](https://jeffallan.github.io/claude-skills/skills/workflow/the-fool/) | stress-testing a plan before building |
| [code-review](https://www.skills.sh/mattpocock/skills/code-review) | reviewing a pull request |
| [diagnosing-bugs](https://www.skills.sh/mattpocock/skills/diagnosing-bugs) | finding why something breaks |
| [implement](https://www.skills.sh/mattpocock/skills/implement) | building an agreed change |
