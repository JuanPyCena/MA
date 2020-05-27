Release Notes - avbase - Version 7.0.0 - 2017-07-19
===================================================

### Changed

* Fixed AVConsoleCommandPrintQObject compile issues with MSVC 2015 [SWE-4526]
* Use CMake function *av_add_pytests_recursive()* instead of *av_add_pytest_run_in_current_directory()* which recursively adds all py.test files [TFW-624]
* (Qt5 only) Extended AVQt3Compat::createPen() to better resemble the Qt3 default QPen [ACEMAX-2891]
  * Changed cap style, join style and default width to the Qt3 default values

### Fixed

* Fixed fatal error on empty data for AVDataFramingDelimiters. [SWE-4922]
* Fixed an AVLog deadlock which occurs at midnight if USER env variable is not set [SWE-5055]
  * Introduced with avbase 3.0.0
* (Qt5 only) Allow all Qt command line options to QGuiApplication [SWE-5083]
* (Qt5 only) Fixed a Windows compile warning regarding a signed/unsigned comparison

### New

* Introduced core dump AVLogError in development environment [SWE-4103, SWE-4523]
  * See [Handling Warnings / Errors During Development](https://confluence.avibit.com:2233/pages/viewpage.action?pageId=14319717) for details
* Write minidumps when crashing under windows [SWE-5048]
  * **ATTENTION** This means that core dump housekeeping must be configured for windows projects from now on!
  * You can disable this behaviour by setting the environment variable AVBASE_ENABLE_MINIDUMP=0
* Table configuration support for transition level computation [SWE-4918]

### Internal and Others

* Temporarily disabled timer tests with race conditions [SWE-4588]
