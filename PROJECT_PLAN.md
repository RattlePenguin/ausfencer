# AusFencer project plan

Draft dated 22 October 2026, updated with the owner's decisions on dummy-data development, eventual real-data coverage, community contributions and profile visibility. Based on the original source review, the product direction discussed with the project owner, and the linked research below. Recommendations are provisional where a question remains unanswered. Sections 1–12 retain the planning baseline; delivered work and remaining checks are recorded in the implementation checkpoint below.

**Implementation checkpoint — 22 October 2026**

| Milestone | Current status | Delivered / remaining |
| --- | --- | --- |
| M0 | Complete | Versioned fictional fixture specification and JSON: 12 entrants, two poules, 41 contested épée bouts, four byes, completed tableau, shared third places, 2025 return/alias, foil/sabre examples, duplicate names, hand totals and preview/seed/reset commands |
| M1 | In progress | Versioned SQLite migrations preserve legacy records; profiles, aliases, issuer-scoped IDs, events/entries, bouts, provenance, protected writes, demo accounts, reviewed claims and visibility are implemented. Google/email-link verification and production account-age/consent onboarding remain unfinished |
| M2 | In progress | Transactional previews/approved imports, repeat/matrix/community deduplication, receipts, conflict queues, score/participant corrections, rejection and revision recovery are implemented. New community competition creation, participant confirmations and broader alias/external-ID/merge/split administration remain unfinished |
| M3 | In progress | Vanilla responsive frontend, Unicode full-name/alias search, distinguishable duplicate names, profiles, bout/event views, poule matrices/rankings/tableau/placings, annual/lifetime statistics, filters, claim/guardian/opt-out controls and moderation screens are implemented. Live browser/device acceptance and assisted guardian handover remain outstanding |

See [the fixture specification and run instructions](fixtures/README.md) and
[the API guide](docs/API.md). The server binds to loopback by default and inserts
no data at startup. Imports require an explicit checksum approval. Demo resets
require a marked demo database and explicitly reset its accounts/privacy settings;
ordinary reimports preserve suppression, grants and accepted corrections.

Known demo totals are checked from both opponents' perspectives: 101 has 7–1
and 67–45 touches in 2026 épée, then 8–1 and 72–48 across both years. The complete
2026 event conserves 41 wins, 41 losses and 489 touches scored/received, with
consistent bracket advancement. Hidden names, aliases, source-listed names and
profile links are redacted across tracker and legacy public routes, including
after reimports and changed source identifiers. Shared numeric results remain.

Validation: the C++ build and all 53 GoogleTest cases pass. JavaScript modules
pass `node --check`. AddressSanitizer/UndefinedBehaviorSanitizer also pass the
53 cases with `ASAN_OPTIONS=detect_leaks=0`; LeakSanitizer cannot run under this
environment's ptrace restriction. A listening-server/browser check could not be
completed here: socket creation returns `Operation not permitted`, and a separate
headless Firefox attempt crashes. In-process HTTP journeys are tested; visual,
mobile and real-browser behavior still need verification in a suitable environment.

Next implementation steps, following the same reviewed ≤100-line commit cycle:

1. Configure and implement Google/email-link verification and account onboarding;
   local demo sessions currently let testers exercise protected flows.
2. Add community competition creation, participant confirmation and the remaining
   identity administration flows; retain audited, reversible decisions.
3. Run actual browser/mobile journeys and fix findings, including guardian
   handover and privacy behavior. Then continue M4 acceptance/recovery checks.

M1–M3 are deliberately not marked complete. No real-result acquisition, provider
adapter, real-data permissions work, tournament hosting or referee app was started.

**1. Product outcome and priorities**

AusFencer should become a reliable record of a fencer's competitive history, starting in Australia and later supporting other countries. The primary journey is: search a person's name, identify the right fencer, explore their bouts and opponents, and understand their progress over time. Tournament pages provide context and a way to verify results.

The delivery order is:

1. Finish and validate the player tracker using fictional data: identities, competition bouts, repeatable fixture imports, name search, profiles, statistics, contributions and privacy controls.
2. Once the app works end to end, establish real-data access and permissions, validate a source adapter against an actual sample, and begin Australian coverage.
3. Add tournament hosting, beginning with one supported individual competition format and a club pilot.
4. Add the phone referee app for practice, reusing the tracker and bout model.
5. Expand international coverage and selected stretch features as data access and demand justify them.

