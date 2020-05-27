Release Notes - avbase - Version 1.0.0 - 2016-09-07
===================================================

### Added

* [SWE-3972] - (Qt3 only) Implemented multicast interface filter for AVSocketDeviceMulticast
* [SWE-3783] - (Qt3 only) Provide a thread-safe UUID generation method (backport from Qt5)
* [SWE-3958] - Moved avlib bbtestlib functionality from SVN to avbase

### Changed

* [SWE-3788] - Changed avconfig2 default "strict checking" mode to "no strict checking".
* [SWE-3959] - Renamed AVTimeReferenceHolder::reset() to AVTimeReferenceHolder::resetToSystemTime()

### Deprecated

* [SWE-3783] - Deprecated seeding the random number generator in application code (it is now done always at initialization time)

### Removed

* Empty

### Fixed

* [SWE-3946] - Improved avconfig2 error output for missing parameters specified on the command line
* [SWE-3963] - Removed usage of QDir::separator in avfilewatcher, as it does not behave correctly on windows

### Internal

* [SWE-3930] - fix log test race conditions
* [SWE-3916] - merge SVN changes since avbase git export
* [SWE-3783] - improved unit test handling of RESOURCE environment variable
* [SWE-3863] - preserve order of registered parameters in avconfig2
* [SWE-3959] - fixed time reference unit test race conditions