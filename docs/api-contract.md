# Practice-bout API contract

Integration contract, 2026-11-16. Every practice endpoint, including GET, requires
`Authorization: Bearer TOKEN` matching `AUSFENCER_ADMIN_TOKEN`. Missing, incorrect
or unconfigured credentials return 403 before parsing or reading records.
Legacy fencer writes require the same token; tracker access remains documented
in [the tracker API guide](API.md). Practice records are excluded from tracker
statistics and competition correction workflows.

## Routes and responses

| Method | Route | Success |
| --- | --- | --- |
| GET | `/api/fencers?q=&page=1&limit=10` | 200, `{"fencers":[...]}` |
| GET/PUT/DELETE | `/api/fencers/<id>` | 200, object/object/`{"success":true}` |
| POST | `/api/fencers` | 200, stored object with generated ID |
| GET | `/api/practice-bouts?page=1&limit=10&fencer_id=1` | 200, `{"bouts":[...]}` |
| GET/PUT/DELETE | `/api/practice-bouts/<id>` | 200, object/object/`{"success":true}` |
| POST | `/api/practice-bouts` | 200, stored object with generated ID |

`fencer_id` is optional; a positive ID matches either side once. Missing Fencers
have empty history. Practice lists use ascending IDs, page >= 1, limit 1–100 (default 10).
Query integers must contain decimal digits only and fit a signed int.
Malformed/empty/zero/out-of-range values return 400; offsets use 64-bit arithmetic.
Concurrent writes can shift numbered pages.

## Bodies

Practice POST and PUT require every mutable field. PUT fully replaces those
fields; the route ID is authoritative. Body `id` and unknown fields are ignored.
Responses include the ID and every mutable field.

Legacy fencer writes retain optional birth year (omitted/null/0 means unknown;
otherwise 1900–2100), text names and a nonblank combined name of at most 200 bytes
following existing Unicode normalization. Public fencer reads omit birth years
and hidden profiles. Fencer search is literal, case-insensitive full-name matching,
ordered by last name, first name, then ID; pages are capped at 1,000,000.

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
numeric IDs are errors, never guests. Equal non-null participants are rejected with 400. Bounds are storage
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

Schema version 6 adds `practice_bouts` separately from the tracker's `bouts`.
Existing tracker databases upgrade without changing competition records. Old
standalone `bout/crud` databases are disposable and have no migration support.
Use a fresh database instead. Operator DELETE removes practice records permanently;
competition corrections retain the existing audited workflow.

Example (replace IDs with existing fencers, or use null for guests):

```sh
curl -X POST http://127.0.0.1:8080/api/practice-bouts \
  -H "Authorization: Bearer $AUSFENCER_ADMIN_TOKEN" \
  -H 'Content-Type: application/json' \
  -d '{"left_fencer_id":null,"right_fencer_id":null,"timestamp":1700000000,"weapon":"foil","time":180,"left_score":15,"right_score":12,"left_yellow":0,"right_yellow":0,"left_red":0,"right_red":0}'
```

Returns 200 with the supplied fields and a generated integer `id`.
