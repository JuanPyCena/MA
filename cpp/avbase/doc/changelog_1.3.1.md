Release Notes - avbase - Version 1.3.1 - 2016-11-28
===================================================

### Added

* Empty

### Changed

* Empty

### Deprecated

* Empty

### Removed

* Removed deprecated method AVHashString::clearValueHashes (SWE-4200)

### Fixed

* Replaced QDir::separator() with "/" according to new coding guideline
* Fixed avexpressions not to rely on Qt3-specific behaviour w.r.t. QString::null / empty string (DIFLIS-5327).
* Fixed AVPath behaviour for application names with identical prefixes (e.g. SAM and SAM_DAQS, SWE-4247)
* Also assign facts in AVConfig2Container::assignLoadedMetadataTo (SWE-4077)
* Workaround for mixed separator issues (SWE-4297, BS-304)
* signed/unsigned issue in avconfiglist.h (Qt5 only)

### Internal

* Special handling for processes without time reference adapter in console method "printTimeRefParameterStamp" (SWE-4232)
* Use AVEnvString instead of AVPath in avconfig2 search path resolution (SWE-4247)