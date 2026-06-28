# Changelog

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

[0.3.1]: https://github.com/mattiejastje/HitmanTracker/compare/v0.3.0...v0.3.1
[0.3.0]: https://github.com/mattiejastje/HitmanTracker/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/mattiejastje/HitmanTracker/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/mattiejastje/HitmanTracker/releases/tag/v0.1.0