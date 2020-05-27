Release Notes - avbase - Version 17.0.0 - 2020-02-26
====================================================


## Changelog


## avlib


### Fixed

* Log as errors as fatal now works with old style AVLog::Write functions. [SWE-6883]

### New

* Introduced data type AVToStringRepresentation [ATC-3797]

### Internal and Others

* Let tests run in parallel on bamboo [ATC-3637]

## avauth


### Changed

* Local HTTP Server only listens on localhost.
* AVAuthenticator supports multiple role lists for different clients in the ID token and selects the correct one. [SWE-7095]
  * Previously it would return the first one.
* Authenticator now takes its config by reference (std::unique_ptr) and not longer by value.

### Fixed

* Fixed assert on login after login timeout. [RDSAM-2234]
* Fixed no error being emitted when refresh timeout could not be computed. [RDSAM-2238]

### New

* Logout invalidates the tokens on the server. [SWE-7101]

### Internal and Others

* Added unittests for Authenticator. [SWE-7109]

## avunittesthelperlib


### Changed

* Unit tests now run with <an>_HOME and current working directory set to a temporary directory [ATC-3637]
  * This allows creating files in the current directory without cluttering source / build / installation folders
  * Log files are also writting to this temporary folder
  * Folder deletion can be prevented with AV_KEEP_TEMPORARY_UNIT_TEST_HOME=1

### New

* Added MockQNetworkAccessManager to replace QNetworkAccessManager in unittests.
