Release Notes - avbase - Version 13.0.0 - 2019-03-11
====================================================


## Changelog


### Changed

* Removed default implementations of some AVTimeReferenceAdapter methods
* (Qt5 only) Disable usage of system proxy when environment AV_USE_SYSTEM_PROXY_CONFIGURATION is set to 0. [SWE-6212]

### Fixed

* Made many methods from AVRule framework const
* Fixed an AVConfig2 problem which could occur with references to references [SWE-6220]
* Fixed a bug in AVConfig2ContainerValueRestriction which prevented usage with types which don't have a QTextStream<< overload
* (Qt5 only) Fixed some UTF-8 encoding issues in AVLog and AVConsole [SWE-6361]

### New

* AVPropertyComputable now supports nested const computables (also added unit tests)
* Implemented loading of subconfig templates [ELSIS-631][ELSIS-630]
* AVTimer has a new property/method to query the remaining time. [SWE-6143]
* provide AVConfig2SharedConfigHolder::setUnitTestConfig()
* added `AVConfig2::hasAssociatedContainer()` to decide whether the unit test constructor was used for a config [SWE-6241]
