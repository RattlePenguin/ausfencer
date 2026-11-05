# Implementation handoff

Updated: 2026-10-09. Active branch: `bout/crud`.
Starting commit: `c393b68`; working tree was clean.
The [continuation plan](continuation-plan.md) remains the milestone checklist;
this file records implementation progress beyond its reviewed baseline.

## Working agreement

- Implement one small, explainable review increment at a time, with its own commit.
- Stop after each increment for manual review until the user requests automation.
- Keep Bout CRUD and its prerequisites on `bout/crud`. Use separate branches for
  independent features; do not fold unrelated cleanup into this work.
- Update this handoff in each implementation commit with the change, evidence,
  outstanding issues, and exact next increment. Check Git status before resuming.

## Latest increment: enable Weapon persistence (V01)

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

## Next review increment

After review, repair B01's copied Bout fixtures/CRUD tests. Use FencerRepo and
BoutRepo sharing one in-memory DbManager, initialize every Bout field, and cover
generated IDs, complete field round trips/updates, missing IDs, removal, empty
lists, and distinct page contents. Keep history filtering/ordering and FK behavior
checks as subsequent review units; do not bundle the handler implementation here.
B08 (C++17 initialization) also remains open. Do not claim a green baseline until
both application and test targets build and the existing suites pass.

Guest support, deletion policy, and all other step 2 API decisions remain open.

- test: repair Bout CRUD fixtures and restore the full build: Replace copied Fencer fields with complete Bout round trips and updates using real FK participants; select Crow Boost networking for both targets. Full build and 25/25 CTest cases pass. Next: ordered pagination and history tests.

- fix(db): order pages and use numeric participant history predicates: Use ascending IDs for both repositories and numeric equality for either participant. Add empty, distinct-page, right-side, unrelated, duplicate-side and filtered-page checks; full build and 26/26 tests pass. Next: safe repository pagination and FK checks.

- fix(db): bound pagination and verify foreign key enforcement: Compute offsets in int64_t, enforce page >= 1 and limit 1-100 in both repositories. Verify left/right FK failures on create/update and referenced-Fencer deletion without data loss; full build and 29/29 tests pass. Next: record the API contract.

- docs: settle Bout fields, guest policy and HTTP errors: Select nullable anonymous guests, protected Fencer history, string weapons, explicit timestamps and elapsed time. Specify complete PUT bodies, bounded strict pagination and status codes before handler implementation. Reviewed against repositories; existing full build and 29 tests remain green. Next: nullable persistence and upgrade checks.

- feat(db): allow anonymous guests and preserve legacy bout records: Map participant IDs to optional integers and use preserving schema synchronization. Upgrade an exact legacy table, verify all fields, guest insertion, generated IDs, FK rejection and file-backed reopen; full build and 30/30 tests pass. Next: atomic mutation outcomes.

- fix(db): report mutation existence under the shared database lock: Return bool from updates/deletions after checking existence while holding the same manager lock. Add coordinated asynchronous delete-before-update coverage and Fencer missing/success outcomes; full build and 32/32 tests pass. Next: shared HTTP validation.

- feat(http): add strict JSON field and query parsers: Add object, required-field, bounded integer, name and complete decimal-query validation. Use the helpers for Fencer creation with standard C++17 initialization; full build and 32/32 tests pass. Next: consistent route exception translation and invalid-input tests.
