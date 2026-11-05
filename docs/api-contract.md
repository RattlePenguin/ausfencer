# Completed-bout API contract

Selected for the bout/crud milestone, 2026-11-05. Both APIs return JSON.
This backend is unauthenticated; deployment access control is separate work.

## Routes and responses

| Method | Route | Success |
| --- | --- | --- |
| GET | `/api/fencers?q=&page=1&limit=10` | 200, `{"fencers":[...]}` |
| GET/PUT/DELETE | `/api/fencers/<id>` | 200, object/object/`{"success":true}` |
| POST | `/api/fencers` | 200, stored object with generated ID |
| GET | `/api/bouts?page=1&limit=10&fencer_id=1` | 200, `{"bouts":[...]}` |
| GET/PUT/DELETE | `/api/bouts/<id>` | 200, object/object/`{"success":true}` |
| POST | `/api/bouts` | 200, stored object with generated ID |

`fencer_id` is optional; a positive ID matches either side once. Missing Fencers
have empty history. Lists use ascending IDs, page >= 1, limit 1-100 (default 10).
Query integers must consist entirely of decimal digits and fit a signed int.
Malformed/empty/zero/out-of-range query values return 400. Large page offsets use
64-bit arithmetic. Fencer `q` matches first OR last name using SQL LIKE; `%` and
`_` retain their wildcard meaning. Concurrent writes can shift numbered pages.

## Bodies

POST and PUT require JSON objects with every mutable field. PUT fully replaces
those fields; the route ID is authoritative. Body `id` and unknown fields are
ignored. Responses always include the ID and every mutable field.

Fencer fields: `first_name`, `last_name` are strings of 1-100 bytes with at least
one non-whitespace character; `birth_year` is an integer from 1 through 9999.
Duplicate names are allowed and receive independent IDs.

Bout fields:

| Field | Meaning and validation |
| --- | --- |
| `left_fencer_id`, `right_fencer_id` | Existing positive Fencer ID or explicit `null` for a guest |
| `timestamp` | Required nonnegative UTC Unix epoch seconds, representable as `time_t` and signed 64-bit |
| `weapon` | Exactly `"foil"`, `"epee"`, or `"sabre"`; persisted as INTEGER 0/1/2 |
| `time` | Elapsed bout seconds, integer 0-86400 |
| `left_score`, `right_score` | Integer 0-99 |
| `left_yellow`, `right_yellow` | Integer 0-1 |
| `left_red`, `right_red` | Integer 0-99 |

Both participants may be guests. A guest is anonymous: no Fencer record or stored
display name is created, and no identity-based guest history is offered. Unknown
numeric IDs are errors, never guests. Equal non-null participants are allowed
for practice records; history still returns that bout once. Bounds are storage
rules, not a complete enforcement of competition rules. Fractional numbers,
booleans and numeric strings are rejected for all integer fields.

## Errors and persistence

Errors use `{"status":"error","message":"..."}`: 400 for invalid inputs, 404 for
missing items, 409 for nonexistent participant references or deletion of a Fencer
referenced by a Bout, 500 for unexpected failures without raw database details.
Invalid input is checked before item existence on PUT. Rejected writes leave
stored data unchanged. FK enforcement is retained; Bout history never cascades
away when deleting a Fencer. Delete the referring bouts first to remove a Fencer.
Updates/deletions report missing rows atomically with the mutation under the
shared DbManager lock. SQLite enforces participants at write time.

Nullable participant columns require a preserving schema synchronization of
existing databases. Upgrade/reopen tests must prove existing IDs, fields and
foreign keys survive; `sync_schema()` alone is not a general migration system.
