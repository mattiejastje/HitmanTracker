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

#### How the game's rating system works

Hitman: Absolution determines your final rating in two separate stages.

##### 1. Condition-based rating

The game first checks whether you qualify for one of its predefined playstyles,
such as **Silent Assassin**, **Piano Man**, **Jinx**, and so on.

For **Silent Assassin**, the requirements are:

* No non-target kills.
* Never spotted.
* All targets eliminated.

If one of these condition-based ratings is awarded, the score-based system is ignored.

**Limitation:** checkpoints without targets can never receive **Silent Assassin**,
because the game never records that all targets have been eliminated.

##### 2. Score-based rating

If no condition-based rating is awarded,
the game falls back to a score-based rating.

The calculation is:

1. Take your raw score (before difficulty and challenge modifiers).
2. Divide it by the checkpoint's internal **Shadow Score**.
3. Convert the result to a percentage (rounded down).

The percentage determines the rating:

|  Score | Rating       |
| -----: | ------------ |
|  0–49% | Agent        |
| 50–79% | Veteran      |
| 80–89% | Specialist   |
| 90–99% | Professional |
|   100% | Shadow       |

**Limitation:** some checkpoints have an incorrectly configured **Shadow Score**,
making **Shadow (100%)** impossible to achieve.

#### How the tracker improves this

The tracker separates the two systems and tracks them independently.

##### Silent Assassin tracking

For checkpoints with targets, **Silent Assassin** is awarded when:

* No non-target kills.
* Never spotted.

An optional setting also allows **Silent Assassin** to be tracked on checkpoints without targets.
This behaviour intentionally differs from the original game and is disabled by default,
but it provides consistent Silent Assassin tracking across every checkpoint.

> **Note**
>
> A small number of missions appear to contain a bug where the game records a non-target kill unless you first locate the target.
> Known examples include King, Dom Osmond, Layla and Jade.

##### Score tracking

For score-rated checkpoints, the tracker:

* Calculates the score rating independently of Silent Assassin.
* Uses the same percentage thresholds as the original game.
* Displays the score required for the highest achievable rating
  (for example, 80% of the Shadow Score if **Specialist** is the highest achievable rating).
* Color-codes score categories such as evidence removed, objectives completed, target kills and signature kills.
  A category is shown in green only when it has reached its maximum possible value,
  making it easy to see what is still missing.

This approach remains faithful to the original game while addressing its shortcomings:

* Makes Silent Assassin attainable on every checkpoint.
* Separates skill-based and score-based progression.
* Makes scoring requirements fully transparent.

#### Rating configuration

Modes define what is tracked per checkpoint and how it is displayed.
Presets provide common configurations built from these modes.

##### Rating modes

| Mode      | Description                                                                             |
| --------- | --------------------------------------------------------------------------------------- |
| **X**     | No tracking.                                                                            |
| **SA**    | Track **Silent Assassin** only.                                                         |
| **SC**    | Track **Score** only.                                                                   |
| **SA+SC** | Track both **Silent Assassin** and **Score**.                                           |
| **SA→SC** | Track **Silent Assassin**. Track **Score** only if **Silent Assassin** is not achieved. |

Not every checkpoint supports every tracking mode.

* **Unrated checkpoints** support only **X** and **SA**.
  **SC** is not available on these checkpoints.
* **All other checkpoints** support every mode.

#### Rating presets

| Preset              | Unrated | No targets | Targets   |
| ------------------- | ------- | ---------- | --------- |
| **Original Game**   | **X**   | **SC**     | **SA→SC** |
| **Maximum Rating**  | **X**   | **SA+SC**  | **SA**    |
| **Silent Assassin** | **SA**  | **SA**     | **SA**    |
| **Full Tracking**   | **SA**  | **SA+SC**  | **SA+SC** |

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
