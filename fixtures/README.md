# Fictional tracker pilot (M0)

All names, clubs, identifiers, competitions and scores are invented demo data.
No provider records are included. Keep this dataset in a separate demo database.
The versioned JSON files in `demo/` are the local fixture adapter's input.
Records use stable string source IDs, scoped to `ausfencer-demo-v1`.
Imports must preview changes, require explicit approval and commit atomically.
Reimporting must preserve canonical IDs, corrections and profile suppression.
Reset deletes only a database explicitly marked as this demo dataset; it must
never erase an arbitrary database. Server startup never inserts sample records.

From the repository root, build the application and run:

```sh
./build/aus-fencer --db build/demo.db --preview fixtures/demo
./build/aus-fencer --db build/demo.db --seed fixtures/demo --approve CHECKSUM_FROM_PREVIEW
./build/aus-fencer --db build/demo.db --demo-account Tester
./build/aus-fencer --db build/demo.db --demo-account Reviewer --role moderator
./build/aus-fencer --db build/demo.db
```

Account creation prints a private demo session token, valid for 24 hours. Use
it as `Authorization: Bearer TOKEN`; accounts and competitor claims stay separate.
The demo account command refuses an unseeded/unmarked database. It provides local
test identities; Google and email-link verification are not implemented yet.
Legacy fencer writes require the separate `AUSFENCER_ADMIN_TOKEN` environment
variable and are disabled when it is absent. Public reads never return birth years.
To start the fictional dataset over, stop the server and replace `--seed` above
with `--reset-demo`. This explicitly resets results, accounts, claims and privacy
settings in the marked demo database, while retaining import outcome history.
Ordinary `--seed` repeats preserve privacy settings and accepted corrections.

## Complete 2026 individual épée event

Twelve entrants (101–112) fence in poules A (101–106) and B (107–112).
Each unordered pair fences once: 30 poule bouts. The lower ID wins 5–2,
except 101 beats 106 by 4–1 at time. Each matrix cell is a perspective on
one canonical bout, never a second result. All bouts have explicit winners.

The supplied post-poule seed order is:
107, 101, 102, 108, 103, 109, 104, 110, 105, 111, 106, 112.
Rank by victories/matches, indicator, hits scored, then the supplied tied seed.
107 has 5/5, +15, 25 scored; 101 has 5/5, +15, 24 scored.
102/108 have 4/5, +9, 22; 103/109 have 3/5, +3, 19;
104/110 have 2/5, −3, 16; 105/111 have 1/5, −9, 13;
112 has 0/5, −15, 10; 106 has 0/5, −15, 9.

The first four seeds have byes in a tableau of 16 (no contested bout).
R16: 103–112 15–8, 109–106 15–9, 104–111 15–10, 110–105 15–11.
QF: 107–110 15–9, 108–103 13–12 (time), 101–104 15–7, 102–109 15–13.
SF: 107–108 15–12, 101–102 15–14. Final: 107–101 15–13.
There is no bronze bout. Final places, in ID order 101–112:
2, 3, 5, 7, 9, 11, 1, 3, 6, 8, 10, 12.
There are 41 contested bouts, 41 wins and 41 losses; byes add none.

Hand calculations for 101 in 2026: poule 5–0, touches 24–9;
DE 2–1, touches (15+15+13)–(7+14+15) = 43–36;
total 7–1, 67–45, +22, win rate 7/8, touch share 67/112.
107: poule 5–0, 25–10; DE 3–0, 45–34; total 8–0, 70–44, +26.

## Additional fixtures and edge cases

2025 épée: returning 101 beats 107 by 5–3 under an alias; final places 1/2.
Lifetime 101: 8–1, 72–48, +24. Lifetime 107: 8–1, 73–49, +24.
Separate foil and sabre events each contain one final, using returning entrants.
101 and 108 share a display name but have different fictional clubs/states.
An alias and issuer-scoped fictional external ID resolve to 101, not 108.
Birth years are optional and never part of a public identity response.

Invalid, incomplete, disputed, unknown-score and administrative results belong
in isolated tests, not in the complete event. Tests must exercise score
orientation, repeats, mirrored observations, conflict review and rollback.
Statistics include accepted, undisputed, contested competition bouts by default;
trust, weapon, calendar year, category and poule/DE filters use the same policy.
Public views must redact hidden names and identifying links, including aliases
and original source names. Numeric opponent results can remain visible.

## Delivery order

Persist the event and bout model, then implement the fixture adapter and its
preview/approval commands. Add corrections and audited identity review before
the responsive search → profile → event journey. Account/claim and guardian
permissions must be verified before exposing any profile-management controls.
