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

Install Git, a C++17 compiler and build tool, CMake 3.16+, and the Boost,
SQLite3, OpenSSL and ICU development packages. Install Python 3.7+ to include
the live HTTP smoke test. Use a browser for the tracker walkthrough. The
frontend runs without Node.js or npm.

**CachyOS or Arch Linux** ([Arch package database](https://archlinux.org/packages/))

```sh
sudo pacman -Syu --needed base-devel git cmake boost sqlite openssl icu python
```

**Ubuntu or Debian**, including Ubuntu under [Windows Subsystem for Linux](https://learn.microsoft.com/windows/wsl/install/) ([Ubuntu package database](https://packages.ubuntu.com/))

```sh
sudo apt update
sudo apt install build-essential git cmake libboost-dev libsqlite3-dev libssl-dev libicu-dev python3
```

**macOS with [Homebrew](https://brew.sh/)**

```sh
xcode-select --install
brew install git cmake boost sqlite openssl@3 icu4c@79 python
```

Run `xcode-select --install` only if the Command Line Tools are missing. Before
configuring on macOS, tell CMake where Homebrew installed the libraries.

```sh
export CMAKE_PREFIX_PATH="$(brew --prefix boost):$(brew --prefix sqlite):$(brew --prefix openssl@3):$(brew --prefix icu4c@79)"
```

Run these commands from the repository root. Fetch the full Crow header tree
on the first setup. `CROW_INCLUDE_DIR` must contain `crow.h` and the `crow/`
subdirectory so the build can make its compatibility copy.
[Crow release](https://github.com/CrowCpp/Crow/releases/tag/v1.3.4)

```sh
mkdir -p build/deps
git clone --depth 1 --branch v1.3.4 https://github.com/CrowCpp/Crow.git build/deps/crow
cmake -S . -B build -DCROW_INCLUDE_DIR="$PWD/build/deps/crow/include"
cmake --build build --parallel 2
(cd build && ctest --output-on-failure)
```

If you already have Crow headers installed in a standard include directory,
you can configure with `cmake -S . -B build` on a fresh build directory. CMake
fetches the pinned sqlite_orm revision and fetches pinned GoogleTest if it
cannot find an installed copy. Allow GitHub access for the first configuration.
The build links the SQLite3 library found by CMake. Run CTest before starting
the demo server so `ProductionHttpSmoke` can bind to port 8080.

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

## Practice bouts

Separate operator-only practice CRUD is available at `/api/practice-bouts`.
Set `AUSFENCER_ADMIN_TOKEN` when starting the server and send
`Authorization: Bearer TOKEN` on every practice request, including reads.
An unset token disables access. Legacy fencer writes use the same token.
Practice bouts do not contribute to competition statistics or event results.
See the [practice API contract](docs/api-contract.md) for payloads and examples.

Schema version 6 adds `practice_bouts` without changing competition `bouts`.
Old standalone databases from `bout/crud` are unsupported; use a fresh disposable
database for that branch's demo data. Existing tracker databases upgrade in place.
Explicit demo reset also clears practice bouts before recreating fixture data.

Crow uses its Boost backend. A narrow constructor compatibility patch is applied
to a build-local header copy, never to installed headers. Python 3.7+ enables the
production HTTP smoke test, which needs port 8080 available.
