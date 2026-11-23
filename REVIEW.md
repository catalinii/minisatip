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
- Stopping below the sized minimum on a caller or runtime boundary
  is partial coverage; the verdict cap in Report format applies.

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
- Challenge the diagnosis, not just the patch: for a reported failure,
  state each alternative mechanism consistent with the symptoms and
  rule it in or out from inspected code. Untested trust is unresolved.
- Trace changed values to every reader: when a change alters the value
  or domain produced at a site, list every read of that value and
  check each one, not just the readers the description names.
- Fix the class, not the instance: when a change mirrors a precedent
  or handles one member of a kind, list the kind's members and
  disposition each (covered, out of scope with reason, or unresolved).

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
- Tests: changed behavior covered, each pinning both the new outcome
  and the preserved outcome; no weakened assertions or skipped tests.

## Report format (adapted for agent reporting)

- Findings ordered by severity: blocker / major / minor.
- Each finding: `path:line`, short description (1-2 lines), suggested fix.
- End with a one-line verdict: approve / approve with comments / request changes.
- Cap the verdict to the coverage: a bare approve needs every rule
  above applied to each material surface; bounded coverage names its
  gaps as residual risk, verdicting at most approve with comments.
- State omitted scope for top-N, sampling, no-retry, capacity, caller limit,
  and runtime budget boundaries; never describe a bounded sample as exhaustive.
