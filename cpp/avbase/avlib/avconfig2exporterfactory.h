///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright:  AviBit data processing GmbH, 2001-2019
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QString>

#include "avlib_export.h"
#include "avmacros.h"
#include "avconfig2cstyle.h"

///////////////////////////////////////////////////////////////////////////////
/**
 * Creates exporters for specified config formats.
 */
class AVLIB_EXPORT AVConfig2ExporterFactory
{
public:
    //! Environment variable to pin cstyle version
    static const QString AVCONFIG2_PIN_CSTYLE_VERSION;

    AVConfig2ExporterFactory();
    virtual ~AVConfig2ExporterFactory() {}

    //! Set pinned version for c style configs.
    //! C style exporter will save configs for the specified version.
    //! For example, if AVConfig2ImporterClient::V_2 is set, exporter is not going to save subconfig templates.
    void setPinnedCStyleVersion(int version);

    //! Returns pinned version.
    int getPinnedCStyleVersion() const;

    //! Factory method for creating an exporter writing the specified format.
    //! \param format One of the possible config2 formats.
    //! \param name The name of the config as in AVConfig2StorageLocation::m_name. Used solely for special classic
    //!             exporter logic, see AVConfig2ExporterClassic::splitSectionAndName.
    //! \return The created exporter. Will assert if the format is invalid.
    virtual AVConfig2ExporterBase *createExporter(AVConfig2Format format, const QString& name) const;

private:
    int m_pinned_cstyle_version;
};

// End of file
