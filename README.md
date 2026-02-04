# Fencing Tracker and Referee UI
Fencing tournament / session hosting and tracking service. Fencing Referee UI designed for use on mobile devices. Currently has to be self-hosted though, working on deployment in the future.

# Features
### Core Features
- Create and look up fencing bouts between clubmates to keep score!
- Host fencing tournaments in standard format (WIP)
- Mobile referee tool to easily referee dry fencing bouts (no electric) or non-scoring systems (no numbers)

# Installation
1. Clone Repository
```
git clone https://github.com/RattlePenguin/im-fencing-it.git
cd im-fencing-it
```

# Web API Usage
```
cd FencingTracker.Api
dotnet run
```

Navigate to endpoint `/scalar` to open the Scalar UI for information and testing.

Nagivate to endpoint `/index.html` to access the referee tool.
