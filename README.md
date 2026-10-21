# AusFencer

## Overview

Read my [blog post](https://duckye.com/aus-fencer/) for a less technical write-up!

See the [project plan](PROJECT_PLAN.md) for milestone status and remaining work.

AusFencer tracks fencing competition histories. The current development version
uses fictional records exclusively; no real results have been collected.

## Features

- Unicode name and alias search, profiles, bout records and event placings.
- Annual/lifetime statistics, weapon/round/trust/coverage/date/opponent filters.
- Event entrant lists, poule matrices, rankings, completed elimination rounds,
  byes and shared third places, all from canonical fictional bouts.
- Repeatable fixture imports, preview/approval, audited corrections and disputes.
- Local demo sessions, reviewed self/guardian claims, opt-out and privacy requests.

Google/email-link sign-in, community competition creation and remaining tracker
acceptance checks are unfinished. Tournament hosting and the phone referee app
remain later milestones.

## Installation

Install a C++17 compiler, CMake, Crow (including its Asio dependency), SQLite3,
OpenSSL and ICU development packages. CMake fetches the pinned sqlite_orm revision;
it uses installed GoogleTest when available, otherwise fetches the pinned revision.
The bundled SQLite sources in `libs/` are retained, but targets link SQLite3
discovered by CMake.

```sh
cmake -S . -B build
cmake --build build -j 2
ctest --test-dir build --output-on-failure
```

## Usage

Run from the repository root so the server can find `web/`. The frontend uses
plain HTML, CSS and JavaScript modules; no asset build or Node installation is
needed to run it.

```sh
./build/aus-fencer --db build/demo.db --preview fixtures/demo
./build/aus-fencer --db build/demo.db --seed fixtures/demo --approve CHECKSUM_FROM_PREVIEW
./build/aus-fencer --db build/demo.db
```

Open `http://127.0.0.1:8080/`. Startup does not insert samples. Preview prints the
new-record counts, repeated observations and checksum; seeding requires that exact
checksum. Keep generated databases under ignored `build/`.

See the [fictional dataset guide](fixtures/README.md) for fixture totals, local
account provisioning and explicit demo reset.
See the [API guide](docs/API.md) for read routes and protected workflow examples.
