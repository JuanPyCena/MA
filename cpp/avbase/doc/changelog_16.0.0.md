Release Notes - avbase - Version 16.0.0 - 2019-11-20
====================================================


## Changelog


## avlib


### Changed

* Use build-system 13.0.0.
* Project Facts: make it a bit less strict [SWE-6547]
  * if allowed_values is empty, all values are allowed to be set in facts

### Fixed

* AVSocketDevice fixed for a connection error still reported in startup phase
* AVDomStringListSep does not discard empty items anymore. [ELSIS-1332]
* Fixed: Saving a config with restrictions could corrupt the config file [ELSIS-1358]
* Avoid incorrect warnings in context of subconfig templates [ATC-3004]

### New

* AVConsole now supports quoting using single quotes (`'`) for arguments containing whitespace [SWE-2942]
  * Within single quotes backslash can be used to escape `'` (e.g. `logMark 'Don\'t do that'`).
* introduced environment variable AVCONFIG2_PIN_CSTYLE_VERSION

## avauth


### New

* Authenticator class was added to facilitate the OAuth2 protocol.
 * Login
 * Logout
 * Refresh manually and periodically.

## avunittesthelperlib


### New

* Added new macro `AVLOG_ENTER_TESTCASE` which prints test name and data tag
