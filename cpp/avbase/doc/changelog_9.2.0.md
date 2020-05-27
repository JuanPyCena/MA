Release Notes - avbase - Version 9.2.0 - 2017-12-12
===================================================

### Changed

* Use build-system 8.3.0

### Fixed

* (Qt5 only) Fix timezone handling on Windows [SWE-5506]
  * This works around a MSCRT problem when the TZ environment variable is set. E.g. in Cygwin.

### Internal and Others

* (Qt3 only) Handle randomly received data in multicast unittest [SWE-5267]

### Removed

* (Qt3 only) Moved AVWMLayerConfig and AVCWPStyle classes to avuilib [ACEMAX-3203]
