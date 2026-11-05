# Implementation handoff

Updated: 2026-11-05. Active branch: `bout/crud`.

## Current handoff: milestone complete

All five continuation-plan steps are complete. Both Fencer and Bout APIs run in
production, including participant history, anonymous guests, validated inputs,
protected history, persistent upgrades and atomic repository mutation outcomes.
Normal and strict C++17/UBSan builds each pass **48/48** tests, including the
real-server HTTP/restart smoke. No required milestone implementation remains.

Start with [implementation status](implementation-status.md), the
[API contract](api-contract.md), and README for current behavior and commands.
The old checkpoints below are preserved as history, followed by the automated
2026-11-05 implementation log. Existing local workspace policy edits were
preserved. Every new commit uses at most 100 added/deleted lines.

## Historical starting point (2026-10-09)
Starting commit: `c393b68`; working tree was clean.
The [continuation plan](continuation-plan.md) remains the milestone checklist;
this file records implementation progress beyond its reviewed baseline.

## Working agreement

- Implement one small, explainable review increment at a time, with its own commit.
- The 2026-11-05 request authorizes automated implementation/review/test/commit
  cycles through completion, with detailed bullet messages and a 100-line budget.
- Keep Bout CRUD and its prerequisites on `bout/crud`. Use separate branches for
  independent features; do not fold unrelated cleanup into this work.
- Update this handoff in each implementation commit with the change, evidence,
  outstanding issues, and exact next increment. Check Git status before resuming.

## Historical increment: enable Weapon persistence (V01)

Added the four sqlite_orm enum traits in `include/db/weapon_mapping.hpp` and
included them before schema construction. `Bout::weapon` remains a `Weapon`;
SQLite stores INTEGER values `Foil=0`, `Epee=1`, `Sabre=2`. The existing numeric
values are now explicit in the enum. This selects persistence only; the external
JSON representation remains undecided. Unknown stored integers throw
`std::domain_error` before conversion to the enum, including values beyond `int`.

Runtime tests exposed another prerequisite blocker (B11): the schema declaration
interleaved foreign-key constraints with columns. The pinned ORM emits that order,
and SQLite rejects it near `right_fencer_id`. Moved both constraints to the end of
the table declaration, preserving their references and default actions.

Added seven focused persistence tests to the existing test target: three weapon
insert/read cases, three updates, and rejection of an unknown stored integer.
They check the actual column type, integer values, and enum predicates/extraction.
The copied Bout repository test file is still included and unchanged.

Validation:

- `cmake --build build --target aus-fencer --parallel 2` passed.
- The new tests and the three existing DbManager tests compiled with strict C++17
  and passed: **10/10**. Reproduce the focused run from the repository root:

  ```sh
  c++ -std=c++17 -pedantic-errors -Iinclude -Ibuild/_deps/sqlite_orm-src/include -Ibuild/_deps/gtest-src/googletest/include tests/test_main.cpp tests/db/test_weapon_persistence.cpp tests/db/test_db_manager.cpp src/db/db_manager.cpp build/lib/libgtest.a -lsqlite3 -pthread -o build/weapon_persistence_tests
  ./build/weapon_persistence_tests
  ```

- The full test target still fails compilation in B01's copied Bout tests; CTest
  has not run. The focused command does not replace the normal test target.
- `git diff --check` passed. No server was launched or runtime database created.
- FK enforcement, file-backed reopen/schema upgrades, and the full repository and
  handler suites remain unverified. B08 is still open.

Review with `git show HEAD`. V01 and B11 are resolved; step 1 remains incomplete.
Stop here for manual review.

## Completed increments

- `de1193d`: B07, raise CMake minimum to the pinned dependencies' declared 3.16.
  Configuration passed with CMake 4.4.4; 3.16 itself was not tested.
- Latest commit: V01 integer Weapon mapping and B11 schema constraint ordering.

## Environment and baseline evidence

Available prerequisites:

| Component | Observed version |
| --- | --- |
| CMake | 4.4.4 |
| GCC / G++ | 16.2.1 |
| Crow | Installed CMake package; headers identify version as `master` |
| Boost | 1.92.0 |
| System SQLite | 3.53.4 |
| sqlite_orm | 1.9.1, commit `5f1a2ce84a3d72711b4f0a440fdaba977868ae67` |
| GoogleTest | Commit `063de7e9578f82b369302001269680b4b1553359` |

- Initial `cmake -S . -B build` failed because the sandbox could not resolve
  GitHub. The approved retry fetched both pinned revisions and configured.
- Baseline `cmake --build build --parallel 2` failed. The log is in the ignored
  `build/baseline-build.log`; it must not be relied on in a fresh checkout.
- **B01 reproduced:** copied Bout tests use invalid initializers, Fencer members,
  and the wrong list signature. Keep the file in the test target while repairing it.
- **V01 confirmed in the baseline, now resolved above:** schema synchronization
  could not compile because `sqlite_orm::type_printer<Weapon>` lacked `print`.
  Inspect the pinned
  `examples/enum_binding.cpp` for the customization interface. Insertion and
  extraction are verified by the latest increment's tests.
