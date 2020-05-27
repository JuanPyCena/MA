Release Notes - avbase - Version 12.0.0 - 2018-08-02
====================================================

### Deprecated

* Deprecated creation by and assignment of string literals or `const char*` to AVCString to limit heap usage. [SWE-5854]
  * The creation of AVCString by string literals is no longer allowed.

### Fixed

* Suppress transient problems in AVSocketDevice
* Fixed AVConsole input file processing [SWE-5869]
  * Suspend the output connection so it cannot be immediately closed but displays the results of the commands executed from the file
  * Log information about successful processing of the file not immediately but at the correct time
  * Delete the file connection after execution has finished
  * Let the console command fail if any command in the file produces an error
* Allow unit tests to provide configuration to AVConfig2SharedConfigHolder [SWE-5919]
* Fixed AVConsole `isExec`, added `isStandby` [SWE-5881]
  * Implementation now uses the `AVExecStandbyChecker`; this means processes supporting this slot must call `init_params.enableExecStandbyChecker`
  * This now correctly handles transient states
