///////////////////////////////////////////////////////////////////////////////
//
// Package:   ADMAX - Avibit AMAN/DMAN Libraries
// Copyright: AviBit data processing GmbH, 2001-2014
// QT-Version: QT5
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Matthias Fuchs, m.fuchs@avibit.com
    \author  QT5-PORT: Matthias Fuchs, m.fuchs@avibit.com
    \brief   Class to check IP addresses.
 */

#ifndef AVIPADDRESS_H
#define AVIPADDRESS_H

// Qt includes
#include <QString>

#include "avlib_export.h"

///////////////////////////////////////////////////////////////////////////////

//! Class to check IP addresses, is IPv4 and IPv6 capable.
class AVLIB_EXPORT AVIPAddress
{
public:

    explicit AVIPAddress();

    //! Checks whether an IP address is valid, works for IPv4 and IPv6
    /*!
        \param address IP address coded as a string. e.g. "204.152.189.116" or "::1"
        \return true if the address is a valid IP address, false otherwise
    */
    static bool isValid(const QString &address);

    //! Checks whether an IP string is within multicast range, works for IPv4 and IPv6
    /*!
        \param address IP address coded as a string. e.g. "224.0.1.1" or "ffxx:..."
        \return true if within a multicast range (i.e. 224.0.0.0 - 239.255.255.255 or "ffxx:..."), false otherwise
    */
    static bool isMulticast(const QString &address);
};

#endif

// end of file
