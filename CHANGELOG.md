# Changelog

## [Unreleased]

### Fixed

- Various minor bugs fixed.

- Hitman Absolution: fixed minor typos in level names.

- Hitman Absolution: efficiency improved.

### Added

- The GOG version of Hitman Codename 47 is now supported.

### Changed

- Minor refactoring of hooking code.

- Various hooking errors will now also be caught in release builds.

- Large internal refactoring to allow the tracker to open multiple windows in future releases.

## [0.5.0] - 2026-07-01

### Fixed

- Hitman Absolution's mission timers now only start when movie cutscenes (shown during mission load)
  are finished playing. This should more closely match livesplit.

### Added

- Hitman Absolution has a new option to apply difficulty and challenge bonus on scores.
  This is enabled by default, so scores will match the score board by default.
  Note the game shows rounded values, whereas the tracker shows true values.

### Changed

- Some code has been refactored and some minor efficiency gains have resulted.

## [0.4.0] - 2026-06-30

### Added

- The game version (Steam, GOG, ...) is now shown for all games.

- New setting to configure display of the game version
  (i.e. Steam, GOG, ...).

- Hitman Absolution:

  * The GOG version of Hitman Absolution is now supported.

  * New options to configure how much detail is shown.

  * New tracking modes with presets for original game, max rating, silent assassin, and full tracking.

  * Improved documentation.

## [0.3.1] - 2026-06-28

### Fixed

- Game detection of Hitman Absolution was broken due to steam modifying
  the executable upon install (aka CEG). This is now fixed.

### Changed

- Now using the date_time_stamp field of the PE header
  to validate executables and dlls instead of relying on a checksum.

## [0.3.0] - 2026-06-27

### Fixed

- In very rare cases, one or more hooks could be missing
  without an error being logged. This is now fixed.

- Fixed bug in aggression (and thus, silent assassin)
  on first map in Hitman Contracts.

- CMakeLists.txt now properly targets C++23 as required.

### Changed

- Replaced F1 with Settings button, which is more intuitive.

- Settings now appear in a popup window
  so they do no longer interfere with the layout.

### Removed

- Removed settings tooltips as no longer needed.

- Removed option to hide menu on application start.

## [0.2.0] - 2026-06-25

### Added

- Added some tooltips so users know to press F1 to toggle menu.
- New option to hide the menu on application start.

### Changed

- Moved menu to top of user interface.
- Menu is now shown by default on application start.

### Removed

- Removed unused font file from distribution.

## [0.1.0] - 2026-06-25

Initial public release.

[unreleased]: https://github.com/mattiejastje/HitmanTracker/compare/v0.5.0...HEAD
[0.5.0]: https://github.com/mattiejastje/HitmanTracker/compare/v0.4.0...v0.5.0
[0.4.0]: https://github.com/mattiejastje/HitmanTracker/compare/v0.3.1...v0.4.0
[0.3.1]: https://github.com/mattiejastje/HitmanTracker/compare/v0.3.0...v0.3.1
[0.3.0]: https://github.com/mattiejastje/HitmanTracker/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/mattiejastje/HitmanTracker/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/mattiejastje/HitmanTracker/releases/tag/v0.1.0