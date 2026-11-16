# Repository Guidelines

## Project Structure & Module Organization

AusFencer is a C++17 HTTP API for fencing club data. Public headers live in `include/`, with implementations in `src/`. Keep HTTP handling in `handlers/`, database access in `db/repos/`, schema and connection management in `db/db_manager.*`, and data structures in `include/models/`. `src/main.cpp` starts the Crow server; `src/server.cpp` wires dependencies and routes.

Tests mirror these layers under `tests/db/` and `tests/handlers/`. `libs/sqlite/` contains bundled SQLite sources; avoid unrelated edits to vendored code. Build output belongs in the ignored `build/` directory. The vanilla frontend lives in `web/`; there is no asset build pipeline.

## Build, Test, and Development Commands

Install a C++17 compiler, CMake, and development packages discoverable by CMake for Crow headers, Boost, SQLite3, OpenSSL and ICU. Configuration fetches pinned sqlite_orm and, when not installed, GoogleTest revisions, so initial setup requires network access.

```sh
cmake -S . -B build                 # Configure dependencies and targets
cmake --build build                # Build the server and tests
ctest --test-dir build --output-on-failure # Run discovered GoogleTest cases
./build/aus-fencer                 # Start the API on port 8080
```

For focused checks, run `./build/tests/ausfencer_tests --gtest_filter='FencerRepoTest.*'`. CMake also exports `build/compile_commands.json` for editor tooling.

## Coding Style & Naming Conventions

Match existing C++ style: two-space indentation, opening braces on the same line, and `#pragma once` in headers. Use PascalCase for types, snake_case for methods and variables, and trailing underscores for member fields. Follow neighboring filenames: models use names such as `Fencer.hpp`; handlers and repositories use `fencers.hpp` and `fencers.cpp`. No formatter or linter configuration is committed.

## Testing Guidelines

Use GoogleTest fixtures and `TEST_F`, with descriptive PascalCase case names. Name files `test_<component>.cpp` and register new files in `tests/CMakeLists.txt`. Database fixtures use SQLite `:memory:` for isolation. Cover changed behavior, missing records, CRUD operations, and relevant search or pagination boundaries. No coverage threshold is configured; run the full suite before submitting.

## Commit & Pull Request Guidelines

Recent commits use short, lowercase summaries such as `added delete` and `updated readme`; no formal prefix convention is evident. Keep commits focused and describe the concrete change. PRs should explain behavior changes, link relevant issues, and record validation commands and results. Include request/response examples for API changes.

## Local Data

The default database is `db.ausfencer`; use `--db build/demo.db` for development. Sample data is inserted only by explicit fixture commands. Keep generated databases and personal data out of commits.

## Integration contract

- Read `docs/api-contract.md` for practice CRUD and `docs/API.md` for tracker routes.
- `Bout`/`BoutRepo` map to `practice_bouts`; competition results use `bouts`.
  Practice records must not enter tracker statistics or competition workflows.
- Every `/api/practice-bouts` request requires `AUSFENCER_ADMIN_TOKEN` as a bearer
  token. Preserve legacy fencer authorization, visibility filtering and public
  birth-year redaction. Unknown birth years remain optional.
- Participants are distinct existing IDs or explicit null guests. PUT replaces
  all mutable fields; referenced fencer deletion fails with 409.
- Retain versioned migrations, CLI fixture/account operations and frontend routes.
  Schema version 6 adds practice storage; old standalone Bout databases need not
  migrate. Keep test databases isolated and never seed at startup.
- Share one DbManager; do not nest repository calls under its mutex. Keep mutation
  existence checks atomic. Validate integer fields before narrowing.
- Follow the session permissions and preserve local `.codex/config.toml` edits.
  Do not request broader filesystem access or bypass denied actions.
- Crow's compatibility patch only modifies a build-local header copy.
