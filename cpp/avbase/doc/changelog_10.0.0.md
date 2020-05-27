Release Notes - avbase - Version 10.0.0 - 2018-01-31
====================================================

### Documentation

* Improved documentation for AVHostname and AVGetProcessName

### Changed

* All AV(Explicit)Singleton instances get deleted eventually on shutdown. [SWE-4973]
 * The order of destruction of AV(Explicit)Singleton can be changed via DEINIT_RANK member.
 * This means that no singleton can be accessed after AVDaemonDeinit.
* Changed access of several AVConfig2 methods. [SWE-5578]
  * All constructors are protected now.
  * registerParameter, registerSubconfigDeprecated, registerSubconfig are public now.
* Allow namespace prefix for AVConsole commands. [SWE-5575]

### Deprecated

* Deprecated `AVConfig2Container::assignLoadedMetadataTo` [SWE-5577]
* (Qt5 only) Deprecated AVHostName

### Fixed

* Introduced AVConfig2Container::findConfig [SWE-5552]
* Moved signal handler registration from AVApplicationPreInit to AVApplicationInit [SWE-5580, SWE-4483]
  * This fixes the overwriting of the signal handler within the QApplication constructor (QCoreApplication is not affected)
* Fixed Qt3 windows AVTempDir to use '/' as directory separator [SWE-5566]
* Fixed AVConfig2Saver to allow calling save() multiple times [SWE-5566]
* Other minor AVConfig2Saver fixes [SWE-5566]
  * Improved error output when using old config implementation in AVConfig2 process
  * Don't log storage location for parameters which actually are filtered
  * Reduced logging failure of locateConfig() from INFO to DEBUG

### New

* (Qt5 only) Added qHash implementation for std::shared_ptr.
* Introduced an AVConfig2 unit test constructor, which avoids association with an AVConfig2Container
* (Qt5 only) Added qHash implementation for all enums. [SWE-5300]
