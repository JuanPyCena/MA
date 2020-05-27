//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2018
//
// Module:    AVLIB - AviBit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   AVVersionReader implementation
 */

// class header include
#include "avenvironment.h"
#include "avpackageinfo.h"
#include "avpath.h"
#include "avversionreader.h"

// system/Qt includes

// common includes

// local includes

const QString AVVersionReader::VERSION_FILENAME = "VERSION";

//-----------------------------------------------------------------------------

QString AVVersionReader::readVersion(const QString &fallback)
{
    QString fallback_version = fallback.isNull() ? AVPackageInfo::getVersion() : fallback;

    QFileInfo path(QDir(AVEnvironment::getApplicationHome()), VERSION_FILENAME);
    AVLogDebug << Q_FUNC_INFO << ": Reading versioninfo from << " << path.absoluteFilePath();

    QFile file(path.absoluteFilePath());
    if (!file.open(QFile::ReadOnly))
    {
        AVLogError << Q_FUNC_INFO << ": could not open " << path.absoluteFilePath()
                   << " (" << file.errorString() << ").";
        return fallback_version;
    }
    QByteArray first_line = file.readLine().trimmed();

    AVLogDebug << Q_FUNC_INFO << ": Read version: " << first_line;

    return first_line.isEmpty() ? fallback_version : first_line;
}


// End of file
