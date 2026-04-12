# HitmanTracker

A statistics tracker for Hitman games.

## Supported Games

For now, only the steam versions are supported.

### Hitman: Codename 47

This game has no rating system, so no stats are tracked.
However, there is basic support to track map name and internal game timer.

Known issues:

* The internal game timer is linked to the frame rate and is inaccurate.
  In particular, when playing at very high frame rates, the clock will run too fast
  (you will also experience game bugs if you do this).
  Play at 60 frames per second for best experience.

* Difficulty is not tracked.

### Hitman 2: Silent Assassin

Fully supported.

### Hitman: Contracts

Fully supported.

### Hitman: Blood Money

Fully supported.

### Hitman: Absolution

Almost fully supported.

The tracker reworks the original scoring logic from the game to make ratings more transparent.

In the base game, ratings are determined in two distinct steps:

- Condition-based playstyles.
  The game first checks for specific playstyles based on how you played.
  Examples include "Silent Assassin", "Jinx", and many more.
  The requirements for "Silent Assassin" are:

  * No non-target kills.
  * Never spotted.
  * All targets eliminated.

  Issue: missions without targets are never awarded "Silent Assassin"
  because the game never records that you have eliminated all targets.

- Score-based playstyles (fallback).
  If no condition-based playstyle is awarded, the game assigns a rating based on score:
  Agent, Veteran, Specialist, Professional, or Shadow.
  This is calculated by:

  * Taking your raw score (before modifiers).
  * Comparing it to an internal "shadow score" for that checkpoint.
  * Converting it to a percentage.

  The thresholds are:

  * 0% - 49%: Agent
  * 50% - 79%: Veteran
  * 80% - 89%: Specialist
  * 90% - 99%: Professional
  * 100%: Shadow

  Issue: Some checkpoints have a too high "shadow score".
  This can make Shadow (100%) impossible to achieve.

This tracker simplifies and improves the system:

- Silent Assassin is always evaluated independently.
  Awarded if:

  * No non-target kills.
  * Never spotted.

  Target presence no longer affects eligibility.

- Score-based ratings are always applied.

  * Agent, ..., Shadow tiers are calculated regardless of Silent Assassin.
  * Uses the same percentage thresholds as the original system.
  * Shows the score required for the highest achievable rating
    (e.g. 80% of the "shadow score" if Specialist is the highest achievable rating).

This system:
* Makes Silent Assassin achievable in all appropriate scenarios.
* Decouples skill-based rating from score-based rating.
* Provides full transparency on scoring thresholds.

This results in a system that better reflects player performance while remaining faithful to the structure of the original game.

Known issues:

* If a checkpoint is unrated,
  at the moment the tracker is unable to track the statistics,
  and awards an "Unrated" rating.
  The technical reason for this is that the tracker uses the score board,
  but the engine keeps no score board in memory for unrated checkpoints.

## Related projects

* https://github.com/nvillemin/HitmanStatistics (statistics tracker for Hitman 2: Silent Assassin and Hitman: Contracts)
* https://github.com/SuiMachine/HitmanStatisticsUber (extended version of HitmanStatistics)
* https://github.com/Attrup/StatTracker (similar to HitmanStatistics but written in rust)
* https://github.com/kurtis2221/hitmanstat (similar to HitmanStatistics, also supporting Hitman: Blood Money)
* https://github.com/OrfeasZ/Statman (statistics tracker for Hitman: Blood Money, Hitman (2016), and Hitman: World of Assassination)
* https://github.com/0danny/re47 (Hitman: Codename 47 reverse engineering project)
* https://github.com/ReGlacier/ReHitman (Hitman: Blood Money reverse engineering project)
* https://github.com/pavledev/HitmanAbsolutionSDK (Hitman: Absolution development kit)
* https://github.com/OrfeasZ/HitmanUnlocker (Hitman: Codename 47 configurable draw distance)