- **V03 linkage verified:** pinned ORM CMake links `SQLite::SQLite3` through its
  interface target; the generated application link command includes
  `/usr/lib/libsqlite3.so`. No extra application SQLite link is needed here.
  Schema-upgrade behavior remains unverified.
- CMake reports a Boost policy warning and the pinned ORM's deprecated SQLite
  target name. These are warnings, not the observed build blockers.
- These are historical baseline observations; use the latest validation above
  for current implementation status.

## Historical next review increment (superseded)

After review, repair B01's copied Bout fixtures/CRUD tests. Use FencerRepo and
BoutRepo sharing one in-memory DbManager, initialize every Bout field, and cover
generated IDs, complete field round trips/updates, missing IDs, removal, empty
lists, and distinct page contents. Keep history filtering/ordering and FK behavior
checks as subsequent review units; do not bundle the handler implementation here.
B08 (C++17 initialization) also remains open. Do not claim a green baseline until
both application and test targets build and the existing suites pass.

Guest support, deletion policy, and all other step 2 API decisions remain open.

## Automated increments (2026-11-05)

- test: repair Bout CRUD fixtures and restore the full build: Replace copied Fencer fields with complete Bout round trips and updates using real FK participants; select Crow Boost networking for both targets. Full build and 25/25 CTest cases pass. Next: ordered pagination and history tests.

- fix(db): order pages and use numeric participant history predicates: Use ascending IDs for both repositories and numeric equality for either participant. Add empty, distinct-page, right-side, unrelated, duplicate-side and filtered-page checks; full build and 26/26 tests pass. Next: safe repository pagination and FK checks.

- fix(db): bound pagination and verify foreign key enforcement: Compute offsets in int64_t, enforce page >= 1 and limit 1-100 in both repositories. Verify left/right FK failures on create/update and referenced-Fencer deletion without data loss; full build and 29/29 tests pass. Next: record the API contract.

- docs: settle Bout fields, guest policy and HTTP errors: Select nullable anonymous guests, protected Fencer history, string weapons, explicit timestamps and elapsed time. Specify complete PUT bodies, bounded strict pagination and status codes before handler implementation. Reviewed against repositories; existing full build and 29 tests remain green. Next: nullable persistence and upgrade checks.

- feat(db): allow anonymous guests and preserve legacy bout records: Map participant IDs to optional integers and use preserving schema synchronization. Upgrade an exact legacy table, verify all fields, guest insertion, generated IDs, FK rejection and file-backed reopen; full build and 30/30 tests pass. Next: atomic mutation outcomes.

- fix(db): report mutation existence under the shared database lock: Return bool from updates/deletions after checking existence while holding the same manager lock. Add coordinated asynchronous delete-before-update coverage and Fencer missing/success outcomes; full build and 32/32 tests pass. Next: shared HTTP validation.

- feat(http): add strict JSON field and query parsers: Add object, required-field, bounded integer, name and complete decimal-query validation. Use the helpers for Fencer creation with standard C++17 initialization; full build and 32/32 tests pass. Next: consistent route exception translation and invalid-input tests.

- fix(http): translate validation and database exceptions into JSON errors: Wrap Fencer route actions with deliberate 400/409/500 responses and sanitized database failures. Exercise malformed/non-object JSON, wrong types, huge integers and blank names without writes; full build and 33/33 tests pass. Next: finish Fencer query and mutation validation.

- fix(http): validate Fencer replacement bodies and pagination: Use strict bounded queries, C++17 aggregate initialization and atomic repository outcomes for PUT/DELETE. Populate real Crow query parameters in regression tests; verify search, invalid queries, unchanged rejected writes and missing mutations. Full build and 34/34 tests pass. Next: Bout JSON conversion.

- feat(http): define complete Bout JSON validation and serialization: Implement all Bout fields, explicit null guests, weapon strings, bounded scores/cards/elapsed time and checked epoch seconds. Strict C++17 conversion probe passes; existing 34/34 tests remain green. Next: replace copied Bout collection handling.

- feat(http): implement ordered Bout collection and participant filtering: Replace copied Fencer search with page/limit and optional fencer_id, serialize complete Bout objects and document guest keys. Full current build and 34/34 tests pass; Bout handler remains outside targets until remaining scaffold methods are repaired. Next: item retrieval and creation.

- feat(http): implement Bout item retrieval and creation: Retrieve actual Bout fields and return complete created objects with generated IDs using the selected validation contract. Full current build and 34/34 tests pass; final copied update method remains unwired. Next: replace mutations and compile the complete handler.

- feat(http): complete Bout mutations and include the handler in both targets: Implement full replacement and atomic missing-row deletion, wrap every Bout route with JSON error translation, and compile Bout repository/handler in production and tests. Full build and 34/34 existing tests pass. Next: production registration and real Bout route tests.

- feat(server): register Bout CRUD and remove automatic seeded data: Register both handlers over one DbManager, ignore SQLite runtime files and remove startup test inserts. Add actual Crow Bout CRUD tests checking every response field and missing-item outcomes; full build and 35/35 tests pass. Next: invalid writes, guest/history integration and persistence conflicts.

