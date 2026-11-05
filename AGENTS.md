# AusFencer agent entry point

Start with [current implementation status](docs/implementation-status.md), then
read the [API contract](docs/api-contract.md) and
[continuation plan](docs/continuation-plan.md). The Bout CRUD milestone is complete
as of 2026-11-05. [Project context](docs/project-context.md) preserves the historical
review at `54e1ffe`; check current code and Git status before making changes.

## Project in one minute

- A C++17 HTTP backend storing Fencers and completed bouts; no frontend or auth.
- Crow handles HTTP/JSON; sqlite_orm maps structs to system SQLite.
- Fencer and Bout CRUD plus participant history are registered in production.
- Bout participants are existing IDs or explicit nulls for anonymous guests.
  Referenced-Fencer deletion returns 409; history never cascades away.
- Production starts in `src/main.cpp`; wiring happens in `src/server.cpp`.
- Schema lives in `include/db/db_manager.hpp`. Transactional preserving sync
  upgrades the previous non-null participant schema without losing records.
- Both repositories share one DbManager and lock each operation. Update/delete
  return bool for atomic missing-row outcomes under that lock.
- Normal and strict C++17/UBSan builds pass 48 tests, including live HTTP/restarts.

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
- Follow the settled API contract: string weapons, required UTC epoch seconds,
  elapsed seconds, bounded scores/cards, complete PUT bodies and null guests.
- Validate integer fields before narrowing. Lists use ascending IDs, positive
  pages and limits 1-100; offsets use signed 64-bit arithmetic.
- Register any new `.cpp` files in the appropriate CMake target; sources are listed
  explicitly, not discovered automatically.
- Build/test commands and payload examples are in README. Default database path
  is relative to the launch directory; startup inserts no data. ServerConfig also
  supports isolated database paths. HTTP smoke tests need a free port 8080.
- Crow headers are copied into the build directory for a narrow constructor
  initialization compatibility fix; never edit installed dependency headers.
- Keep changes reviewed/tested and commit reasonable chunks with detailed bullet
  messages, at most 100 added/deleted lines per commit. Preserve local policy edits.
