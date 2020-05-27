///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON
// Copyright: AviBit data processing GmbH, 2001-2014
// QT-Version: QT5
//
// Module:    AVLIB
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   AVPackageInfo implementation
 */

// Qt includes

// avlib includes


// local includes
#include "avpackageinfo.h"

//-----------------------------------------------------------------------------

AVPackageInfo::AVPackageInfo()
{
}

//-----------------------------------------------------------------------------

AVPackageInfo::~AVPackageInfo()
{
}

//-----------------------------------------------------------------------------

QString AVPackageInfo::getName()
{
    if(!isSingletonInitialized())
        return QString();
    return singleton().m_name;
}

//-----------------------------------------------------------------------------

QString AVPackageInfo::getVersion()
{
    if(!isSingletonInitialized())
        return QString();
    return singleton().m_version;
}

//-----------------------------------------------------------------------------

QString AVPackageInfo::getRevision()
{
    if(!isSingletonInitialized())
        return QString();
    return singleton().m_revision;
}

//-----------------------------------------------------------------------------

QString AVPackageInfo::getBuildKey()
{
    if(!isSingletonInitialized())
        return QString();
    return singleton().m_build_key;
}

//-----------------------------------------------------------------------------

void AVPackageInfo::setName(const QString& package_name)
{
    m_name = package_name;
}

//-----------------------------------------------------------------------------

void AVPackageInfo::setVersion(const QString& package_version)
{
    m_version = package_version;
}

//-----------------------------------------------------------------------------

void AVPackageInfo::setRevision(const QString& package_revision)
{
    m_revision = package_revision;
}

//-----------------------------------------------------------------------------

void AVPackageInfo::setBuildKey(const QString& package_build_key)
{
    m_build_key = package_build_key;
}

//-----------------------------------------------------------------------------

// End of file
