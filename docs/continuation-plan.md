# Continuation plan and logical-flow review

Reviewed baseline: `54e1ffe`, 2026-10-08. This document records future work;
no application, test, or build defects were fixed during the review.
Read [project context](project-context.md) for architecture, contracts, and setup.

Implementation began on 2026-10-09 on `bout/crud`. Read the
[implementation handoff](implementation-progress.md) for the latest change,
validation results, manual review checkpoint, and next small increment.

The next milestone is a buildable, tested Bout CRUD API with participant history
retrieval and an explicit guest-participant policy. Keep the existing layered
design. Mobile refereeing, teams, and tournaments are separate later milestones.

## Review findings to carry into implementation

Priorities: P0 blocks compilation/integration; P1 affects correctness or data;
P2 covers robustness and follow-up cleanup. “Confirmed” means directly established
from source or an isolated probe, not necessarily observed in a running server.

### B01 — P0: Bout repository tests are still Fencer tests (confirmed)

Evidence: `tests/db/repos/test_bout_repo.cpp:18`, `:36`, `:61`.
Fixtures put strings into integer participant fields, assertions reference
nonexistent names/birth year, and list calls pass three arguments to a two-argument
method. `tests/CMakeLists.txt:5` includes this file in the default build.
A model-only compiler probe rejected the initializer and missing member.
Replace the copied tests with fully initialized Bout fixtures and real referenced
Fencers; retain and extend CRUD coverage. Do not hide the failing file to obtain
a green build.

### B02 — P0: Bout handler is a non-compiling scaffold and is not wired (confirmed)

Evidence: `src/handlers/bouts.cpp:44`, `:66`, `:75`, `:100`;
`include/handlers/bouts.hpp:11`; `CMakeLists.txt:27`;
`tests/CMakeLists.txt:1`; `src/server.cpp:30`.
The handler uses `get_all(q,page,limit)` and nonexistent Fencer fields.
Neither CMake target includes the Bout handler; production also omits BoutRepo.
Server setup registers only Fencer routes. Replace parsing/serialization with the
real Bout contract, add handler tests, add sources, and register `/api/bouts`.
This is unfinished integration, not a working API regression.

### B03 — P1: Malformed Fencer writes are not handled as client errors (confirmed)

Evidence: `src/handlers/fencers.cpp:72` and `:92`.
POST/PUT dereference JSON fields without validating parsing, object shape, keys,
types, or narrowing from JSON integer to `int`. Invalid bodies can throw, and no
local handler translates those failures to the existing 400 JSON helper.
Database exceptions are also untranslated in all Fencer operations. Actual Crow
fallback status/body depends on the installed version and was not executed here.
Add deliberate validation and error translation to both handlers rather than
copying this flow into Bout CRUD.

### B04 — P1: Pagination permits signed overflow and unbounded requests (confirmed)

Evidence: `src/handlers/fencers.cpp:20`, `:31`;
`src/db/repos/fencers.cpp:37`; `src/db/repos/bouts.cpp:38`, `:50`.
HTTP accepts page `2147483647` with limit 10, and `(page-1)*limit` then overflows
`int`. A UBSan probe of the exact expression reported signed overflow. No maximum
limit exists. `stoi("2junk")` is also accepted as 2, while invalid inputs fall back
silently. Specify query validation, cap limits, and calculate/check offsets in a
wider type before narrowing or binding. Repositories need safe argument handling
too; valid HTTP inputs must not make their arithmetic undefined.

### B05 — P1: Starting the server repeatedly inserts test data (confirmed)

Evidence: `src/server.cpp:14`–`:17`.
Every Server construction inserts a hard-coded Fencer into the persistent
database. Repeated starts grow real data even without requests. Remove automatic
test insertion in the implementation phase; use explicit test fixtures instead.
Add database ignore patterns as related cleanup. The direct insertion happens
before worker startup, so its lack of locking alone is not a demonstrated race.

### B06 — P1: Existence checks and mutations are separate operations (confirmed)

Evidence: `src/handlers/fencers.cpp:93`, `:107`, `:118`, `:124`;
repository locks in `src/db/repos/fencers.cpp:19`–`:31`.
Another request can delete a row after the handler's existence check. Since
updates/deletes return no affected-row result, a later no-op can be reported as
success. Decide whether concurrent no-op deletion is acceptable; make update
existence and mutation atomic if the API promises an updated persisted row.
Keep any FK-sensitive participant creation/check/write sequence in an appropriate
transaction and translate final database constraint failures. A pre-check alone
does not prevent a Fencer from disappearing before a Bout insert.

### B07 — P2: Declared CMake minimum is too old (confirmed)

