Release Notes - avbase - Version 12.2.0-atcse1 - 2019-11-07
==================================================================


## Changelog


### Fixed

* Avoid incorrect warnings in context of subconfig templates [ATC-3004]
* Fixed an AVConfig2 problem which could occur with references to references [SWE-6220]
* Fixed a bug in AVConfig2ContainerValueRestriction which prevented usage with types which don't have a QTextStream<< overload

### New

* Introduce API for removing config entries in AVConfig2. (ELSIS-1038)
* Implemented loading of subconfig templates [ELSIS-631][ELSIS-630]
* Implemented saving of subconfig templates [ELSIS-633]
* Implemented automatic creation of avconfig2 subconfig template map entries [ELSIS-632]
* provide AVConfig2SharedConfigHolder::setUnitTestConfig()
* added `AVConfig2::hasAssociatedContainer()` to decide whether the unit test constructor was used for a config [SWE-6241]
