Release Notes - avbase - Version 18.0.0 - 2020-04-16
====================================================


## Changelog


## avlib


### Changed

* AV[Explicit]Singleton are visible per default if compiled with "-fvisibillity-hidden" on Linux.
* Ensure AVThread is really started and log QThread releated messages as error.
* Removed checking the [name_space.]camelCase convention for AVConsole slots [SWE-7180]
* Use build-system 14.0.0.

### Fixed

* Fixed sanitizer warning when reading multiple times into the same AVCVariant instance.

### New

* Added AVTypesafeSignalSpy to allow for testing of signals with types not registered with the QMetaType system.

## avunittesthelperlib


### Fixed

* Added missing QTemporaryDir forward declaration in avunittesthelpers.h
