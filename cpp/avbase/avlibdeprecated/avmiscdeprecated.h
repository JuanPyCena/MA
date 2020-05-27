///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright: AviBit data processing GmbH, 2001-2016
//
// Module:    AVLIBDEPRECATED - Deprecated Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Dr. Thomas Leitner, t.leitner@avibit.com
    \author  QT4-PORT: Alexander Randeu, a.randeu@avibit.com
    \brief   Definition of miscellaneous helper classes and functions.
*/

#if !defined(AVMISCDEPRECATED_H_INCLUDED)
#define AVMISCDEPRECATED_H_INCLUDED

// System includes

// QT includes
#include <QString>

// AV includes
#include "avlibdeprecated_export.h"

// forward declarations
class AVSocketDevice;
class QString;

///////////////////////////////////////////////////////////////////////////////
//! Detects if a client has disconnected or an error has occured in the
//! socket device. Returns true if there was no error and the client is still
//! connected. False otherwise.
/*! \note this function checks sd.bytesAvailable(), sd.error() and
          hasClientDisconnected(sd).
          For log output the parameter peerport, peeraddr and port are used,
          because if the client is not connected any more, we cannot get this
          values from sd.
 */
AVLIBDEPRECATED_EXPORT bool checkClientConnection(AVSocketDevice &sd, const QString &peeraddr,
                                                  int peerport, int port,
                                                  const QString &class_name = "",
                                                  bool server_mode = false);

///////////////////////////////////////////////////////////////////////////////
//! Function returning a continuous heading (i.e. may be >360) depending on a track's last heading
AVLIBDEPRECATED_EXPORT double head2cont ( double new_heading, double last_heading );

///////////////////////////////////////////////////////////////////////////////
//! Function returning the difference/angle between two headings.
AVLIBDEPRECATED_EXPORT double dheading ( double old_heading, double new_heading );

#endif

// End of file
