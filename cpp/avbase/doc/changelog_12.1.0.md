Release Notes - avbase - Version 12.1.0 - 2018-11-02
====================================================


## Changelog


### Fixed

* avfromtostring now correctly escapes \n in QStrings (quotes were missing before).
* Prevent data race on AV/QDateTime read from AVDataStream. [SWE-5866]
   * This fixes spurious crashes while using Squish.
* Initalizers registered via REGISTER_CONFIG_SINGLETON are only called when AVConfig2Global singleton has been initialized.

### New

* Added AVQt3Compat::normalizeRect() to emulate behavior of Qt3's QRect::normalize() [ACEMAX-4152]
* Introduced AV_FUNC_INFO and AV_FUNC_PREAMBLE. (SWE-6098)
* Added AVPropertyComputable that uses Q_PROPERTY for valueOfLHS and assignValue [SWE-5950]
* Added AVExpressionParser::registerOperator to register custom AVExpression operators [DF-240]
