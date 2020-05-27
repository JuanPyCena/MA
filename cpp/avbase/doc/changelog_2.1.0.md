Release Notes - avbase - Version 2.1.0 - 2017-02-16
===================================================

### Fixed

* Fixed QDateTime clazy warnings [SWE-4463]
* (Qt5-only) Fixed parsing of AV_PLUGIN_PATH in windows [SWE-4486]

### New

* New AVEnvironment::getPath() method using OS-specific path separators [SWE-4486]
* Added SHM variables for smrproxy [ACEMAX-3556]

### Internal and Others

* (Qt5 only) Fixed minor Qt 5.8 compile issue
