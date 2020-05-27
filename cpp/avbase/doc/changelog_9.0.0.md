Release Notes - avbase - Version 9.0.0 - 2017-11-16
===================================================

### Changed

* Use *unique_ptr* in AVExpressionParser to indicate ownership [SWE-5358]
* AVSingleton and AVExplicitSingleton are not longer allowed to be used without inheritance [SWE-4551]
* *AVSingleton::createInstance* can be used with parameters which are forwarded to the constructor
* Use build-system 8.1.0 [SWE-5362]
* Renamed *AVConfig2Global::keepConfigInstance* to *AVConfig2Global::keepOrDeleteConfigInstance* and pass a unique_ptr for clarity
* (Qt5 only) The Qt message handler now shows the method name before the log message
* Private constructor of AVExecStandbyChecker (using test driver) is now public to allow non-singleton instances in unit tests [PBST-49]

### Fixed

* (Windows only) Attempt to work around an endless recursion in the unhandled exception filter which can occur on some platforms [SWE-5452]
  * The problem has only been observed with windows server 2008, and may depend on other registry settings
* Allow config object creation in postRefresh() and subconfig map constructor implementations [SWE-5437]
  * Without this fix, postRefresh() can randomly not be called if a config is created in any of those situations.
* Ensure objects are deleted eventual after call of deleteLater [SWE-5358]
  * **ATTENTION** This means that the Q(Core)Application has do outlive the call to AVDaemonDeinit (this is now asserted)
  * sendPostedEvents is now used instead of the AVDeleteLater workaround (which has been removed)
* Calls to abort(), AVASSERTS and AVLogFatals now also create core dumps in windows [SWE-5410]
* Fixed compiler warning when using AVLog without braces in if/else

### New

* Introduced C++14 feature *std::make_unique* into new namespace "avstd" as avstd::make_unique [SWE-5358]
  * If stdlib supports make_unique its implementation is used

### Internal and Others

* Use `environment_script\_add_source_directory_to_environment_variable` to add `avlib/bbtestlib` to `PYTHONPATH` to fix BB tests in devel env [BS-456]

### Removed

* Removed AVDeleteLater workaround [SWE-5358]
* Moved AVWMLayerConfig class to avuilib [ACEMAX-3203]
