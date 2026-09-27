# REVIEW.md

Note: review the code in a subagent before merging.

## Task for the review subagent

Review the changes on the current branch against `master`
(`git diff master...HEAD`), plus any touched test files.

## Scope sizing

- First take stock of which files the change touches and how large it is,
  then size the review to that list.
- A small change gets a few focused reviewers plus one verify vote,
  not the full multi-angle shape.
- Scale the number and diversity of reviewers to the request,
  then stop on evidence state or an explicit caller or runtime boundary.
- A fixed batch count never proves completion.

## Rules (same as the original review protocol)

- Discovery pointers are not inspected evidence when the body is readable.
  Open and read every implementation and test body you cite.
  Search and grep output only locates candidates.
- Back every assigned claim with inspected evidence, or name it as unresolved.
- Track `complete` (boolean), `evidence` (list), and `unresolved` (list).
  Treat an unsuccessful result, missing or wrong-typed data,
  `complete !== true`, or nonempty `unresolved` as incomplete.
- Never use data from an unsuccessful result as evidence or gap disposition.
- Critic unavailability is a synthesis note, never a research gap.
- Preserve compact evidence, provenance refs, and every unresolved item
  in synthesis. Synthesize from compact structured results,
  not from uninspected summaries. Disclose omitted scope.
- Use independent verification when a claim has materially different
  failure modes. Repeating the same prompt is not independent coverage.
- Split reviewers across genuinely different evidence surfaces
  (implementation, tests, design records, operational traces).
  Different role names do not increase coverage with the same search plan.
- Adversarial verification: give the skeptic a concrete falsification target
  for each material claim; retain only claims that survive inspected
  counterevidence, and mark an unavailable or invalid verdict unresolved.
- For behavior, abuse resistance, and a reported failure, use separate
  correctness, security, and reproduction lenses,
  each with a distinct falsification target.

## Convergence

- Open discovery and a known evidence gap are different jobs.
- Open discovery: track seen items across rounds; a round with no fresh item
  is dry; stop discovery after two consecutive dry rounds.
  A caller limit, capacity boundary, or runtime budget may stop it earlier;
  that stop is partial unless all requested scope is covered.
- Explicit gap follow-up: dispatch exactly one focused follow-up per gap
  lineage, then carry the narrowed, reworded, or still-unresolved descendant
  unchanged into synthesis. Never re-dispatch its new wording as a new gap.

## Checklist

- Correctness: logic errors, edge cases, off-by-one, error handling.
- Security: injection, unsafe input, secrets.
- Concurrency: races, deadlocks, lifetime issues.
- Style: comments max 2 lines (see AGENTS.md); match surrounding conventions.
- Tests: changed behavior covered; no weakened assertions or skipped tests.

## Report format (adapted for agent reporting)

- Findings ordered by severity: blocker / major / minor.
- Each finding: `path:line`, short description (1-2 lines), suggested fix.
- End with a one-line verdict: approve / approve with comments / request changes.
- State omitted scope for top-N, sampling, no-retry, capacity, caller limit,
  and runtime budget boundaries; never describe a bounded sample as exhaustive.
