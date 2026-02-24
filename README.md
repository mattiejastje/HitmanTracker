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

### Hitman 2: Silent Assassin

Nearly fully supported. Known issues:

* Difficulty is not tracked.

### Hitman: Contracts

Nearly fully supported. Known issues:

* Difficulty is not tracked.

### Hitman: Blood Money

Fully supported.

### Hitman: Absolution

Experimental support. Not ready for general use.

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
