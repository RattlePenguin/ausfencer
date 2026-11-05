# AusFencer

A C++17 HTTP backend for Fencers and completed fencing bouts, using Crow,
sqlite_orm and SQLite. This backend has no authentication or frontend.

## Features

- Fencer CRUD and first/last-name search.
- Bout CRUD with all scores, cards, elapsed time and weapon fields.
- Participant history filtered by either side, with stable paginated lists.
- Anonymous guests using explicit null participant IDs.
- Validated requests and foreign keys that protect stored bout history.

## HTTP API

| Resource | Collection GET/POST | Item GET/PUT/DELETE |
| --- | --- | --- |
| Fencer | `/api/fencers` | `/api/fencers/<id>` |
| Bout | `/api/bouts` | `/api/bouts/<id>` |

Success responses use status 200, item objects, list envelopes (`fencers` or
`bouts`), and `{"success":true}` for deletion. PUT requires all mutable fields.
List queries accept `page` (default 1) and `limit` (default 10, maximum 100).
Fencers also accept `q`; bouts accept `fencer_id` to match either participant.
Invalid input returns JSON 400, missing items 404, participant/history conflicts
409, and unexpected failures 500. See the [complete API contract](docs/api-contract.md)
for request fields, exact validation bounds and guest/history semantics.

Mobile referee screens, live scoring, teams and tournaments are future work.

## Build and test

Requires CMake 3.16+, a C++17 compiler, Crow headers supporting `CROW_USE_BOOST`,
Boost headers, system SQLite development files, and pthreads. Initial CMake
configuration fetches the pinned sqlite_orm and GoogleTest revisions from GitHub.
Vendored `libs/sqlite/` is not used. The build copies Crow headers locally and
initializes request connection flags when the installed constructor omits them;
system headers are never modified (see `cmake/crow-compat.cmake`).

```sh
cmake -S . -B build
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/aus-fencer
```

With Python 3.7+, CTest also runs the production HTTP/restart smoke test. It needs
port 8080 free and uses temporary databases. Run it separately with:

```sh
python3 tests/smoke_test.py build/aus-fencer
```

The server listens on port 8080 with two workers. It stores `db.ausfencer` relative
to the launch directory, preserves records across restarts and adds no test data.
C++ callers can set port, worker count (at least two), and database path through
`ServerConfig`. The executable uses defaults; there are no CLI or environment
configuration options. Startup exceptions return a nonzero status on stderr.

## Example requests

Create two participants, then use their returned IDs (shown as 1 and 2 here):

```sh
curl -H 'Content-Type: application/json' -d '{"first_name":"Alice","last_name":"Smith","birth_year":2000}' http://localhost:8080/api/fencers
curl -H 'Content-Type: application/json' -d '{"first_name":"Bob","last_name":"Jones","birth_year":2001}' http://localhost:8080/api/fencers
curl -H 'Content-Type: application/json' -d '{
  "left_fencer_id":1, "right_fencer_id":2,
  "timestamp":1700000000, "weapon":"foil", "time":180,
  "left_score":15, "right_score":12,
  "left_yellow":1, "right_yellow":0, "left_red":0, "right_red":1
}' http://localhost:8080/api/bouts
curl 'http://localhost:8080/api/bouts?fencer_id=1&page=1&limit=10'
```

`timestamp` is required UTC epoch seconds; `time` is elapsed seconds (0-86400).
Weapons are `foil`, `epee`, `sabre`; scores/red counts are 0-99 and yellow counts
0-1. These bounds do not enforce every competition rule.

Use explicit `null` for either participant to record an anonymous guest without
creating a Fencer. Guests have no stored display names or identity history.
Unknown numeric IDs return 409. Deleting a referenced Fencer also returns 409;
remove the referring bouts first. Existing databases upgrade participant columns
to nullable fields using preserving schema synchronization in a transaction.
The upgrade/reopen test covers the previous schema; this is not a general
versioned migration system.
