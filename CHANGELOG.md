# Changelog

All notable changes to this tutorial repository are documented here. Format
loosely follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/);
entries are grouped under `Unreleased` until the curriculum's first tagged
release.

## [Unreleased]

### Added

- Initial repository scaffold: `Getting_Started`, `Sensors_And_Storage`,
  `ESP32_Connectivity`, `Cloud_Messaging`, `AWS_IoT_Core`,
  `Production_Firmware`, and `Developer_Contributed`, each with a README and
  numbered tutorial folders following the Feature/Design Tutorials split.
- Tutorial content is scaffolded (introduction, learning goals, prerequisites,
  reference-implementation links, step outline, done-when criteria) but not
  yet fully written up step by step — see each tutorial's `README.md`.
- Tiremo Cortex logo (light/dark) and brand banner strip on every README,
  under `images/`.
- Turkish translation of the top-level README (`docs-tr/README.md`), linked
  from a language switcher at the top of the English one.

### Changed

- Removed every doc reference to `referance_only/` (private, gitignored,
  never shipped to a clone of this repo). Each tutorial's former
  "Reference Implementation" section is now a "Solution" section, honestly
  stating that no solution has been written yet; the eventual plan is for
  each tutorial to carry its own local `Sw/` folder once one exists.
