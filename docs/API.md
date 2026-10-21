# Demo tracker API

Start the seeded server from the repository root; see [setup](../README.md) and
the [fixture guide](../fixtures/README.md). All data in this pilot is fictional.

## Public reads

| GET route | Result |
| --- | --- |
| `/api/tracker/fencers?q=Mira%20Vale&page=1&limit=20` | Unicode/full-name/alias search; duplicate names retain separate IDs and affiliations |
| `/api/tracker/fencers/101` | Public profile, canonical bout history, statistics, annual breakdown and event placings |
| `/api/tracker/events` | Events with local dates, weapon, category, timezone and coverage |
| `/api/tracker/events/epee-2026` | Event metadata, entrant snapshots, seeds, placings, byes and canonical bouts |
| `/api/tracker/bouts/1` | Bout, explicit winner side, status, dispute state, provenance, update time and revision |

Profile filters: `weapon=foil|epee|sabre`, `year=2026`, `stage=pool|de|R16|QF|SF|F`,
`trust=synthetic|community|organiser`, `category=open`, `from=2026-01-01`,
`to=2026-12-31`, `opponent=107`, and
`coverage=complete|partial|results-only|unresolved`. Omit a filter for all values.
Entry placings use event/date/category/coverage filters; bout trust and round
filters apply to bouts and their statistics. Placings can exist without bouts.
Search limits are 1–100, pages 1–1,000,000, and queries at most 200 UTF-8 bytes.

Database projections encode field values as JSON strings; statistics are JSON
numbers. Missing scores and hidden participant IDs are omitted. A zero-denominator
rate is `null`. Hidden public profiles return 404; shared records say
`Private fencer` and omit names, aliases, source names, affiliations and profile
links for that participant. Public bout IDs are opaque row numbers, separate
from private source keys. Public reads never return birth years or claim evidence.
API responses carry `Cache-Control: no-store`.

For `GET /api/tracker/fencers/101?year=2026&weapon=epee`, statistics include
7 wins, 1 loss, 67 scored, 45 received and a 0.875 win rate. Without the year
filter they include 8 wins, 1 loss, 72 scored and 48 received. The source files
and hand calculations are in the fixture guide.

## Accounts and writes

Provision a local account with `--demo-account LABEL`, optionally using
`--role moderator`. The command prints a private 24-hour token. Supply it as
`Authorization: Bearer TOKEN`. The server stores its hash and supports logout.
Google/email verification is still pending. Accounts do not automatically claim
competitor identities, and choosing a fencer ID grants no management permission.

`GET /api/account` returns the caller's claims, reports and contribution receipts;
moderators also receive pending profile/privacy and result queues.

POST a JSON object to `/api/actions/ACTION`:

| Action | Required fields / permission |
| --- | --- |
| `claim` | `fencer`, `kind` (`self`/`guardian`), `evidence`, `consent`; guardian consent version is `demo-v1` |
| `review-claim` | `id`, `approved`; independent moderator review |
| `visibility` | `fencer`, `visible`; approved management grant |
| `withdraw` | `fencer`; approved manager; hides the profile and revokes their grants |
| `privacy` | `fencer`, `contact`, `evidence`; account optional |
| `review-privacy` | `id`, `approved`; moderator |
| `submit` | `operation`, `event`, `stage`, `pool`, `left`, `right`, `left_score`, `right_score`, `winner`, `evidence`; signed-in member |
| `report` | `bout`, `kind` (`score`/`identity`), `detail`; signed-in member |
| `correct` | `bout`, current `revision`, scores, `winner_side`, `status`, `reason`; moderator |
| `correct-identity` | `bout`, current `revision`, `side`, corrected `fencer`, `reason`; moderator; target must be an event entrant |
| `reject-report` | `id`, `reason`; moderator; keeps accepted facts and records the decision |
| `logout` | Empty object; current session |

Example `submit` body (an existing fixture bout, so no duplicate is created):

```json
{"operation":"demo-bout-001","event":"epee-2026","stage":"pool","pool":"A","left":101,"right":102,"left_score":5,"right_score":2,"winner":101,"evidence":"Fictional supporting record"}
```

Success is `{"status":"ok","id":1}` for operations creating a receipt; other
actions return `{"status":"ok"}`. Reuse the operation ID and identical facts
when retrying. Valid missing bouts publish as community-submitted. Conflicts
create a private review item, retain accepted scores and remove the disputed bout
from default statistics until reviewed. Corrections preserve before/after facts,
actor and reason; stale revisions fail. Changed identity/outcome context marks
event coverage partial for review. Reimports preserve corrections and suppression.

Scores may both be `null` when unknown. Submissions require an explicit winner
and completed outcome. Corrections also support `withdrawal`, `administrative`
and `unfinished`, with `winner_side=none` where applicable. Requests are limited
to 8 KiB; supporting evidence is text, with no upload endpoint. Hourly member
limits are 30 contributions, 30 reports and 10 claims; safe contribution retries
remain available after the limit. Error responses use 400 for invalid data, 403
for permission/policy/state failures, 404 for unavailable public records and 500
for internal failures.

Legacy `/api/fencers` CRUD remains available. Writes need the separate
`AUSFENCER_ADMIN_TOKEN`; birth year is optional. These legacy writes do not create
tracker profiles or competition entries. The fixture adapter currently supplies
those records. Community competition creation and participant confirmation are
unfinished; merge/split and broader identity administration are future work.
