Release Notes - avbase - Version 11.0.0 - 2018-06-26
====================================================

### Changed

* Some Qt warnings like QObject::connect warnings are treated as error now. [SWE-5600]
* `LogIndent` renamed to `AVLogIndent` [SWE-5709]
* Use build-system 9.3.0.
* avconfig2: don't make missing `PROJECTS_HOME` fatal [BS-575]
* Changed `AVConfig2Container::RegisteredParameterManipulator<T>::setRestriction()` to take a unique_ptr for clarity

### Deprecated

* `isDSWExec` and `getDSWSicStatus` are now deprecated. Use `AVExecStandbyChecker` instead [SWE-5776]

### Fixed

* Fixed avconfig2 bug which broke command line overrides for parameters in subconfigs in conjunction with inheritance [SWE-5787]
* Use Qt wrapper for env functions to prevent data races. [SWE-5866]
* Fixed missing time jumps when starting a new time reference session whithin a very short time interval. [SWE-5710]
* (Qt3 only) Added missing AVConfig2 AVSet support

### New

* Allow overriding the system time used by the AVTimeReference::singleton() in unit tests [SWE-5811]
  * Use this to avoid race conditions in time critical unit tests
* Implemented unit test helper class AVExecStandbyCheckerUnitTestDriver [SWE-SWE-5776]
* New `AVLOG_INDENT` implementation which replaces `AVLOG_ENTERXXX` [SWE-5709]
  * Allow usages with locally built strings: AVLOG_INDENT.setSection("brick: " + brick_name);
  * Extensible for future features such as AVLOG_INDENT.suppressEmpty();
* AVConsole now provides an API to register slots which ignore the "execute on EXEC only" flag [SWE-5776]
* Added AVConfig2SharedConfigHolder [SWE-5306]
  * Used to access configuration from different places in libraries
  * See AVAircraftTypeLibrary for usage pattern

### Internal and Others

* AVConsole now uses AVExecStandbyChecker internally instead of isDSWExec [SWE-SWE-5776]
* AVConsoleUnitTestConnection moved to internal helper lib [SWE-SWE-5776]
* Did some internal avconfig2 refactorings
* Fixed flaky test in module avtimereference [SWE-5792]