Both tournament hosting and the referee app depend on a completed tracker. Their relative order after that is an open decision; the sequence above follows the order in the project brief. International identifiers and names must work from the beginning, even though eventual initial data collection is Australian. The current priority is a working app with dummy data. Source acquisition, provider agreements, legal documentation for real-data use and historical backfill are deferred until that working-app milestone is accepted.

**2. What exists today**

This table records the original planning baseline; use the checkpoint above for
the implementation's current state.

| Area | Repository evidence | Work still required |
| --- | --- | --- |
| Server | C++17, Crow, SQLite, sqlite_orm; handlers and repositories are separated | Configuration, deployment, operational checks, protected write access |
| Fencers | CRUD at `/api/fencers`; first name, last name, required birth year | Identity resolution, aliases, optional personal fields, affiliation history |
| Search | Substring matching against either first or last name; pagination | Full-name queries, name order, Unicode-aware matching, stable ordering and bounded page sizes |
| Bouts | `include/models/Bout.hpp` has fencer IDs, weapon, scores, time, yellow/red cards | Database mapping, foreign keys, repositories, routes, date, context, outcome and provenance |
| Competitions | No competition schema or routes | Tournament, event, entry and round records |
| Tests | Fencer repository/handler tests and basic database tests | Import, identity, bout, statistics and later tournament behavior |
| Interface | No frontend | A responsive website for finding people and exploring results |

The README's feature bullets express intent rather than verified completion. Tournament hosting is crossed out there but is restored as a future objective by this brief. The root `notes` file suggests storing names directly in bouts; use stable fencer references for canonical records, while retaining original source names as import evidence. Replace the current sample-fencer insertion on every startup with an explicit, repeatable development seed/reset command.

This was a source review. Existing tests were inspected, not executed, and no assessment of production readiness is implied.

**3. First release boundary**

