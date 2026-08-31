# AusFencer

## Overview

Read my [blog post](https://duckye.com/aus-fencer/) for a less technical write-up!

AusFencer seeks to solve pain points found in fencing clubs across
the world, mainly revolving around scoring equipment.

## Features

- Completed bouts can be stored securely and retrieved later for viewing.
- Fencers can be searched for via name, along with all bouts they fenced in.
- ~Mobile-supported referee tool to support fencing without electrics or remote control.~
- ~~Host tournaments in standard FIE format.~~
  - ~~Designate fencers to pools based on rank and affiliation.~~
  - ~~Designate fencers to DE based on pools results.~~
  - ~~Keep track of available pistes.~~

### Current State

Currently, the app only supports HTTP requests for the Fencer class via Crow server.

- Supports CRUD for the Fencer class.

### Next Steps

- CRUD implementation for Bout class, find solution for foreign key on Fencer if not exist.
  - Not too bad, similar to what we already have for Fencer
  - I want users to still be able to key in bouts without creating a fencer, perhaps a guest fencer entry
  - Thinking of a solution for team bouts, 3 fencers on each side and we need to know the order
- Mobile remote screen for refereeing, should look something like this:
  - big timer 3 mins default
  - two big numbers 0-99 for score
  - two big plus and minus buttons for score
  - yellow card and red card buttons

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
