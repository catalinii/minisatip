# REVIEW.md

Note: review the code in a subagent before merging.

## Task for the review subagent

Review the changes on the current branch against `master`
(`git diff master...HEAD`), plus any touched test files.

## Rules

- Open and read every implementation and test body you cite.
  Search and grep output only locates candidates; it is not evidence.
- Back every claim with inspected evidence, or name it as unresolved.
- Keep each finding to 1-2 lines (see AGENTS.md).

## Checklist

- Correctness: logic errors, edge cases, off-by-one, error handling.
- Security: injection, unsafe input, secrets.
- Concurrency: races, deadlocks, lifetime issues.
- Style: comments max 2 lines; match surrounding code conventions.
- Tests: changed behavior covered; no weakened assertions.

## Report format

- Findings ordered by severity: blocker / major / minor.
- Each finding: `path:line`, short description, suggested fix.
- Track `complete` (boolean), `evidence` (list), `unresolved` (list).
- End with a one-line verdict: approve / approve with comments / request changes.
- Disclose any omitted scope (sampling, limits, unreviewed files).
