Release Notes - avbase - Version 3.0.0 - 2017-04-06
===================================================

### Changed

* It is now legal to call AVExplicitSingleton::deleteSingleton for not initialized singletons
* (Qt5 only) Use class enum for AVDateTime::UtcOffsetBehaviour [SWE-4501]
* (Qt5 only) added remove() and replace() to AVPtrList  [SWE-4737] [ACEMAX-3228]
* (Qt5 only) AVMultiMap adaptions [SWE-4737] [ACEMAX-3228]

### Deprecated

* Deprecated usage of AVSingleton without inheritance [SWE-4226]
  * Reason: this leads to multiple singleton instances in conjunction with windows DLLs.

### Fixed

* Fixed Archraw racing condition [SWE-4360]
* Use AVResolveHost in AVTimeReferenceAdapterServer, working around Qt3 name resolution issues [SWE-4542]
* Fixed windows exception handling code [SWE-4687]
  * Just write out that something went wrong and abort(), don't try to "properly shut down" via AVDaemonDeinit()
* AVConfig2 command line parsing now handles empty argument strings correctly
* AVDEPRECATE() now handles uninitialized AVDeprecatedList singleton
* (Qt5 only) Fixed AVFromStringInternal compile problem [SWE-4627]
* (Qt5 only) Fixed porting error in AVSet::substract (sic) [SWE-4513]
* (Qt5 only) Fixed conversion degree (decimal) to degree/minutes/seconds [SWE-4584]
* (Qt3 only) Fixed archraw race condition [SWE-4360]

### New

* Report unsupported AVConfig2 types at compile time [SWE-4525]
* AVConfig2 support for AVDateTime [SWE-4525]
* AVLog: log user which started a program when opening a log file [SWE-4485]
* Added SHM variable name required for smrproxy5000
* (Qt5 only) QHash support for AVConfig2
* (Qt5 only) added Qt5 ported AVTextWriter to deprecated [SWE-4737] [ACEMAX-3228]
* (Qt5 only) added missing avmisc SIC/SAC functionality required for Qt3 ports [SWE-4737] [ACEMAX-3228] [SWE-4735] [ACEMAX-3050]

### Removed

* Deleted AVDateTime::currentDateTime to catch violations of clazy AVDateTime checks earlier

### Internal and Others

* (Qt5 only) Removed implementations of setMSecsSinceEpoch and toMSecsSinceEpoch which are now provided by Qt
* Fix gcc 6.2 build [SWE-4622]
  * Use unique_ptr instead of deprecated auto_ptr
  * Add missing math include for "trunc" call
* Use LocateBuildSystem.cmake template from bs 4.0.0
* Workaround for "replace py.test --no-sugar" [BS-403]
* Extended AVResolveHost unit test [SWE-4477]
