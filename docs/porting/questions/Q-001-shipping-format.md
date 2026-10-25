# Q-001: How do we ship LabCompiler to students?

- **Topic:** porting
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, before writing the release scripts

## Context
Windows needs about 1.5 GB of bundled tools (MinGW, CMake, editor); Linux and macOS need none of that (ADR-0001). The git
repository holds only sources and scripts.

## Options
### A. One zip per OS, built from the same repo
`LabCompiler-windows.zip` (sources + MinGW + CMake + editor), `LabCompiler-linux.zip` and `LabCompiler-macos.zip` (sources + scripts).
- (+) Small downloads; each student gets only what they need.
- (-) Three artifacts to produce and test.

### B. One universal zip with every bundle
- (+) One thing to hand out.
- (-) Windows tools make it huge for everybody; confusing.

## Recommendation
A.

## Answers
- Marcus:

## Decision
