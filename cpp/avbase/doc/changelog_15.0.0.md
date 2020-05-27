Release Notes - avbase - Version 15.0.0 - 2019-09-13
====================================================

### Changed

* Allow single digit fields in date/time AVFromString code; triggered by different behaviour in Qt 5.12 [SWE-6656]
* Config warning check changed from Error to Warning to support new version 3.
* Use build-sytem 12.0.0.

### Fixed

* AVLog now emits signalFatalLog when fatal log occours.
* Fixed unittests having exit code 0 despite having failed.
* Fixed too strict assert check in AVDomDouble::setValue (ELSIS-1325).
* AVDomStringListSep does not discard empty items anymore. [ELSIS-1332]

### New

* Added coldstart parameter as avconfig2builtin parameter [SWE-5487]
  * from now every process can be started with the coldstart command-line switch, used for manual restart of all processes with our runtime
* Introduce API for removing config entries in AVConfig2. (ELSIS-1038)
* Implemented saving of subconfig templates [ELSIS-633]
* Implemented automatic creation of avconfig2 subconfig template map entries [ELSIS-632]
* Added new process deadlock detection mechanism. [SWE-6247]
   * See: https://confluence.avibit.com:2233/x/naUIAw
* Allow pinning of saved cstyle config file version [ELSIS-1338]

### Removed

* (Qt5 only) Removed deprecated AVConfig2 constructor and AVConfig2::registerSubconfigDeprecated. (ELSIS-634)
* Removed avstd::make_unique. Use std::make_unique instead.
