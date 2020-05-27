Release Notes - avbase - Version 2.2.0 - 2017-07-24
===================================================

### Fixed

* Fixed windows exception handling code [SWE-4687]
  * Just write out that something went wrong and abort(), don't try to "properly shut down" via AVDaemonDeinit()
* use python unit test macro (avoid no longer supported "--nosugar" cmd line argument) [BS-403]

### Changed

* Use build system 3.0.5

### New

* Write minidumps when crashing under windows [SWE-5048]
  * **ATTENTION** This means that core dump housekeeping must be configured for windows projects from now on!
  * You can disable this behaviour by setting the environment variable AVBASE_ENABLE_MINIDUMP=0
* Table configuration support for transition level computation [SWE-4918]
