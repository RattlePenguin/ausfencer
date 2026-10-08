# AusFencer agent entry point

Start with [project context](docs/project-context.md), then read the
[continuation plan and review findings](docs/continuation-plan.md).
These describe the reviewed baseline at commit `54e1ffe` on 2026-10-08;
check the current code and Git status before relying on their status claims.

## Project in one minute

- A C++ HTTP backend for storing fencing participants and completed bouts.
- Crow handles HTTP and JSON; sqlite_orm maps plain structs to SQLite.
- Fencer CRUD is wired into the server. Bout CRUD is unfinished: its model,
  schema, and repository exist, but its handler and tests are still Fencer copies.
- There is no frontend, mobile referee screen, tournament system, or team model.
- Production starts in `src/main.cpp`; dependency construction and route wiring
  happen in `src/server.cpp`.
- The schema lives in `include/db/db_manager.hpp`, not in SQL migration files.
- Repositories serialize each database operation with the shared DbManager mutex.
- `tests/db/repos/test_bout_repo.cpp` is in the test target and contains compile
  errors. A green baseline has not been established.

## Where to work

| Concern | Files |
| --- | --- |
| Data shapes and enum | `include/models/Fencer.hpp`, `include/models/Bout.hpp` |
| Schema, storage, locking | `include/db/db_manager.hpp`, `src/db/db_manager.cpp` |
| CRUD and queries | `include/db/repos/`, `src/db/repos/` |
| HTTP parsing, serialization, routes | `include/handlers/`, `src/handlers/` |
| Production wiring | `src/server.cpp`, `CMakeLists.txt` |
| Test wiring and suites | `tests/CMakeLists.txt`, `tests/db/`, `tests/handlers/` |

## Working guidance

- Follow the handler → repository → DbManager flow. Use the Fencer implementation
  to understand structure, but consult the review before copying its behavior.
- Bout lists use `(page, limit)`; Fencer lists use `(q, page, limit)`.
  A Bout has participant IDs and scoring fields, never Fencer name/birth fields.
- Share one DbManager across both repositories. Direct storage access does not
  lock itself; avoid nesting repository calls under its non-recursive mutex.
- Route callbacks capture handler `this`; keep handlers alive as the Server does.
- Decide guest participants, referenced-Fencer deletion, and weapon persistence
  before treating the Bout API contract as settled. These are open decisions.
- Register any new `.cpp` files in the appropriate CMake target; sources are listed
  explicitly, not discovered automatically.
- Build commands, test coverage, dependency details, and validation limits are in
  the context document. The database path is relative to the launch directory,
  and starting the current server inserts a test Fencer into it.
- The 2026-10-08 task produced documentation only. Listed defects remain unfixed;
  the plan records future work and is not a claim that it has been implemented.
