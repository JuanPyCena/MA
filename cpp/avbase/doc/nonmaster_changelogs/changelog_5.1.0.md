Release Notes - avbase - Version 5.1.0 - 2017-07-19
===================================================

### Changed

* Use build system 5.0.2
* (Qt5 only) Track thread names in QThread's object name [SWE-4653]

### Fixed

* Fixed an AVLog deadlock which occurs at midnight if USER env variable is not set [SWE-5055]
  * Introduced with avbase 3.0.0
* (Qt5 only) Allow all Qt command line options to QGuiApplication [SWE-5083]

### New

* Write minidumps when crashing under windows [SWE-5048]
  * **ATTENTION** This means that core dump housekeeping must be configured for windows projects from now on!
  * You can disable this behaviour by setting the environment variable AVBASE_ENABLE_MINIDUMP=0
