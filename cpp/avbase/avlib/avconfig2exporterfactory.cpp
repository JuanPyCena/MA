///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright:  AviBit data processing GmbH, 2001-2019
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

#include "avconfig2exporterfactory.h"

#include <memory>

#include "avconfig2classic.h"
#include "avconfig2cstyle.h"
#include "avconfig2xml.h"
#include "avlog.h"
#include "avenvstring.h"

///////////////////////////////////////////////////////////////////////////////

const QString AVConfig2ExporterFactory::AVCONFIG2_PIN_CSTYLE_VERSION = "AVCONFIG2_PIN_CSTYLE_VERSION";

///////////////////////////////////////////////////////////////////////////////

AVConfig2ExporterFactory::AVConfig2ExporterFactory() :
    m_pinned_cstyle_version(AVConfig2ImporterCstyle::V_CURRENT)
{
    QString pin_version_env = AVEnvironment::getEnv(AVCONFIG2_PIN_CSTYLE_VERSION, false, AVEnvironment::NoPrefix);
    if (!pin_version_env.isEmpty())
    {
        bool ok;
        int pin_version = pin_version_env.toInt(&ok);
        if (ok && pin_version > 0 && pin_version <= AVConfig2ImporterCstyle::V_CURRENT)
        {
            m_pinned_cstyle_version = pin_version;
        }
        else
        {
            AVLogError << AV_FUNC_PREAMBLE << "Invalid version number in " << AVCONFIG2_PIN_CSTYLE_VERSION
                       << " environment variable: " << pin_version_env;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterFactory::setPinnedCStyleVersion(int version)
{
    AVASSERT(version >= AVConfig2ImporterCstyle::V_2);
    m_pinned_cstyle_version = version;
}

///////////////////////////////////////////////////////////////////////////////

int AVConfig2ExporterFactory::getPinnedCStyleVersion() const
{
    return m_pinned_cstyle_version;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ExporterBase *AVConfig2ExporterFactory::createExporter(AVConfig2Format format, const QString& name) const
{
    switch (format)
    {
    case CF_CLASSIC:
        return new (LOG_HERE) AVConfig2ExporterClassic(name);
    case CF_CLASSIC_MAPPING:
        return new (LOG_HERE) AVConfig2ExporterClassicMapping;
    case CF_XML:
        return new (LOG_HERE) AVConfig2ExporterXml;
    case CF_CSTYLE:
    {
        AVConfig2ExporterCstyle *exporter = new (LOG_HERE) AVConfig2ExporterCstyle;
        exporter->setPinnedVersion(m_pinned_cstyle_version);
        return exporter;
    }
    case CF_LAST:
        AVLogger->Write(LOG_FATAL, "AVConfig2ExporterBase::createExporter: Unknown format %d",
                        format);
    }
    return nullptr;
}

// End of file
