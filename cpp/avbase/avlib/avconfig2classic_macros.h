///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   Header to facilitate transition to the new config implementation.
 */

#if !defined AVCONFIG2CLASSIC_MACROS_H_INCLUDED
#define AVCONFIG2CLASSIC_MACROS_H_INCLUDED

#undef ADDCE
#undef ADDCEP
#undef ADDCEPT
#undef ADDCEPTT
#undef ADDCETT

// If changing these macros, check that $AVCOMMON_HOME/bin/convert_config_macros.sh is still
// working! This script is the reason why those are macros and not template methods.
// Redundant "setDeprecatedName" calls are removed by convert_config_macros.sh and are here only to facilitate
// implementation of the script.
#define ADDCE(name, ptr, default_val, help) \
registerParameter(name, ptr, help).\
    setSuggestedValue(default_val).setDeprecatedName(name);
#define ADDCEP(name, ptr, default_val, help, cmd_line) \
registerParameter(name, ptr, help).\
    setSuggestedValue(default_val).\
    setCmdlineSwitch(cmd_line).setDeprecatedName(name);
#define ADDCEPT(name, ptr, default_val, help, cmd_line, max) \
registerParameter(name, ptr, help).\
    setMax(max).\
    setSuggestedValue(default_val).\
    setCmdlineSwitch(cmd_line).setDeprecatedName(name);
#define ADDCEPTT(name, ptr, default_val, help, cmd_line, min, max) \
registerParameter(name, ptr, help).\
    setMinMax(min, max).\
    setSuggestedValue(default_val).\
    setCmdlineSwitch(cmd_line).setDeprecatedName(name);
#define ADDCETT(name, ptr, default_val, help, min, max) \
registerParameter(name, ptr, help).\
    setMinMax(min, max).\
    setSuggestedValue(default_val).setDeprecatedName(name);

#endif

// End of file
