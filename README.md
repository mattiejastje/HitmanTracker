# HitmanTracker

A statistics tracker for Hitman games.

Note only Steam is supported.

* Download: https://github.com/mattiejastje/HitmanTracker/releases
* Report issues: https://github.com/mattiejastje/HitmanTracker/issues

## Usage

Simple extract the zip file anywhere you like, and run the executable.
The tracker will automatically detect whichever Hitman game you are running.

Windows Defender may give a warning for (good!) security reasons:
to function, the tracker must directly access the game's memory and it is therefore flagged.
Unfortunately there appears no way around this.

All settings are stored in the ``HitmanTracker.ini`` file.
If, for whatever reason, it becomes corrupted,
or you simply want to restore the default settings,
you can safely delete it and a new one will be generated the next time the tracker starts.

## Development

Issues and pull requests are most welcome.
All development takes place at https://github.com/mattiejastje/HitmanTracker

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
In the settings, there is a toggle to use real time instead.
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

  Additionally, there's an option in the settings to track Silent Assassin
  even for maps that do not have targets.
  If enabled, for maps without targets, instead of showing "No Targets",
  the tracker will show "Silent Assassin" or "No Silent Assassin"
  depending on your statistics.
  This is a deviation from the game so it is not enabled by default.
  However, it makes the Silent Assassin rating consistent across all maps,
  and some players may prefer playing this way.

  Beware that
  *in a handful of missions, the game triggers a non-target kill if you do not first locate the target*
  (e.g. King, Dom Osmond, Layla, Jade, possibly more).
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
* https://github.com/Deji69/HitmanStatisticsUber (extended version of HitmanStatistics with shots fired fix for H2SA)
* https://github.com/Attrup/StatTracker (similar to HitmanStatistics but written in rust)
* https://github.com/kurtis2221/hitmanstat (similar to HitmanStatistics, also supporting Hitman: Blood Money)
* https://github.com/OrfeasZ/Statman (statistics tracker for Hitman: Blood Money, Hitman (2016), and Hitman: World of Assassination)
* https://github.com/0danny/re47 (Hitman: Codename 47 reverse engineering project)
* https://github.com/ReGlacier/ReHitman (Hitman: Blood Money reverse engineering project)
* https://github.com/pavledev/HitmanAbsolutionSDK (Hitman: Absolution development kit)
* https://github.com/OrfeasZ/HitmanUnlocker (Hitman: Codename 47 configurable draw distance)