- test(http): cover malformed Bout bodies and field validation: Exercise every field with wrong JSON types, missing keys, null scalar values, negative/overflow bounds and invalid weapons. Verify rejected POST/PUT bodies preserve saved data; full build and 37/37 tests pass. Next: guests, complete replacements and history-preserving conflicts.

- test(http): verify guest bouts, complete replacement and protected history: Exercise every mutable field on PUT, route-ID authority, both FK conflict sides, rejected-write preservation, one/both anonymous guests and all weapon strings. Referenced Fencer deletion returns 409 and becomes possible after removing the reference. Full build and 38/38 tests pass. Next: query and unexpected-failure coverage.

- test(http): cover Bout query parsing and sanitized database failures: Exercise actual Crow history/filter pagination, duplicate-side matching, missing-Fencer history and strict query limits/overflow. Reject timestamp overflow and verify unexpected SQLite failures produce sanitized JSON 500 responses; full build and 40/40 tests pass. Fresh configuration also exposed a separate installed Crow package dependency issue, to fix next.

- build: configure Crow headers directly with the supported Boost backend: Use a shared header-only HTTP target with explicit Boost and Threads dependencies; avoid the installed Crow package unconditional standalone-Asio requirement. Fresh-cache configuration, both targets and 40/40 tests pass without cached Asio paths. Next: server lifecycle and startup status.

- fix(server): honor worker settings and make setup idempotent: Validate Crow minimum worker count, apply configured concurrency, allow isolated database paths and register/validate routes once. Remove unused running state and return stderr/nonzero on startup exceptions. Repeated setup and unseeded file-backed restarts pass; full build and 42/42 tests pass. Next: live HTTP/restart and startup-failure checks.

- fix(http): reject route IDs before narrowing to repository integers: Review found Crow route IDs could narrow from int64_t to int and target an unrelated row. Validate positive signed-int IDs before GET/PUT/DELETE in both APIs; regression tests cover zero and overflowing IDs while preserving row 1. Full build and 44/44 tests pass. Next: final persistence/concurrency review.

- fix(db): make schema synchronization transactional and verify concurrent deletes: Run preserving schema synchronization inside a SQLite transaction. Verify no orphan references after file-backed upgrade/reopen; coordinated simultaneous deletes return exactly one success in both repositories. Full build and 46/46 tests pass. Next: production HTTP smoke, restart persistence and final documentation.

- test: exercise production HTTP, persistence and startup failure through CTest: Add an isolated real-server smoke test for full CRUD, participant filtering, validation, guests, protected history, restart persistence, no seeds/orphans, configured workers and stderr/nonzero startup failure. Register with CTest when Python 3.7+ is present; requires free port 8080. Full build and 47/47 tests pass. Next: refresh documentation and final strict verification.

- docs: describe the completed Fencer and Bout HTTP APIs: Replace outdated Fencer-only status with implemented routes, CRUD/history, guests, pagination and error semantics; link the selected field contract and keep UI/team/tournament work out of this milestone. Documentation reviewed against 47 passing tests and the production smoke. Next: prerequisites and concrete payload examples.

- fix(build): initialize Crow request flags in a local compatibility copy: Strict live HTTP validation exposed uninitialized connection booleans in installed Crow HTTPParser request moves. Copy headers into each build tree and patch only the known empty default constructor; installed headers remain read-only. Add regression coverage for default flags; normal and strict C++17/UBSan builds both pass all 48 tests, including production HTTP. Next: finalize documentation.

- docs: add reproducible builds, payload examples and persistence semantics: Document system prerequisites, pinned fetches, the local Crow compatibility fix, test/smoke commands, default configuration and database location. Include Fencer/Bout payloads, history queries, field units, bounds, anonymous guests and migration/deletion policy. Reviewed against normal and strict C++17/UBSan runs: 48/48 tests in each. Next: close the continuation checklist and refresh agent context.

- docs: record completed milestone and verified finding resolutions: Add authoritative current status with B01-B11/V01-V04 resolution evidence, API/persistence decisions, normal and strict C++17/UBSan 48/48 results, environment versions and remaining product scope. Mark the original project context as historical. No code changed; documentation cross-checked against passing builds/tests. Next: close the checklist and refresh handoff instructions.

- docs: close every continuation-plan milestone checkpoint: Mark all 30 remaining work items complete and link current resolution evidence and selected API decisions. Keep historical findings and later UI/team/tournament scope clearly separated from delivered CRUD. Reviewed every exit condition against the two passing 48-test runs. Next: update the active handoff and AGENTS entry point.

- docs: replace the stale manual checkpoint with the completed handoff: Lead the handoff with completed scope, 48/48 normal and strict validation, authoritative current documents and the authorized automated workflow. Retain dated baseline checkpoints and per-commit evidence as history. Documentation reviewed; no required milestone work remains. Next: refresh AGENTS while preserving pre-existing workspace policy edits.
