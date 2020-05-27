//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2015
//
// Module:    AVLIB - AviBit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \author  Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   AVPackageInfo header
 */

#ifndef AVPACKAGEINFO_INCLUDED
#define AVPACKAGEINFO_INCLUDED

// local includes

// Qt includes
#include <QString>

// avlib includes
#include "avexplicitsingleton.h"
#include "avlib_export.h"

// forward declarations

//-----------------------------------------------------------------------------
//! Holds information about the package and version
/*!
 *  The information is usually set by the AVPackaegInfoLoader class,
 *  which is defined in an automatically cmake-generated code file
 *
*/
class AVLIB_EXPORT AVPackageInfo : public AVExplicitSingleton<AVPackageInfo>
{
public:
    AVPackageInfo();
    ~AVPackageInfo() override;

    static QString getName();
    static QString getVersion();
    static QString getRevision();
    static QString getBuildKey();

    void setName(const QString& package_name);
    void setVersion(const QString& package_version);
    void setRevision(const QString& package_revision);
    void setBuildKey(const QString& package_build_key);

private:

    QString m_name;
    QString m_version;
    QString m_revision;
    QString m_build_key;
};

#endif // AVPACKAGEINFO_INCLUDED

// End of file
