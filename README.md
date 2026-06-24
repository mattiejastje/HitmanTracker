# HitmanTracker

A statistics tracker for Hitman games.

## Configuration

After running the tracker for the first time,
a ``HitmanTracker.ini`` file will be created.
This file contains the default configuration settings.
If it becomes corrupted, or if you simply want to restore the default settings,
you can safely delete it and a new one will be generated the next time the tracker starts.

Most settings are self-explanatory, but a few options are worth highlighting:
* ``font-size=...`` sets the overall font size (default is 20).
* ``topmost=true`` will force the tracker to be always the topmost window.
* ``hbm-real-time=true`` will use real time instead of mission time in *Hitman: Blood Money*.
* ``hma-always-track-sa=true`` will track Silent Assassin status even in missions with no targets in *Hitman: Absolution*.

## Supported Games

For now, only the steam versions are supported.

### Hitman: Codename 47

This game has no rating system, so no stats are tracked.
However, there is basic support to track map name, internal game timer,
and difficulty.

### Hitman 2: Silent Assassin

Fully supported.

### Hitman: Contracts

Fully supported.

### Hitman: Blood Money

Fully supported.

The game tracks time internally using 1024 ticks per second.
However, the final mission screen shows time assuming 1000 ticks per second.
This appears to be a bug.
The tracker will show a time that is consistent with the final mission screen
i.e. the timer will run too fast by a factor of 1.024
(or 1.44 seconds per minute).
If you prefer to use real time instead,
set ``hbm-real-time=true`` in ``HitmanTracker.ini``.
The tracker's timer will then be in sync with real time,
but obviously out of sync with the final mission screen.

### Hitman: Absolution

Fully supported.

To make rating system more transparent,
the tracker displays ratings differently from how the game shows it.
In the game, the final rating is determined in two distinct steps:

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

- Silent Assassin rating is awarded for maps with targets if:

  * No non-target kills.
  * Never spotted.

  Additionally, if you set ``hma-always-track-sa=true`` in ``HitmanTracker.ini``,
  then the tracker awards Silent Assassin even for maps that do not have targets.
  Specifically, for maps without targets, instead of showing "No Targets",
  the tracker will show "Silent Assassin" or "No Silent Assassin"
  depending on your statistics.
  This is a deviation from the game so it is not enabled by default.
  However, it makes the Silent Assassin rating consistent across all maps,
  and some players may prefer playing this way.

  Beware that
  *in a handful of missions, the game triggers a non-target kill if you do not first locate the target*
  (e.g. King of Chinatown, Vixen Club, possibly a few others).
  This appears to be a bug in the game.

- Score-based rating is awarded as follows, for rated maps:

  * Calculate score-based rating regardless of Silent Assassin.
  * Use the same percentage thresholds as the original system.
  * Show the score required for the highest achievable rating
    (e.g. 80% of the "shadow score" if Specialist is the highest achievable rating).
  * Scores for evidence removed, objectives completed, target kill, and signature kill,
    are color-coded: they show green only when they are at their maximum possible value
    so can easily tell if you are still missing objectives, evidence, or targets.

This better reflects player performance
while remaining faithful to the rating system of the original game:

* Makes Silent Assassin achievable for all checkpoints.
* Decouples skill-based rating from score-based rating for rated checkpoints.
* Provides full transparency on scoring thresholds.

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
