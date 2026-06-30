# HitmanTracker

A statistics tracker for Hitman games.

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

### Hitman: Codename 47

For now, only the Steam version is supported.

This game has no rating system, so no stats are tracked.
However, there is basic support to track map name, internal game timer,
and difficulty.

### Hitman 2: Silent Assassin

For now, only the Steam version is supported.

### Hitman: Contracts

For now, only the Steam version is supported.

### Hitman: Blood Money

For now, only the Steam version is supported.

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

Fully supported on both Steam and GOG.

#### The game's rating system explained

The rating system is notoriously confusing.
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
the tracker tracks both rating systems independently as follows:

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

#### Rating Configuration

Each map can be tracked in one of the following modes:

* **X** – Do not track anything.
* **SA** – Track **Silent Assassin** only.
* **SC** – Track **Score** only.
* **SA+SC** – Track both **Silent Assassin** and **Score** independently.
* **SA→SC** – Track **Silent Assassin**, but fall back to displaying **Score** if Silent Assassin is not achieved.

##### Map Support

Not every map supports every tracking mode:

* Unrated support only **X** and **SA**.
* All other maps support every tracking mode.

For maps with targets, **SC** is generally not very useful because achieving the Shadow score also requires Silent Assassin.

##### Display Options

Although the underlying tracking modes are flexible, exposing every possible combination would make the configuration unnecessarily complicated. Instead, the tracker presents two simple options that cover the common playstyles.

1. Where should Silent Assassin be tracked? Choose whether Silent Assassin is tracked:

   * **Only on maps with targets** (default, matching the game's behaviour), or
   * **On all maps**.

2. How should Score be displayed on maps where Silent Assassin is tracked? Choose one of the following:

   * **Hide** (**SA**)
   * **Show as fallback** (**SA→SC**, default) — display the score only if Silent Assassin was not achieved.
   * **Always show** (**SA+SC**)

The combination of these two settings determines the tracking mode used for each map type.

| Track SA on all maps | Score display    | Unrated maps | Maps without targets | Maps with targets |
| -------------------- | ---------------- | ------------ | -------------------- | ----------------- |
| No                   | Hide             | **X**        | **SC**               | **SA**            |
| No                   | Show as fallback | **X**        | **SC**               | **SA→SC**         |
| No                   | Always show      | **X**        | **SC**               | **SA+SC**         |
| Yes                  | Hide             | **SA**       | **SA**               | **SA**            |
| Yes                  | Show as fallback | **SA**       | **SA→SC**            | **SA→SC**         |
| Yes                  | Always show      | **SA**       | **SA+SC**            | **SA+SC**         |

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
