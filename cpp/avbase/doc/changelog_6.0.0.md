Release Notes - avbase - Version 6.0.0 - 2017-06-22
===================================================

### Changed

* (Qt5 only) Removed setting TZ veriable from avdaemon, all processes will now use local time [IB-32]
* (Qt5 only) Track thread names in QThread's object name [SWE-4653]
* Use build-system 6.0.0
* Improved output and documentation for conflicts when registering AVConfig2 parameters [DIFLIS-5909]
* Changed SHM variables from define to const QString [SWE-4544]
* Added a deleted overload of QObject::startTimer to AVThread to prevent accidental usage. [SWE-4575]

### Deprecated

* AVSet is has been deprecated in favor of std::set or QSet [SWE-4535]

### Fixed

* Fixed AVFlightplanProvider to build correctly using MSVC Qt5 [SWE-4779]
* Don't allow QTextStream manipulators usage with AVToString / logging [SWE-4707]
* Fixes QTimer threading issue. [SWE-4969]
* Fixed crash when saving deprecated parameter in conjunction with file name filtering [SWE-4838]
* Ensure that the stdin reader thread terminates gracefully on Windows [SWE-4824]

### New

* Changes required for AVD Qt5 integration [ACEMAX-3561]
* TTL for multicasts can be set [SWE-4768]
* Log revision info and arguments to stdout on startup [SWE-4550]
* Added SHM var name for previous state of smrproxy [ACEMAX-3615]

### Internal and Others

* Unit test to verify file descriptor are not inherited by sub-processes under Windows [SWE-4906]
* Refactoring around AVSocketDeviceMulticast [SWE-4480]
