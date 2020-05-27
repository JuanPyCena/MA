Release Notes - avbase - Version 9.1.0 - 2017-11-28
===================================================

### Changed

* Sort entries in QHash and QSet AVToString representation to achieve consistent output in config files [SWE-5444]
* Use build-system 8.2.0

### Fixed

* Always preserve order when saving subconfig maps [SWE-5465]
  * This problem was introduced in avbase 6.0.0 with the replacement of AVSet
* Improved documentation for `AVLogStream::SuppressRepeated`
* Windows: don't die silently but create core dump when the invalid parameter handler is called [SWE-5469]

### Internal and Others

* Added AVConfig2 regression test for EMA CMS bug [SWE-4807]
