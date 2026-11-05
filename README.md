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

## Installation

sqlite 3 installed in libs
sqliteOrm via fetchcontent

## Usage

```
mkdir build
cd build
cmake ..
make
./aus-fencer
```
