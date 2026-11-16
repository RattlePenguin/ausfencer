# Integration status

The integration retains the tracker, frontend, fixture CLI, account/privacy
workflows and competition statistics from main. Bout CRUD now represents separate
practice records at `/api/practice-bouts`, protected for reads and writes by the
operator token. Guest participants are explicit nulls; self-bouts are rejected.

Schema version 6 adds `practice_bouts`. The existing competition `bouts` table and
its provenance/revision relationships are preserved. Practice records do not
contribute to competition statistics. Old standalone Bout databases are disposable
and unsupported; existing tracker databases upgrade in place.

Shared changes include strict pagination parsing and route-ID bounds, atomic
repository mutations, sanitized HTTP errors, idempotent server setup, and a
build-local Crow compatibility patch. Legacy fencer privacy and optional birth
years retain main's behavior.

See [practice API](api-contract.md), [tracker API](API.md), and the main
[project plan](../PROJECT_PLAN.md). The continuation plan, project context and
implementation progress files are historical branch records.

Validation on 2026-11-16: `cmake -S . -B build`, `cmake --build build -j 4`,
and `ctest --test-dir build --output-on-failure` passed all 92 tests, including
production HTTP authentication/CRUD/restarts, version-5 upgrades, unchanged
competition histories/statistics, and explicit demo reset with practice records.
