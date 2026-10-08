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

## Latest increment: correct the CMake minimum (B07)

Changed the top-level minimum from 3.10 to 3.16. Both exact pinned dependencies
declare 3.16 in their root CMakeLists.txt, so 3.14 (the FetchContent requirement)
would still understate the build requirement. Dependency pins are unchanged.

Review this increment with `git show HEAD`. This handoff and the checklist update
belong to the same commit as the CMake change. B07 is resolved; step 1 as a whole
is still incomplete. Stop here for the user's manual review.

Validation: `cmake -S . -B build` passed after the change using CMake 4.4.4;
`git diff --check` passed. CMake 3.16 itself was not installed/tested. The minimum
comes from the pinned dependency declarations. The baseline compilation failures
below remain; no test suite could run.

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
- **V01 confirmed as a blocker:** schema synchronization cannot compile because
  `sqlite_orm::type_printer<Weapon>` lacks `print`. Inspect the pinned
  `examples/enum_binding.cpp` for the customization interface. Insertion and
  extraction also need verification when adding support.
- **V03 linkage verified:** pinned ORM CMake links `SQLite::SQLite3` through its
  interface target; the generated application link command includes
  `/usr/lib/libsqlite3.so`. No extra application SQLite link is needed here.
  Schema-upgrade behavior remains unverified.
- CMake reports a Boost policy warning and the pinned ORM's deprecated SQLite
  target name. These are warnings, not the observed build blockers.
- FK enforcement, weapon round trips, and runtime tests remain unverified.
  No server was launched and no runtime database was created.

## Next review increment

After review, address V01 only: add explicit sqlite_orm support for `Weapon`,
preserving the typed Bout field, and verify all three weapon round trips. Integer
storage matching existing enum values is a candidate; record the chosen mapping
and evidence. This does not settle the external JSON representation.

Then repair B01's Bout fixtures/CRUD tests in a separate commit. Follow with
participant filtering/ordering and FK behavior checks as separate review units.
B08 (C++17 initialization) also remains open. Do not claim a green baseline until
both application and test targets build and the existing suites pass.

Guest support, deletion policy, and all other step 2 API decisions remain open.
