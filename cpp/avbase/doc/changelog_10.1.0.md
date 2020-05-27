Release Notes - avbase - Version 10.1.0 - 2018-02-21
====================================================

### Changed

* Use build-system 9.1.0

### Fixed

* Fixed refresh of sub config did not work.  [SWE-5537]
* Fixed avconfig2 problem when saving optionals changed by application. [SWE-5652]
* AVConfig2::UnitTestConstructorSelector now is public.
* Allow setting multicast TTL on all AVSocketDeviceBase derived classes. [SWE-5586]
* Do not FATAL if telnet console connection emit disconnect signal twice. [SWE-1555]

### New

* Added `AVVersionReader` class for reading APP_HOME/VERSION file. [SWE-5587]
* New REGISTER_CONFIG_SINGLETON for registering config singletons. [SWE-5551]
  * This registers config singletons at the earliest point in time to allow e.g. saving.
* (Qt5 only) New protected AVThread::exec method allows AVThread classes to call QThread::exec from the underlying QThread. [SWE-5644]
* (Qt3 only) Added backport for Q_GLOBAL_STATIC.
