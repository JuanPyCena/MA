Release Notes - avbase - Version 2.0.0 - 2017-01-20
===================================================

### Fixed

* Fixed shutdown of AVTimeReferenceAdapterServer (don't access deleted memory) [SWE-4350]
* (Qt5 only) Fixed KDAB-ported AVList to avoid memleaks (inherit from AVPtrList) [SWE-4413]

### Internal and Others

* (Qt3 only) Fixed memleak in TstAVTimeReferenceAdapterServer unit test

### Removed

* Removed unused AVMsgLRImg [SWE-4117]
