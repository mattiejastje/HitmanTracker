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

Fully supported.

The tracker reworks the original scoring logic from the game to make ratings more transparent.
In the game, ratings are determined in two distinct steps:

- Condition-based rating.
  The game first checks for specific playstyles based on how you played.
  Examples include "Silent Assassin", "Piano Man", "Jinx", and so on.
  The requirements for "Silent Assassin" are:

  * No non-target kills.
  * Never spotted.
  * All targets eliminated.

  **Issue: checkpoints without targets are never awarded "Silent Assassin".**
  This is because the game never records that you have eliminated all targets.

- Score-based rating.
  If no condition-based rating can be awarded,
  the game falls back to a rating based on score.
  This is calculated by:

  * Taking your raw score (before difficulty/challenge modifiers).
  * Dividing it by an internal "shadow score" for that checkpoint.
  * Converting it to a percentage between 0 and 100 (rounded down).

  A rating is then awarded as follows:

  * 0% - 49%: Agent
  * 50% - 79%: Veteran
  * 80% - 89%: Specialist
  * 90% - 99%: Professional
  * 100%: Shadow

  **Issue: some checkpoints have a too high "shadow score".**
  This can make Shadow (100%) impossible to achieve.

To fix these issues,
the tracker displays both rating systems independently as follows:

- Silent Assassin rating is awarded if:

  * No non-target kills.
  * Never spotted.

  Target presence no longer affects eligibility.
  This is an intentional deviation from the original rule
  so that "Silent Assassin" can be tracked also in checkpoints without targets.

- Score-based rating is awarded as follows:

  * Calculate score-based rating regardless of Silent Assassin.
  * Use the same percentage thresholds as the original system.
  * Show the score required for the highest achievable rating
    (e.g. 80% of the "shadow score" if Specialist is the highest achievable rating).
  * Scores for evidence removed, objectives completed, target kill, and signature kill,
    are color-coded: they show green only when they are at their maximum possible value
    so can easily tell if you are still missing objectives, evidence, or targets.

This better reflects player performance
while remaining faithful to the rating system of the original game:

* Makes Silent Assassin achievable for all rated checkpoints.
* Decouples skill-based rating from score-based rating.
* Provides full transparency on scoring thresholds.

Known issues:

* If a checkpoint is unrated (i.e. when no score board is shown in the game),
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
