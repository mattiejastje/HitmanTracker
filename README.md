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

Partial support.

A very simple silent assassin rating is implemented
to mimick how the rating system works in the other games,
whilst retaining some consistency with the scoring system in Absolution:

* If a checkpoint is unrated,
  at the moment the tracker is unable to track the statistics,
  and awards an "Unrated" rating.
  The technical reason for this is that the tracker verifies score board,
  but the engine keeps no score board for unrated checkpoints.

* If a checkpoint is rated, the tracker awards a "Silent Assassin" rating
  in these circumstances:

    - Not spotted.
    - No civilians killed.
    - No non-targets killed if the map has dedicated targets (i.e. if the map
      has no dedicated targets, every non-civilian is considered a target).

For convenience, the tracker shows whether evidence is left,
but this currently does not affect the rating awarded by the tracker.

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
