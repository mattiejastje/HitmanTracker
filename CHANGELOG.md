# Changelog

## [Unreleased]

### Added

- Diagnostics: show memory throughput.

### Changed

- Hitman 2 Silent Assassin and Hitman Contracts performance improvement: the difficulty property offset is now cached so the code no longer needs to iterate over the full property list 10 times per second. For Hitman Contracts this reduces memory throughput by a factor 10.

## [0.8.3] - 2026-08-18

### Added

- Hitman Blood Money: new option to merge npcs (innocents/enemies/police) into a single row.

- Hitman Blood Money: new option to merge frisk failed and cover blown into a single row.

- Hitman Blood Money: new option to merge bodies found (target/unconscious/total) into a single row.

- Hitman Blood Money: new option to merge witnesses and on camera into a single row.

- Hitman Blood Money: new option to merge items left into a single row.

- New diagnostics tab, showing frame time, fraction of time spent on updating statistics and timer, and update failure rates.

### Changed

- When opening the game process, only the strictly necessary access rights are requested.

- Signals and timers will now behave better after lag spikes.

- Some minor internal quality-of-life refactoring of code.

### Fixed

- Fix bug when pointer reading for large address aware 32-bit executables.

- Fix formatting of negative times (should they ever occur).

- Signals are now consistently updated even when a game is not running.

## [0.8.2] - 2026-08-07

### Changed

- Some internal quality-of-life refactoring of code.

### Fixed

- Log tracing is now properly disabled when settings are reset.

- Fixed invalid D3D context causing tracker to hang in rare circumstances.

- Fixed timer wrapping around 60 minutes.

### Removed

- Deleting log files during operation could result in a rare crash. Since the logs folder can be easily opened for manual deletion, for simplicity, the clear log files button has been removed.

## [0.8.1] - 2026-07-19

### Added

- The tracker window now has a white border when not in overlay mode, to make it easier to find. The size of the border is configurable, and the border can be disabled.

- Logs are now stored in a ``logs`` subfolder next to the executable instead of the install root.

- The help menu and logging tab have a new "Log Folder" entry for quickly finding the logs when reporting a bug.

- Log files can be easily deleted.

- Recent errors can be optionally shown, copied, and cleared.

- Logs now show game for additional context.

- The github builds now cache vcpkg dependencies, resulting in faster builds.

### Changed

- The ``--overlay`` flag is now called ``--overlay-mode`` for consistency with the UI and to make it more clearly distinguishable in case more overlay options are added in the future.

- Some internal quality-of-life refactoring of code.

## [0.8.0] - 2026-07-15

### Added

- The tracker window now has its position and size stored across sessions.

- Resizing the font size will now also resize the tracker window.

- Menu bar: reset settings to default, link to bug tracker, open readme, open changelog, open license, about.

### Changed

- The vcpkg baseline has been updated. Now using brotli@1.2.0, bzip2@1.0.8#6, cli11@2.6.2, fmt@12.2.0, freetype@2.14.3, imgui@1.92.8#1, libpng@1.6.58, nameof@0.10.6, spdlog@1.17.0#1, and zlib@1.3.2#1.

- Log level and flush level are now synced when configured in settings, for simplicity. Separate values can still be used on the command line.

### Fixed

- The settings window no longer loses focus when disabling overlay mode.

- Fixed rare bug affecting lost Direct3D device.

- Fixed rare shutdown bug when ImGui could not initialize.

## [0.7.0] - 2026-07-13

### Added

- The GOG version of Hitman 2 Silent Assassin is now supported.

- The GOG version of Hitman Blood Money is now supported.

- The old topmost option has been enhanced, and is now called "overlay mode". In this mode, the window is transparent and click-through so it can be used as direct in-game overlay, provided the game is played in windowed mode.

- The layout can now be edited: all individual elements can be shown or hidden.

### Changed

- Game version is now shown on a separate line, with its own configurable size and color.

- Various minor refactorings and improvements.

- The settings window is now separate from the stats window.

- The stats window can be dragged around by clicking anywhere, and resized by clicking its borders.

- The settings are now organized into tabs for easier use.

### Removed

- Background color is now always black (for technical reasons, this simplifies implementation of transparency).

## [0.6.0] - 2026-07-07

### Fixed

- Various minor bugs fixed.

- Hitman Contracts: reduce CPU usage.

- Hitman Absolution: fixed minor typos in level names.

- Hitman Absolution: reduce CPU usage.

### Added

- The GOG version of Hitman Codename 47 is now supported.

- The GOG version of Hitman Contracts is now supported.

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

[unreleased]: https://github.com/mattiejastje/HitmanTracker/compare/v0.8.3...HEAD
[0.8.3]: https://github.com/mattiejastje/HitmanTracker/compare/v0.8.2...v0.8.3
[0.8.2]: https://github.com/mattiejastje/HitmanTracker/compare/v0.8.1...v0.8.2
[0.8.1]: https://github.com/mattiejastje/HitmanTracker/compare/v0.8.0...v0.8.1
[0.8.0]: https://github.com/mattiejastje/HitmanTracker/compare/v0.7.0...v0.8.0
[0.7.0]: https://github.com/mattiejastje/HitmanTracker/compare/v0.6.0...v0.7.0
[0.6.0]: https://github.com/mattiejastje/HitmanTracker/compare/v0.5.0...v0.6.0
[0.5.0]: https://github.com/mattiejastje/HitmanTracker/compare/v0.4.0...v0.5.0
[0.4.0]: https://github.com/mattiejastje/HitmanTracker/compare/v0.3.1...v0.4.0
[0.3.1]: https://github.com/mattiejastje/HitmanTracker/compare/v0.3.0...v0.3.1
[0.3.0]: https://github.com/mattiejastje/HitmanTracker/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/mattiejastje/HitmanTracker/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/mattiejastje/HitmanTracker/releases/tag/v0.1.0