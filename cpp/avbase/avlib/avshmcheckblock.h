///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
//////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Alexander Wemmer, a.wemmer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Checks a given SHM  block variable for it's value. If it
    says block, the doBlock() methods returns true, false otherwise.
*/

#include <avprocessstate.h>
#include <avprocessstatedesc.h>

#ifndef __AVSHMCHECKBLOCK_H__
#define __AVSHMCHECKBLOCK_H__

#include "avlib_export.h"

//!
/*! Checks the given SHM variable of the given process for a "block" value
 *  (0=no block, 1=block).
 * Used by DAQ_xxx processes to look if they should block the input or not.
 */
class AVLIB_EXPORT AVSHMCheckBlock
{
public:

    //! Standard Constructor
    explicit AVSHMCheckBlock(const QString& shm_var_name, const QString& proc_name = "");

    //! Destructor
    virtual ~AVSHMCheckBlock() {};

    //! returns true, if the SHM block variable was set to block (=1), false
    //! otherwise. Inits the SHM variable to 0 if it does not exist.
    bool doBlock();

private:

    //! the SHM process name to check for blocking
    QString m_proc_name;

    //! the SHM varname to check for blocking
    QString m_shm_var_name;

    //! the SHM variable value
    AVCVariant m_shm_var_value;

     //! Hidden copy-constructor
    AVSHMCheckBlock(const AVSHMCheckBlock&);
    //! Hidden assignment operator
    const AVSHMCheckBlock& operator = (const AVSHMCheckBlock&);
};

#endif /* __AVSHMCHECKBLOCK_H__ */

// End of file