Build the first working version entirely with fictional fencers, events and results. Use the organisation of the [AFF results index](https://www.ausfencing.org/results/) and the [Sydney individual épée example](https://www.ausfencing.org/competitions/open-mens-epee-individual-sydney-7/) as structural references: browsing competitions by season/year, an entrant list with names and affiliation, poule score matrices with V/M, indicator and hits scored, post-poule rankings, a direct-elimination tableau and final placings. Create new names, event titles, identifiers and scores. Label the development dataset and its views as dummy/demo data, with synthetic provenance.

Start with a small, complete individual épée fixture, such as 12 fictional entrants in two poules of six feeding a tableau of 16 with byes. Include victories below the usual target score, shared third places and consistent advancement. Each poule matrix must describe the same bouts from both perspectives; its totals, rankings, tableau and final placings must agree with the canonical records. Store the completed bracket as fixture data; automated tournament allocation and progression remain part of the later hosting milestone. Add a second event in another calendar year with returning competitors, an alias and duplicate names, plus smaller foil and sabre fixtures. Keep deliberately incomplete, disputed or invalid cases separate from the complete example. Record expected statistics calculated by hand and provide a deterministic seed/reset workflow.

After the dummy-data app passes its completion checks, start real coverage with AFF competitions, working backwards from the most recent completed 2026 events, then through 2025 and earlier years. Inspect one actual event and its permitted source format at that stage before expanding. Refresh recent events for corrections while historical backfill progresses, and show incomplete coverage explicitly. Support individual foil, épée and sabre results with the same schema. Keep team results and practice records out of individual competition totals until their semantics are implemented.

The working dummy-data app should let a tester complete the following journeys using fictional competition records; these remain the goals for the later real-data release:

- Search by full or partial name, including aliases, and distinguish people using available club, state/country and event context.
- Open a stable profile showing recorded bouts, opponents, event results, lifetime and annual statistics.
- Filter by weapon, year/date range, competition category and pool versus direct elimination.
- Open a bout or event and see the underlying result, source, update time and any uncertainty.
- Report a wrong score, missing bout or incorrect identity association, then track the review outcome.
- Sign in, contribute a bout or competition, and see their contribution and its confirmation status.
- Claim their profile and control whether it is publicly searchable; request privacy changes without having to create an account.

Confirmed direction: competition profiles are publicly searchable unless the fencer opts out, and signed-in community members can contribute results and competitions. The first release is an unofficial community tracker; federation licences and in-person judge verification are not prerequisites for contributing. Use Google sign-in as the initial account option, with email-link access as a fallback. Accounts represent contributors and organisers, while fencer records represent competitors. An unclaimed public record can be searchable without its fencer having an account, subject to privacy controls and the proposed junior handling below.

**Community contributions and trust.** Let any signed-in member submit their own results, a missing bout or a competition. Ask for the event, date, participants, score/outcome and an optional supporting record they have permission to share. Validate structure, likely duplicates and plausible event context, then publish valid, undisputed submissions with a visible **community-submitted** label. Keep unresolved identities separate. An optional **participant-confirmed** label records independent acknowledgement from both linked participants; **organiser-supplied** identifies provenance, not federation endorsement. These are separate signals rather than a vote-based claim of truth. Google authentication alone establishes neither fencer identity nor result accuracy.

Existing accepted scores and identity links cannot be overwritten by another user's submission. Conflicts and reports enter a moderation queue with evidence and an audit trail; disputed records remain visible as disputed but leave default statistics until resolved. Show all included trust categories in statistics filters, with accepted, undisputed community records included by default. Low-risk contributions should not require manual approval. Rate limits, upload limits, duplicate detection, permissions, reporting and reversible revisions constrain abuse. A contributor cannot edit someone else's privacy settings or claim their identity merely by choosing their name.

Make participation enjoyable through a personal contribution history, acknowledgements and an optional recap of gaps filled. Avoid rewarding raw submission counts or turning popularity into verification. More elaborate badges can follow once abuse patterns are understood.

**Accounts, claims and profile visibility.** Keep authentication identities, fencer profiles and profile-management permissions separate. Match Google accounts using the provider's stable subject identifier, validate sign-in tokens on the server, and request only basic sign-in scopes. Do not automatically attach a Google name or photo to a competitive profile. Profile claims need separate evidence, with manual review for ambiguous or conflicting requests. [Google OpenID Connect documentation](https://developers.google.com/identity/openid-connect/openid-connect).

Provide one clear “Show my profile publicly” control plus a privacy-request route for unregistered fencers. Hiding a profile must remove its public profile page, name-search results and identifying links/names in opponent histories, event views, public APIs and exports; use a neutral “Private fencer” label where a shared bout remains visible. Preserve opponents' numeric results where appropriate, explain the remaining context and its possible identifying effect, and review stronger suppression requests individually. Apply the visibility setting on the server and invalidate public caches. Search-engine removal can lag; a no-index tag alone is not a privacy control. Keep an internal suppression record so a later import or alias does not recreate a hidden public identity. Explain profile hiding, retained result records and deletion separately, and manually review disputed claims without making an account compulsory for a privacy request.

**Simple proposed junior flow.** Juniors can participate; there is no blanket rule that all under-18s must use a guardian. OAIC guidance says the Privacy Act sets no fixed age for privacy decisions. Where individual assessment is impractical, its APP guidance permits a presumption of capacity from 15, absent contrary indications, and presumes under-15s lack capacity. Use **15+ self-service signup and under-15 guardian-managed profiles** as a simple initial product policy, with an assisted route where capacity or authority is unclear. This is a recommendation based on current guidance, not a universal legal signup threshold. [OAIC APP guidance, B.59–B.61](https://www.oaic.gov.au/privacy/australian-privacy-principles/australian-privacy-principles-guidelines/chapter-b-key-concepts).

For younger fencers, an adult signs in to their own account, chooses “Manage my child's profile”, confirms their relationship and authority, and approves a short, plain-language explanation of collection and public visibility. Record the adult account, declaration, consent version/date and managed profile. The child needs no email or separate login. Guardian claims can use the same manual review as adult profile claims; an email confirmation or checkbox is a declaration, not independent proof of guardianship. Allow withdrawal of permission, hide the profile promptly on a credible request, and provide a reviewed handover when the young person becomes eligible to self-manage. Avoid routinely collecting identity documents.

For known under-15s, recommend holding public profile aggregation until a guardian approves it; then support the same opt-out control. An existing age-category result can remain an unresolved event record while publication is settled. Do not publish a junior's account email, full date of birth, guardian details or private notes. Ask for only the age information needed for the account flow, keep it private, and do not treat a fencing category or Google login as proof of age. Public result availability and permission to assemble a searchable personal history are distinct questions.

Google generally allows independently managed accounts from 13 in Australia, and Family Link controls can affect third-party access. That Google threshold is separate from AusFencer's proposed privacy-consent policy; guardian-owned accounts avoid making a younger child's Google account a prerequisite. [Google account age requirements](https://support.google.com/accounts/answer/1350409?hl=en), [Family Link third-party access](https://support.google.com/families/answer/9204736?hl=en).

Before public launch, check applicable privacy obligations and the final Children's Online Privacy Code; OAIC currently says it must be finalised and registered by 10 December 2026. Treat draft provisions as draft. Australia's under-16 social-media account restriction is not a universal website-account ban. Assess AusFencer against eSafety's criteria using its actual features, and reassess if social feeds, comments, messaging or other interaction become significant. Guardian permission would not substitute for an applicable social-media age restriction. [OAIC code status](https://www.oaic.gov.au/privacy/privacy-registers/privacy-codes/childrens-online-privacy-code), [eSafety service assessment](https://www.esafety.gov.au/about-us/industry-regulation/social-media-age-restrictions/assessment).

**4. Data model and identity**

| Record | Purpose and relationships |
| --- | --- |
| Fencer | Stable internal ID, display name and searchable aliases. Birth year is optional and need not be public. Preserve original spelling and support names beyond a required first/last split. |
| External identity | Map a fencer to an issuing organisation and external identifier, such as a federation membership number or provider competitor ID. Scope IDs to their issuer; they are optional and are never the internal primary key. Record confidence and provenance. |
| Club / affiliation | Club, state association and country context, with event-time snapshots or dated memberships. Changing clubs must not rewrite history. Nationality and club location are separate concepts. |
| Tournament | A meeting with name, location, country, local dates and timezone. |
| Event | A competition within that meeting: weapon, category, age group, individual/team type, date, status and eventual ruleset version. |
| Event entry | Links a fencer to an event, with source-listed name, affiliation, seed and final placing when known. |
| Round / pool | Locates bouts within pool rounds and elimination tables. Historical imports may have incomplete round information. |
| Bout | Two participant references for an individual bout, event/round when known, scores, explicit outcome/winner, result status, date precision and optional duration/cards. |
| Source / import run / observation | Original identifiers, source location or supplied file, acquisition time, checksum, parser version, permitted-use basis, extracted facts, validation results and canonical links. Several sources may describe one bout. |
| Correction / revision | Reporter, evidence, reviewer, previous and new values, reason and timestamps. Supports reversal and accountability. |
| Account / sign-in identity | An internal user account linked to provider and subject IDs, with roles and account-age/consent state kept private. Supports contributors who are not competitors. |
| Profile claim / management grant | Links an account to a fencer with an approved self/guardian role, supporting evidence, visibility settings and any consent/withdrawal history. |
| Contribution / confirmation | Submitted facts, actor, source evidence, validation and dispute state, independent participant acknowledgements, and links to published records. |

Retain the two foreign keys for individual bout participants; a general participant join table is unnecessary until team encounters require it. Event entries provide the fencer-to-event relationship. Later, model team encounters and their component individual bouts explicitly so neither is counted twice.

Identity matching proceeds from strong external identifiers to reviewed contextual matches. Normalised names, aliases, club, event history and optional birth year can suggest a match. Name equality alone must not automatically merge people. A source competitor can remain unresolved without attaching their results to the wrong profile. Provide merge and split tools that preserve source observations and redirects from merged profile IDs.

Enforce foreign keys, distinct opponents and non-negative known scores. Validate scores against the applicable format rather than imposing a universal winning score. Keep unknown values nullable: unknown score, time or card count is different from zero. Record byes, withdrawals, exclusions and walkovers distinctly. A recorded winner can be necessary when scores alone do not explain the decision.

Use indexes for both participant references, event/date filters and external IDs. Use explicit, versioned migrations and restrict deletion of referenced fencers; the existing delete behavior must change once bouts depend on them.

**5. Real-data integration after the app works**

This section is deferred work after M4. Do not make obtaining real records, finding legal documentation for their use, provider outreach or a historical backfill list a prerequisite for building or accepting the dummy-data app. For M0–M4, use a documented, provider-neutral fixture format and a local fixture adapter to exercise validation, identity resolution, previews, publication, corrections and repeat imports. The AFF pages in section 3 guide the fixture structure; they are not an ingestion dependency. The source-access research below is retained for the later real-data phase and must be rechecked then.

The Fencing Time Live homepage currently redirects to a login page. Its published terms restrict automated extraction, redistribution, aggregation and competitive use, and describe authorised API access under a separate written agreement. Consequently, an unrestricted scraper is not an established dependency we can promise. Plan for an authorised integration and obtain clarity on storage, derived statistics, public display and use alongside future tournament hosting. These are source-access findings, not a conclusion about the enforceability of the terms. [Login page](https://www.fencingtimelive.com/account/login), [published terms, sections 3–4](https://www.fencingtimelive.com/terms).

When real-data work begins, assess three acquisition routes independently:

1. **Organiser-supplied records:** request example exports or original competition records with permission for the intended use. Fencing Time documents per-event XML result exports and report exports, but a sample is needed to establish whether a particular file includes bout scores or only final placings. An export capability does not itself establish redistribution rights. [Fencing Time user's guide, chapters 20–21](https://fencingtime.s3.amazonaws.com/FencingTimeUsersGuide.pdf).
2. **Provider integration:** investigate an agreement with Fencing Time covering the intended Australian archive, refreshes, display and later product scope. Prefer a documented feed if available; API availability and pricing remain unverified. No outreach is part of this planning task.
3. **Community and other independent sources:** accept participant contributions and assess original federation/state or club records with their own provenance and permitted-use basis. Prioritise AFF events from the most recent 2026 results backwards, then 2025 and earlier. The AFF results page is a discovery starting point, not proof that complete bout-level data is available or reusable. User submission is not a workaround for a provider's extraction restrictions. [AFF results](https://www.ausfencing.org/results/).

If scraping is authorised, the connector should discover an agreed set of tournament/event IDs, retrieve permitted competitor, pool, elimination and results pages, and map them into a provider-neutral import format. Use an agreed request budget, caching where permitted, incremental refreshes, retry backoff, resumable jobs and parser versioning. Stop on access failures or unexpected layouts and flag the import for review. Inspect actual permitted pages before selecting HTTP parsing or browser automation. Avoid designing around assumed endpoints.

All routes use the same pipeline:

`acquire → preserve permitted source evidence → parse → validate → resolve identities → preview changes → publish with trust status → refresh statistics`

For bulk imports, require an operator's preview and approval. Valid, undisputed individual submissions can publish automatically as community-submitted; route ambiguous matches, conflicting results and flagged records to review. Track historical coverage by event/year and separate new-result refresh jobs from backwards historical backfill.

Deduplicate using source-scoped event/bout IDs and canonical mappings. Where no bout ID exists, include event, round/pool and bout position in a reviewed matching key; the same two people may fence repeatedly. Pool matrices often express a bout from both competitors' perspectives and must produce one canonical bout. Reimporting a file must be idempotent. Conflicting observations go to review; corrected upstream data must not silently overwrite an approved local correction. Apply each approved event import transactionally and retain a recovery path.

Publish coverage explicitly: event results only, partial bouts, complete bouts, or unresolved participants. Date a result by its competition date, with known precision, rather than its import time. Never reconstruct individual bouts from final placings.

**6. Statistics that mean what users expect**

Calculate statistics from canonical, accepted, undisputed results, including community-submitted results by default with clear trust filters, so imports and corrections change both opponents' views consistently. During dummy-data development, verify the same policy against known fixture totals and label the figures as demo statistics. Once real coverage begins, label lifetime figures as recorded career totals and show coverage; the eventual 2026-first backfill cannot initially establish a complete career record. Being listed as an AFF event does not make a user-entered score officially verified.

| Statistic | Proposed definition |
| --- | --- |
| Wins / losses | Accepted, completed, actually contested bouts with an explicit outcome. Administrative advancements and unfinished bouts are shown separately. |
| Win rate | Wins divided by wins plus losses; display the denominator and use “—” when it is zero. |
| Touches scored / received | Sum known final scoreboard scores from the fencer's perspective for the eligible bout set. Disclose that penalty-awarded points can be included. |
| Touch differential | Scored minus received over the same bouts. |
| Touch share | Scored divided by scored plus received; show “—” for a zero denominator and the score-known bout count. |
| Annual change | Calendar year using the event's local date; compare equivalent periods when one year is incomplete. Show change in win rate in percentage points. |
| Head-to-head | The same definitions restricted to one opponent, with weapon and pool/DE filters. |
| Events / placings | Distinct event participation and recorded final places, even where bout-level coverage is unavailable. |

Expose pool and DE splits because five-touch and longer bouts have different score distributions. Do not infer a competition winner solely from the larger score, especially for exceptional outcomes. Exclude undated bouts from annual charts and disclose their count. Separate unknown-score bouts from the touch denominator. Keep official/imported competition results, reviewed self-reports and practice data distinguishable; the default view should make its included sources clear.

Start with database queries and one shared statistics policy. Add cached summaries only after measuring a need, with invalidation on corrections, identity merges and import revisions. Future ratings require chronological replay when historical results change; ordinary win/loss totals should not depend on a rating system.

**7. Delivery milestones and completion checks**

| Milestone | Deliverable | Completion check |
| --- | --- | --- |
| M0 — define the dummy-data pilot | Specify fictional AFF-style entrants, poules, rankings, a completed tableau and final placings; define the fixture format, seed/reset workflow and expected statistics | A complete, internally consistent fixture specification with stable IDs and hand-calculated totals; no real records or source permissions required |
| M1 — persist the domain | Migrations, identities, tournaments/events/entries, bouts, provenance, Google/email-link accounts, profile claims, visibility, protected writes and validation | A seeded event is retrievable from either opponent's profile; unauthorised changes fail; privacy settings are enforced; upgrades preserve records |
| M2 — exercise result workflows | Fictional community submissions, a local fixture adapter, bulk previews, repeatable imports, trust labels and correction/identity workflow | A tester can submit a dummy result; repeats create no duplicates; conflicts cannot overwrite accepted records; a correction and a failed import are recoverable |
| M3 — deliver the tracker | Responsive name search, profiles, bout history, event results with poules/tableau/placings, annual/lifetime stats, trust/coverage filters, opt-out and guardian management, all using dummy competition data | Fixture statistics match hand calculations; duplicate names are distinguishable; event views agree with bout histories; a hidden profile stays hidden across public views and reimports; guardian permissions and withdrawal work |
| M4 — accept the working app | End-to-end review with testers using fictional results; backups/restore, demo deployment, monitoring, contributor permissions and a documented fixture dataset | Testers complete search → profile → bout/event → statistics, contributions, corrections and privacy journeys; fixture identities and totals are correct; recovery works; the app is accepted as working before real-data acquisition begins |
| M5 — introduce real results | Inspect one completed AFF event, establish a viable acquisition route and permissions, validate a real source adapter and coverage, and plan newest-first backfill | An approved sample imports repeatably with reviewed identities and correct totals; actual source limitations are documented; real-data launch checks, including privacy and junior policy, are completed; dummy records stay out of real profiles and statistics |
| M6 — host a club tournament | Implement one specified format, organiser/referee roles, scoring, progression, piste assignments, spectator results and exports | Replay known examples and complete a supervised club event with a recovery/export fallback |
| M7 — phone referee app | Practice scoring, timer, cards, undo, local recovery and explicit submission to the tracker | Finish and recover bouts on real phones; reconnect/retry creates exactly one saved bout; practice data stays separate |
| M8 — expand | Further tournament formats, countries/providers and validated stretch features | Each addition has a data/rules owner, supported examples and its own acceptance checks |

M0–M4 deliver and validate the working player tracker with dummy data. M5 introduces real results only after that acceptance gate; source access, permissions research and backfill planning start there. If historical permission is unavailable then, narrow real coverage to independently supplied records and state the limits. M6 and M7 may be reordered after the tracker milestones. Do not assign calendar deadlines until the fixture scope, team capacity and first interface are understood. Keep demo records isolated from any later real-data environment.

Keep the existing C++ API and repository structure. Add domain/statistics/import services where business logic needs to be shared; HTTP handlers should translate requests and repositories should handle persistence. Import jobs should run outside interactive request handling, through controlled application interfaces rather than unmanaged writes to a live SQLite file. Keep SQLite initially and assess a database change only against measured write concurrency and operational requirements. Choose the frontend stack when M3 is scoped.

**Implementation workflow (all milestones)**

Break each milestone into small steps, each covering one reasonable implementation chunk. For example, implementing the Bout handler's create function is one step; its read, update and delete functions are separate steps. Include the supporting declarations and relevant tests within the same chunk where practical.

For each step, complete this cycle before starting the next:

1. **Implement the step.** Make the scoped change and run the relevant checks.
2. **Review the just-completed work.** Inspect the diff for correctness, edge cases, test coverage, repository conventions and scope.
3. **Implement fixes if necessary.** Address review findings, rerun affected checks and review the fixes. Repeat until findings are resolved.
4. **Commit the completed chunk to git.** Commit only after implementation, review and any fixes are complete and required checks pass. Use a short, concrete commit message.

Each commit has a hard budget of **100 changed lines**, counting additions plus deletions across implementation, tests, documentation and any other included files. Inspect the staged diff with `git diff --cached` and verify its size with `git diff --cached --numstat` before committing. If a step would exceed the budget, split it into smaller, coherent steps and apply the full cycle to each; each commit must leave the project buildable and its tests passing. Keep unrelated changes out of the commit.

Implementation checks should cover duplicate names and aliases; missing IDs and fields; search/pagination boundaries; participant foreign keys; repeated imports; duplicate matrix observations; conflicting revisions; score orientation; pool/DE and year boundaries; exceptional outcomes; and known totals calculated by hand. Include sign-in validation, claim permissions, community submission/dispute states, trust filters, guardian grants/withdrawal, privacy requests and hidden identities across APIs, exports, caches and reimports. Use isolated SQLite fixtures and register new test files in `tests/CMakeLists.txt`. Run the full repository suite before submitting implementation changes. Add browser checks for the actual name-to-profile journey when a frontend exists.

**8. Tournament hosting: learn the format before automating it**

Build around an event lifecycle: entries and check-in → initial seeding → pool allocation and bout order → pool scores and ranking → qualification and DE seeding → elimination bouts → final results and publication. This is the proposed first supported workflow, subject to the pilot organiser's rules. The FIE publishes separate organisation and technical rules, and AFF documents Australian overrides and competition management responsibilities. Keep the relevant editions attached to the event instead of relying on an unversioned “standard FIE” flag. [FIE rules](https://fie.org/documents/rules), [AFF rules](https://www.ausfencing.org/rules/), [AFF competition management](https://www.ausfencing.org/competition-management/).

Work through one real tournament with an experienced organiser and record decisions on pool sizes, seeding sources, affiliation separation, victory ratios, indicators, ties, promotion cuts, bracket placement, byes, withdrawal and exclusion handling, final-place ties and score/time limits. Define expected outputs before implementing each rule. Historical imports must preserve what happened under the rules then in force.

Start with a club individual event using an agreed pool-plus-DE format. Add organiser previews and explicit approval before each progression. Changes to earlier scores must identify downstream seeds and bouts affected; do not silently regenerate a bracket after later bouts have started. Record overrides and actor/revision information, and prevent two officials from unknowingly overwriting each other.

The hosting pilot needs entry import/check-in, printable pool sheets and brackets, referee/piste assignment, resource conflict checks across concurrent events, live read-only results, manual corrections, recoverable saves and useful exports. Choose a local venue operating and cloud publication strategy before the first live event; internet loss must have a rehearsed operating procedure. Validate with replay, then a small supervised tournament. Federation submission and sanctioning compatibility need separate verification before claiming national-event readiness. Payments, complex international exemptions, teams and alternative formats can follow later.

**9. Referee app after tracker completion**

Start with a mobile web app/PWA unless device testing shows a need for native apps. Give the referee large score controls, start/pause/reset, period/rest handling, priority where relevant, cards, clear left/right identity and undo. Make scoring and timing configurable for the supported practice formats. Screen changes, reloads and backgrounding need device testing and recoverable state.

Keep a bout on the device until explicitly saved, support guest opponents and optional later identity matching, and make publication deliberate. Sync with an idempotent client-generated operation ID. Record corrections and, where useful, a timestamped action log. A phone clock and manually entered touches support a referee; automated hit detection and direct scoreboard hardware control would be separate integrations. Practice results remain outside competition statistics by default. Reuse the same validation and statistics policies when practice views are added.

**10. Research-backed opportunities and stretch goals**

Community threads are anecdotal and largely international. They identify problems to validate with Australian fencers, rather than establishing how common each problem is or whether a current product still has it.

| Observed need and evidence | AusFencer opportunity | Priority |
| --- | --- | --- |
| Abbreviated names and incorrect membership associations can detach bouts from a profile. [Fencer report and follow-up discussion](https://www.reddit.com/r/Fencing/comments/1p1gmrf/my_fencingtracker_page_is_not_showing_my_bout/) | Aliases, reviewed matching, merge/split tools and visible source names | Tracker requirement |
| Users report missing events and unclear correction progress. [Missing-results discussion](https://www.reddit.com/r/Fencing/comments/1tpd5la/fencing_tracker_error_and_response_time/) | Coverage and freshness indicators, import status, correction tracking and a clear review owner | Tracker requirement |
| Fencers describe difficulty receiving timely piste assignments and practical connectivity limits. [Piste-assignment discussion](https://www.reddit.com/r/Fencing/comments/1b2c8yn/) | A personal “where am I next?” view with optional alerts, update timestamps and an on-site fallback | After hosting works |
| A club organiser values live sharing while keeping in-house events off a global public listing. [Club live-results feedback](https://www.reddit.com/r/Fencing/comments/1ug1336/smart_fencing_poolsheets_now_with_an_ultrasimple/) | Unlisted club events, expiring read-only links and spectator views | Hosting/referee extension |

FencingTracker currently advertises name/USFA-number search, profiles, bout histories, strength trends and reviewed data corrections. Basic analytics and correction reporting therefore provide useful baseline behavior; they are not by themselves unique differentiators. Its USFA search does not establish that a US commercial data licence is available to AusFencer. [FencingTracker](https://fencingtracker.com/).

Further ideas below are product suggestions, not findings of demonstrated demand:

- **Season recap:** shareable annual cards with recorded wins, touch differential, new opponents and personal milestones, showing coverage and respecting profile visibility.
- **Rivalry timeline:** head-to-head history showing how margins change over time; keep commentary factual and let people control sharing.
- **Personal progress goals:** private targets such as attending events or improving a pool win rate, avoiding incentives to inflate bout counts.
- **Fencing passport:** an opt-in record of clubs, states and countries fenced in, derived only from available event locations.
- **Private training journal and video links:** annotate bouts with lessons and timestamps; tactical conclusions need information beyond a final score.
- **Coach view:** permission-based season summaries across several athletes and event-day scheduling conflicts.
- **Experimental strength ratings:** separate by weapon and relevant bout format, publish method and uncertainty, validate on historical holdouts, and label them as unofficial. Defer until identities and coverage are dependable.
- **Portable data:** personal exports and, if source agreements permit, a documented API so people can build their own analyses.

Prioritise trust and coverage first, then season recaps and rivalry timelines, then live-event convenience. Validate proposed additions with a small Australian group including fencers, a junior parent/guardian, a coach and a tournament organiser before committing substantial work.

**11. Questions and decisions to resolve**

| Question | Why it matters | Provisional recommendation |
| --- | --- | --- |
| After M4, can we obtain a representative recent AFF 2026 source file or original record? | Deferred to M5; actual bout fields and access do not block the dummy-data app | Inspect one completed event when real-data work begins, before expanding through 2026, 2025 and earlier |
| For the later real-data phase, does FencingTracker's “USA specific license” mean USFA membership IDs, a data licence, or both? Is there a dependable Australian identifier available? | Identity design and acquisition agreements are different problems; real identifier availability is deferred | Internal IDs plus optional issuer-scoped external IDs; use fictional external IDs in fixtures |
| Who will handle ambiguous identity claims, disputed results and privacy requests? | Community contribution is decided, but exceptions still need a review owner | Automatic publication for valid, undisputed submissions; manual review of exceptions |
| Is the proposed 15+ self-service / under-15 guardian-managed policy suitable after checking launch-time rules? | Public search with an opt-out is decided; junior consent needs a simple implementation | Guardian's own account, no child email, plain-language approval and revocable publication; recheck final guidance before launch |
| Is the first release individual competition results only? Do unsanctioned events count in default statistics? | Defines trust labels and whether a single total would be misleading | Individual competition data first; clearly label source and event type |
| Should annual comparisons use calendar years or federation seasons? | Determines date filters and comparison semantics | Calendar years first, season support later if needed |
| Which organiser and format should the hosting pilot support? Must it operate without internet? | Defines rules, reliability and venue architecture | One club event; agree on a recoverable venue workflow before implementation |
| After the tracker, should tournament hosting or the practice referee app come next? Does “remote” imply a second device/display? | Affects sequencing and device scope | Hosting then practice app; single-phone scoring initially |
| Is the service free/community-funded, subscription-based, or club-funded, and who will operate it? | Affects hosting budget, source agreements and review capacity | Leave pricing open; establish an owner for imports, corrections and backups |

Confirmed owner decisions: finish the app with dummy data first, using the supplied AFF pages as structural references; defer real-data collection and documentation for its use until the app is working. Eventual real coverage starts with AFF competitions, newest completed 2026 events backwards through 2025 and earlier. Broad community contributions to an unofficial tracker, publicly searchable profiles with an opt-out, and account sign-in including Google remain the intended behavior. The owner permits a simple guardian-supervised approach if needed. The specific age split, junior publication flow, confirmation labels and moderation mechanics above are recommendations, not additional decisions already made by the owner.

**12. First implementation slice**

Start with two fictional fencers, a fictional event and one bout: persist them; retrieve the bout from either profile; calculate each perspective's totals; attach synthetic provenance; import the same fixture observation twice without duplicating it; and correct the score with an audit record. Grow this into the complete AFF-style dummy event specified in section 3, including entrant lists, poule matrices, rankings, a completed tableau and final placings. Follow with two competitors who share a name and a second event in another year that uses an alias. Then let a signed-in tester submit a fictional missing result, exercise the dispute flow, and hide a claimed dummy profile across all public views. Complete and verify the app journeys through M4 before obtaining real event records or implementing a real provider adapter.
