---
name: fix-issue
description: Fix a GitHub issue by number. Use when asked to fix a bug or implement a tracked issue.
argument-hint: "<issue-number>"
disable-model-invocation: true
---

# Fix GitHub Issue #$ARGUMENTS

Fix the issue reported at zwaliebaba/interstellaroutpost.dx12#$ARGUMENTS.

## Steps

1. **Read the issue** — Use the GitHub MCP tool to fetch issue #$ARGUMENTS. Understand the bug report or feature request fully before touching any code.

2. **Reproduce / locate the problem**:
   - Search the codebase for relevant symbols, filenames, or error strings mentioned in the issue
   - Read the identified files to understand the current behaviour
   - For rendering bugs, start in `NeuronClient/` or `GameRenderer/`
   - For networking bugs, start in `NeuronCore/`, `NeuronClient/clienttoserver`, or `NeuronServer/`
   - For gameplay bugs, start in `InterstellarOutpost/` or `GameLogic/`

3. **Write a fix**:
   - Make the minimal change that resolves the issue — do not refactor surrounding code
   - Follow the existing code style (see `.clang-format` and `.editorconfig`)
   - For DX12 issues, run the `review-dx12` skill on the changed files first

4. **Verify**:
   - Confirm the fix addresses all symptoms described in the issue
   - Check for related code paths that may have the same bug

5. **Commit and push**:
   - Commit with message: `fix: <short description> (closes #$ARGUMENTS)`
   - Push to the current working branch

6. **Comment on the issue** — Post a brief comment on #$ARGUMENTS explaining what was changed and why.
