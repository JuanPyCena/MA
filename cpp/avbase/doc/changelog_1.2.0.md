Release Notes - avbase - Version 1.2.0 - 2016-11-07
===================================================

### Added

* Empty

### Changed

* [DIFLIS-5210] - 64 bit compile fixes
* [SWE-4188] - fixes for clazy build
* [SWE-4203] - Use QApplication instead of QGuiApplication in Qt5 unit test macros to allow using widget

### Deprecated

* [SWE-4106] - Deprecated AVHashString::clearValueHashes() - cleanup now happens automatically. Will be removed in next version (SWE-4200).

### Removed

* Empty

### Fixed

* [SWE-4106] - Fixed root logger and AVHashString memleaks

### Internal

* [SWE-3998] - uncommented tstavtimer.cpp, test cases of TstAVTimer fixed
* [ACEMAX-3199] - prerequisites for avd config2 porting