Evidence: `CMakeLists.txt:1`, `:7`, `:18`, `:25`.
The project advertises 3.10, but FetchContent was introduced in 3.11 and
FetchContent_MakeAvailable in 3.14. Raise the minimum to meet all pinned
dependencies after checking their requirements.
Source: [CMake FetchContent documentation](https://cmake.org/cmake/help/latest/module/FetchContent.html).

### B08 — P2: Fencer initialization relies on a newer language feature (confirmed)

Evidence: `CMakeLists.txt:4`; `src/handlers/fencers.cpp:75`, `:100`.
Designated initializers are a C++20 feature despite declared C++17. GCC may accept
an extension; a `-std=c++17 -pedantic-errors` model-only probe rejected the form.
Either use standard C++17 initialization or deliberately adopt C++20 and validate
all dependency/compiler requirements. Do not describe this as a guaranteed
failure under the current default GNU compiler flags.

### B09 — P2: Advertised thread configuration is ignored (confirmed)

Evidence: `include/server.hpp:12`; `src/server.cpp:38`.
Setting `ServerConfig::threads` does not affect the run call. Implement and test
the intended Crow concurrency configuration, or remove the unused setting.
Also define setup lifecycle: calling public `setup()` before `start()` leads to
another route registration because `start()` invokes it unconditionally.

### B10 — P2: Startup errors result in a successful process exit (confirmed)

Evidence: `src/main.cpp:5`–`:15`.
Caught startup exceptions only print to stdout and execution reaches the end of
`main`, returning zero. Return a nonzero status for startup failure and report it
to stderr so scripts can detect failure.

## Checks and decisions not yet proven to be bugs

- **V01 — Weapon persistence, verify first.** `Bout::weapon` is an enum mapped
  directly in `include/db/db_manager.hpp:34`, with no custom binding in this repo.
  sqlite_orm's [enum binding example](https://github.com/fnc12/sqlite_orm/blob/master/examples/enum_binding.cpp)
  demonstrates custom type printing, binding, extraction, and field printing.
  This makes missing support a likely compile blocker, potentially even in the
  Fencer executable because DbManager syncs the full schema. The pinned revision
  was unavailable, so inspect and compile it before declaring failure or choosing
  adapters versus an integer persistence field. Test all three weapons.
- **V02 — FK enforcement and deletion policy.** Two references exist, but effective
  connection PRAGMAs and enforcement were not tested. Do not label foreign keys
  disabled solely because DbManager has no explicit PRAGMA. Verify invalid insert
  and update rejection, referenced-Fencer deletion, and orphan checks on memory
  and file databases. If enforced, existing Fencer DELETE can now fail on a
  referenced row and needs an intentional status; otherwise orphan risk exists.
  Prefer preserving Bout history over silently cascading it away.
- **V03 — SQLite linkage and schema upgrades.** Confirm the pinned ORM propagates
  SQLite's imported target; production does not explicitly link it, while tests
  do. Do not assume a linker bug without checking. Inspect actual DDL and schema
  synchronization before changing guest fields or enum storage; verify existing
  records survive upgrades. Vendored SQLite is currently unused by the targets.
- **V04 — Query semantics and ordering.** `get_all_by_fencer` uses LIKE on integer
  IDs. An isolated SQLite probe showed 1 did not match 10, 11, or 101. Prefer
  numeric equality for intent and indexing, without claiming a reproduced
  substring-match bug. All list queries lack ORDER BY: choose a stable order
  before promising reproducible pages and test page contents, not just lengths.
- **Product decisions:** duplicate Fencers are allowed; guest identity, same
  Fencer on both sides, timestamp defaults/units, time meaning, yellow/red bounds,
  and invalid-query behavior are not settled. Names in `q` currently allow LIKE
  wildcards. These are contract questions, not automatically defects.

## Work sequence

### 1. Establish the build and repair the Bout repository baseline

- [x] Provision compatible CMake, Crow, Boost, SQLite development files, compiler,
  and access to the pinned FetchContent repositories. Record versions.
- [ ] Verify V01 and V03 against the exact pinned sqlite_orm, then resolve confirmed
  dependency/build issues, B07, and B08 without unnecessary dependency upgrades.
- [ ] Replace B01's copied tests. Create two or more Fencers through FencerRepo
  sharing the fixture's DbManager, then initialize every Bout field explicitly.
- [ ] Cover generated IDs, every field round trip, missing IDs, all mutable fields
  on update, removal, empty lists, and multiple pages with distinct results.
- [ ] Verify left/right participant matching, exclusion of unrelated Bouts, one
  result when a Bout matches both sides, and filtered pagination. Address V04.
- [ ] Verify V02 with both invalid left and invalid right IDs on create/update and
  deletion of referenced Fencers. Establish a green repository/Fencer baseline.

Exit condition: the application target and repaired existing test target build;
all repository tests pass; persistence and FK behavior are recorded.

### 2. Settle the Bout contract and participant policy

- [ ] Write the selected request/response shapes and validation/error rules before
  completing the copied handler. Proposed routes: collection GET/POST and item
  GET/PUT/DELETE at `/api/bouts`, plus collection query `fencer_id` to use the
  existing participant-filter repository method. This routing is a proposal.
- [ ] Decide guest support. Existing `int` FK fields require existing Fencers.
  Options are nullable participant IDs (with optional stored display names), or
  guest Fencer records created as part of the Bout operation. Nullable IDs better
  match “without creating a fencer”; they require model/schema changes and lose
  identity-based history for anonymous guests. Guest records preserve identity
  but need explicit metadata/defaults and transaction handling. Do not invent a
  sentinel ID, weaken FK enforcement, or equate unknown IDs with intentional guests.
- [ ] Set referenced-Fencer deletion behavior. Recommended initial policy: reject
  deletion with a clear 409 error while history references the Fencer. Guest/null
  or soft-deletion alternatives need explicit migration and history semantics.
- [ ] Set weapon JSON representation and persistence together. A proposed external
  representation is `"foil"`, `"epee"`, `"sabre"`; validate before conversion.
- [ ] Define timestamp as an explicit unit/convention (proposed UTC epoch seconds),
  who supplies/defaults it, and overflow-safe conversion to `time_t`.
- [ ] Define `time` meaning and accepted range, scores 0–99, card count bounds, and
  whether equal participant IDs are valid. Do not derive strict fencing rules
  solely from informal model comments.
- [ ] Define PUT as full replacement or document different semantics. Use route ID
  as authoritative. Proposed responses retain Fencer-style object/list envelopes
  and 200 success statuses for consistency; deliberately choose alternatives.
- [ ] Define 400 validation errors, 404 absent resources, 409 FK/conflict errors,
  and 500 unexpected database failures. Avoid exposing raw database diagnostics.
- [ ] Select bounded, strictly parsed pagination and stable ordering for both APIs.

Exit condition: guest and deletion policies, all field semantics, routes, and
error statuses are written down as selected decisions, not left as assumptions.

### 3. Finish Bout handlers and production integration

- [ ] Replace B02's stale Fencer fields and list signature; serialize every Bout
  field consistently for collection, item, create, and update responses.
- [ ] Implement the selected validation, FK/guest policy, and error translation.
  Reuse parsing/serialization helpers where useful without broad restructuring.
- [ ] Implement participant filtering and safe pagination. Do not reinterpret
  stale `q` name-search comments as an existing Bout search requirement.
- [ ] Add `src/db/repos/bouts.cpp` and `src/handlers/bouts.cpp` to production;
  add the handler source and new handler tests to the test target.
- [ ] Register a BoutHandler using the same DbManager as FencerHandler.
  Preserve handler lifetimes for Crow callbacks.
- [ ] Address B05 before using a persistent server smoke test.

Exit condition: both APIs are reachable in the real Server and use the same
database; starting it does not insert unsolicited data.

### 4. Fix existing flow defects and add regression coverage

- [ ] Resolve B03 and B04 for Fencer and Bout routes. Cover malformed JSON,
  non-object JSON, missing/wrong-type fields, narrowing overflow, invalid weapons,
  participant errors, score/card/time bounds, and query overflow/limits.
- [ ] Resolve B06 with atomic persistence semantics or explicitly documented
  accepted no-op behavior. Test the chosen behavior with coordinated concurrent
  operations, not timing-dependent sleeps. Do not nest locks on the same manager.
- [ ] Test collection/filter query handling with genuinely populated Crow query
  parameters; the existing synthetic helper does not parse them automatically.
- [ ] Cover missing item GET/PUT/DELETE, unchanged data after rejected writes,
  generated IDs, all fields in responses, and history-preserving Fencer deletion.
- [ ] Add a file-backed reopen/persistence check and representative schema-upgrade
  checks if step 2 changes storage. Verify no orphaned references remain.
- [ ] Resolve B09 and B10 as small separate follow-ups, with behavior checks.

### 5. Verify the milestone and refresh context

- [ ] Configure/build both targets and run CTest. Record actual results and versions.
- [ ] Smoke-test the real server in a temporary working directory: create Fencers,
  create/get/list/filter/update/delete a Bout, exercise chosen guest and deletion
  behavior, then restart and verify retained data and absence of seeded entries.
- [ ] Update README with prerequisites, payload examples, routes, field meanings,
  error responses, database location, and the chosen guest policy.
- [ ] Refresh AGENTS/context status and mark findings resolved with evidence.

Definition of done: builds and tests pass; Bout CRUD and participant history work
through production routes; invalid writes cannot silently corrupt data; guest
and referenced-Fencer behavior match the documented policy; earlier defects have
either been resolved or explicitly remain tracked with a follow-up scope.

## Later work, after this milestone

- Name-based Bout discovery can first search Fencers, then request their history;
  additional search filters and ID-only responses remain unimplemented ideas.
- Model team membership and ordered legs separately from the two-ID individual
  Bout model; decide scoring/history relationships before extending the schema.
- Build the mobile referee timer/score/card UI against the settled API. Live timer
  synchronization or remote equipment control is not supported by current code.
- Revisit tournaments only when product scope calls for them; README currently
  marks them deferred.
