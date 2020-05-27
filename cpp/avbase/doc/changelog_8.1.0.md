Release Notes - avbase - Version 8.1.0 - 2017-09-21
===================================================

### Documentation

* Added design for shared memory in confluence [SWE-4104]

### Changed

* Encode AVConfig2 files as UTF-8 [SWE-5315]

### Fixed

* (Qt5 only) Fixed incorrect line and file in qtMessageHandler [SWE-5340]
* (Qt5 only) Fixed memory exhaustion problem with AVConsoleTelnetConnection [SWE-5293]
  * The QTcpSocket objects were deleted on shutdown and not after disconnect.
* AV_REGISTER_METATYPE now allows special characters in type (pointers, templates) [SWE-5276]
* Better error output if running out of SHM space [SWE-4104]
* Increased SHM default values for PSD count and size [SWE-4104]

### New

* Allow referencing AVConfig2 QStringList parameters in AVRule <forEach> expressions. Example:
```
    <forEach name="rp">@config.fdp2.housekeeping.icao_route_points_for_activation</forEach>
```

* SHM improvements [SWE-4104]
  * Print basic SHM usage summary at process startup
  * AVConsole slot *printShmUsageSummary* for a more detailed summary
  * Log an error if less than 5 PSD blocks or 20 param blocks remain
  * Exceeding the maximum configured number of SHM PSD blocks now is FATAL (because this is expected to occur at startup time)

### Internal and Others

* Avoid avbase internal usage of deprecated AVConfig2 constructor with *help_group* parameter
