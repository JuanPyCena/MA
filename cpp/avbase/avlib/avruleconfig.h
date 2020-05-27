///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// Copyright:  AviBit data processing GmbH, 2001-2010
// QT-Version: QT4
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author   Wolfgang Eder, w.eder@avibit.com
    \author   Alexander Terbu, a.terbu@avibit.com
    \author   QT4-PORT: Peter Draxler, p.draxler@avibit.com
    \brief    This file declares the AVRuleConfig class.
*/

#ifndef AVRULECONFIG_H
#define AVRULECONFIG_H

// AviBit common includes
#include "avlib_export.h"
#include "avrule.h"

///////////////////////////////////////////////////////////////////////////////

//! This is a configuration class that contains a list of rules.

class AVLIB_EXPORT AVRuleConfig : public AVDomConfig, public AVExpressionNamespace
{
    Q_OBJECT
public:
    AVRuleConfig(
        const QString& fileName, const QString& rootTag, const QString& rulesTag,
        const QString& ruleTag);
    bool applyRules(AVComputable& c) const;
    bool applyRulesWithLog(AVLogComputable& c, const ForEachMap& forEachMap) const;
    //! Read the variable's value from the QDomElement.
    void readFromElement(QDomElement& element) override;

public:
    AVDomString             debugRules;
    AVDomStringListSep      tempNames;
    AVRuleList              rules;
    AVDomStringList         includeFiles;
    std::vector<std::unique_ptr<AVRuleConfig>>     nestedConfigs;
private:
    Q_DISABLE_COPY(AVRuleConfig);
    //! Returns true if rule debugging is enabled, false otherwise.
    bool getDebugRules() const;
    //! True if rule debugging is enabled on the parent, false otherwise.
    bool                   m_parent_debug_rules;
};

#endif // AVRULECONFIG_H

// End of file
