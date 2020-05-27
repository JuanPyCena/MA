Release Notes - avbase - Version 1.3.2 - 2016-12-07
===================================================

### Fixed

* Unquote strings used as part of aggregate references [SWE-4315]
* Qt3 only: fixed AVSocketDeviceMulticast for Windows [SWE-4239]
* Fixed AVLicense::makeLicense() access violation [SWE-4271]
* Fixed logic error in AVConfig2Container::addInheritedSection (incorrect error message if all parameters were overridden) [SWE-4318]
