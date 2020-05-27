///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIBDEPRECATED - Deprecated Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Dr. Thomas Leitner, t.leitner@avibit.com
    \author  QT4-PORT: Alexander Randeu, a.randeu@avibit.com
    \brief   Implementation of miscellaneous helper classes and functions.
*/

// local includes
#include "avmiscdeprecated.h"

// system includes

// qt includes

// avlib includes
#include "avmisc.h"
#include "avsocketdevice.h"

/////////////////////////////////////////////////////////////////////////////

bool checkClientConnection(AVSocketDevice &sd, const QString &peeraddr,
                           int peerport, int port, const QString &class_name,
                           bool server_mode)
{
    QString server_client = server_mode?"server":"client";

    int bavailable = sd.bytesAvailable();
    if (bavailable < 0)
    {
        AVLogger->Write(LOG_WARNING, "%s::checkClientConnection: "
                        "I/O error (sd.bytesAvailable()) %s=%s/%d "
                        "on port=%d, bytes_available=%d",
                        qPrintable(class_name), qPrintable(server_client),
                        qPrintable(peeraddr), peerport, port, bavailable);
        return false;
    }

    if (sd.hasError())
    {
        AVLogger->Write(LOG_WARNING, "%s::checkClientConnection: "
                        "I/O error (sd.error()) %s=%s/%d on port=%d, "
                        "err=%d", qPrintable(class_name), qPrintable(server_client),
                        qPrintable(peeraddr), peerport, port, sd.error());
        return false;
    }

    if (hasClientDisconnected(sd))
    {
        AVLogger->Write(LOG_INFO, "%s::checkClientConnection: "
                        "%s= %s/%d on port=%d disconnected",
                        qPrintable(class_name), qPrintable(server_client),
                        qPrintable(peeraddr), peerport, port);
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

double head2cont(double new_heading, double last_heading)
{
    double continuous_heading = new_heading;

    double dheading = new_heading-last_heading;
    if (fabs(dheading) > 180.0)
    {
        continuous_heading = new_heading - 360.0 * floor(dheading/360.0 + 0.5);
        //AVLogger->Write(LOG_INFO,"head2cont : %.2f to %.2f",new_heading,continuous_heading);
    }

    return (continuous_heading);
}

///////////////////////////////////////////////////////////////////////////////

double cont2head(double continuous_heading)
{
    double heading = continuous_heading;

    if (continuous_heading >= 360.0)
    {
        heading = fmod(continuous_heading,360.0);
        //AVLogger->Write(LOG_INFO,"cont2head : %.2f to %.2f",continuous_heading,heading);
    }
    else if (continuous_heading < 0.0)
    {
        heading = fmod(continuous_heading,360.0) + 360.0;
        //AVLogger->Write(LOG_INFO,"cont2head : %.2f to %.2f",continuous_heading,heading);
    }

    return (heading);
}

///////////////////////////////////////////////////////////////////////////////

double dheading ( double old_heading, double new_heading )
{
    double h1 = cont2head(old_heading);
    double h2 = cont2head(new_heading);

    double dh = h2 - h1;
    if (fabs(dh) > 180.0)
    {
        dh = double(AVsign(dh)) * ( fabs(dh) - 360.0 );
    }

    return (dh);
}

///////////////////////////////////////////////////////////////////////////////

// End of file
