Release Notes - avbase - Version 10.2.0 - 2018-03-28
====================================================

### Changed

* (Qt5 only) Fixed usage of override keyword within `AVObservableEvent` derived classes.

### Fixed

* Fixed avconfig2 performance issues [SWE-5647] [SWE-5716]
  * improve performance loading config file
  * improve performance refreshing parameters after loaded
  * add benchmark tests for loading config file and refresh parameters
  * compose benchmark result for CDash server

### New

* (Qt5 only) Added AVLogMuter to mute AVLog messages for lifetime of object [SWE-5677]
* (Qt5 only) QDebug logging now also supports AVPath

### Removed

* (Qt5 only) Removed dangerous `AVDomString::latin1()` method.
