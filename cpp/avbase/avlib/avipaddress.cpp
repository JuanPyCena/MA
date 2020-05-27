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

// Qt includes
#include <QStringList>
#include <QHostAddress>

// local includes
#include "avipaddress.h"

///////////////////////////////////////////////////////////////////////////////

AVIPAddress::AVIPAddress()
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVIPAddress::isValid(const QString& address)
{
    QHostAddress hostAddress;
    return hostAddress.setAddress(address);
}

///////////////////////////////////////////////////////////////////////////////

bool AVIPAddress::isMulticast(const QString& address)
{
    QHostAddress hostAddress;
    hostAddress.setAddress(address);

    if (hostAddress.protocol() == QAbstractSocket::IPv4Protocol)
    {
        uint first_byte = address.left(3).toUInt();
        return (first_byte >= 224 && first_byte <= 239);
    }
    else if (hostAddress.protocol() == QAbstractSocket::IPv6Protocol)
    {
        return (hostAddress.toIPv6Address().c[0] == 0xff);
    }

    return false;
}

// end of file
