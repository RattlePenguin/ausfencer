# Completed Bout CRUD milestone

Verified 2026-11-05 on `bout/crud`. This is the current implementation status;
[project context](project-context.md) retains the original architectural review,
and [continuation plan](continuation-plan.md) retains its findings/checklist.
The implementation milestone is complete. Later UI/team/tournament work remains
outside this branch's scope.

## Implemented behavior

Both `/api/fencers` and `/api/bouts` support collection GET/POST and item
GET/PUT/DELETE in the real server. Bout lists optionally filter by `fencer_id`
on either side, once per bout. Both repositories order lists by ascending ID.
All handlers validate bodies, integer ranges, query parameters and route IDs.
Updates/deletions report missing rows under the shared database mutex.

Participants are existing positive IDs or explicit nulls for anonymous guests.
Guests create no Fencer records. SQLite enforces numeric references; deleting a
referenced Fencer returns 409 and preserves history. Schema synchronization now
preserves records inside a transaction; the previous non-null participant schema
is covered by file-backed upgrade/reopen and orphan checks.

See [API contract](api-contract.md) for every field, unit, bound and status code,
and [README](../README.md) for setup and example requests.

## Review findings resolved

| Finding | Evidence |
| --- | --- |
| B01, B11 | Real Bout fixtures, complete CRUD tests, valid schema ordering |
| B02 | Complete handler, both CMake targets, shared-manager production routes |
| B03 | Shared strict input helpers, sanitized JSON 400/409/500 errors |
| B04 | Strict page/limit parsing, cap 100, signed 64-bit offsets |
| B05 | No startup insertion; repeated construction and live restarts stay unseeded |
| B06 | Locked mutation outcomes, coordinated concurrent delete/update tests |
| B07, B08 | Minimum CMake 3.16, standard C++17 aggregates, strict compilation |
| B09 | Validated/applied worker count and idempotent setup |
| B10 | Startup failures return nonzero on stderr; subprocess check |
| V01 | Integer enum traits, all weapons persisted and exposed as strings |
| V02 | Invalid left/right references and protected deletes in memory/file/HTTP tests |
| V03 | ORM propagates SQLite linkage; previous schema records survive upgrades |
| V04 | Numeric participant predicates and stable distinct-page assertions |

Additional review fixed oversized route IDs narrowing to unrelated rows. Fresh
configuration also exposed the installed Crow package's standalone-Asio dependency;
the project now uses headers with Boost networking directly. UBSan exposed omitted
request flags in Crow's default constructor. A narrow compatibility fix applies
to a build-local header copy, with regression coverage; system headers stay intact.

## Validation

- Both targets configured/built successfully, including a fresh CMake cache.
- Normal CTest: **48/48 passed** (47 GoogleTest cases plus production HTTP smoke).
- Strict C++17 plus UBSan: **48/48 passed**, including live HTTP and restarts.
- The production smoke covers CRUD/filtering, validation, guests, 409 conflicts,
  persistence after restart, no seeds/orphans, two workers and startup failure.
- `git diff --check` and the 100-added/deleted-line commit budget are enforced.

Strict validation used `-pedantic-errors -fsanitize=undefined
-fno-sanitize-recover=undefined` in `build/clean-config`, reusing the pinned source
checkouts from `build/_deps`. Normal build/test commands are in README. Python
3.7+ enables the HTTP smoke in CTest; it requires a free port 8080.

Observed environment: CMake 4.4.5, GCC 16.2.1, Boost 1.92.0, system SQLite 3.54.0,
Crow headers identifying as `master` (with the local compatibility fix), sqlite_orm
commit `5f1a2ce84a3d72711b4f0a440fdaba977868ae67`, GoogleTest commit
`063de7e9578f82b369302001269680b4b1553359`. CMake 3.16 itself was not tested.

There is no authentication, frontend or general versioned migration framework.
Anonymous guests have no display names/history identity. These are documented
scope boundaries, not unfinished CRUD implementation. Future work is listed at
the end of the continuation plan. Existing workspace policy changes remain local.
