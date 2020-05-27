Release Notes - avbase - Version 14.0.1 - 2019-06-05
====================================================


## Changelog


### Fixed

* Fixed telnet commands being swallowed when slot starts an event loop. [SWE-6618]
* Fixed avmacros.h [SWE-6718]
  * avoid clang warning:  default label in switch which covers all enumeration values
  * adapted to coding style
