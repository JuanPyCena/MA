Release Notes - avbase - Version 8.0.0 - 2017-08-28
===================================================

### Changed

* Use build-system 7.1.0
* Renamed AVConfig2Global::save() to AVConfig2Global::internalSave() for clarity
* Consistently use the "override" keyword (via clang-tidy)
* (Qt5 only) qFormatLogMessage will be used to apply custom log patters in qtMessageHandler [SWE-5143]

### Deprecated

* Deprecated the usage of the avbase-internal *ENUM_FUNCTIONS* macro [SWE-5210]
  * Define your enums and their functions in classes, structs or namespaces instead.
* Renamed AVDateTime methods *start*, *elapsed*, *restart* into *startWithAvtimereference*, *elapsedWithAvtimereference*, *restartWithAvtimereference* for clarity
  * Naming those methods identical to their Qt pendants was bad API design
  * Many call sites use this methods incorrectly (e.g. for profiling) and should use QTime instead
* Deprecated *AVDateTime::fromTime_t* (deprecated since Qt 5.8)

### Fixed

* Fixed application crashing when AVPath was a static member [SWE-5263]
* Use fixed LocateBuildSystem.cmake so project-specific build systems can be found [SWE-5156]
* (Qt5 only) AVThreadObjectManager threads are properly named

### New

* (Qt5 only) Added AVDateTime method *fromMSecsSinceEpoch* (required to fix clazy AVDateTime errors)
* (Qt5 only) AVFromToString (and also AVConfig2) now supports QBitArray [SWE-5153]
* (Qt5 only) AVFromToString (and also AVConfig2) now supports QTransform [SWE-5153]
* Created enum macro for usage in namespaces: *DEFINE_TYPESAFE_ENUM_IN_NAMESPACE* [SWE-5210]
* (Qt5 only) Added new Macro to automatically register a type as meta type (AV_REGISTER_METATYPE)
* (Qt5 only) Register AVPath and AVEnvString as metatype on application startup

### Internal and Others

* Fixed timing dependency of time reference holder unit tests
