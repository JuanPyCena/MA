///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// Copyright:  AviBit data processing GmbH, 2001-2010
// QT-Version: QT4
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author   Wolfgang Eder, w.eder@avibit.com
    \author   QT4-PORT: Peter Draxler, p.draxler@avibit.com
    \brief    This file defines the AVRuleConfig class.
*/


// AviBit common includes
#include "avlog.h"
#include "avruleconfig.h"


///////////////////////////////////////////////////////////////////////////////

AVRuleConfig::AVRuleConfig(
    const QString& fileName, const QString& rootTag, const QString& rulesTag,
    const QString& ruleTag)
    : AVDomConfig(rootTag, true),
    debugRules  (this, "debugRules",   true, nullptr, "false"),
    tempNames   (this, "temps",        true, " "),
    rules       (this, rulesTag,       true, ruleTag),
    includeFiles(this, "includeFiles", true, "fileName"),
    m_parent_debug_rules(false)
{
    AVDomConfig::setFileName(fileName);
}

///////////////////////////////////////////////////////////////////////////////

bool AVRuleConfig::applyRules(AVComputable& c) const
{
    AVLogComputable lc(c);
    lc.debugRules = getDebugRules();
    lc.initializeTemps(tempNames.value());
    ForEachMap forEachMap;
    return applyRulesWithLog(lc, forEachMap);
}

///////////////////////////////////////////////////////////////////////////////

bool AVRuleConfig::applyRulesWithLog(AVLogComputable& lc, const ForEachMap& forEachMap) const
{
    bool result = rules.applyRulesWithLog(lc, forEachMap);
    for (const auto& nestedConfig : nestedConfigs) {
        lc.debugRules = (nestedConfig->debugRules.text() == "parent" ? getDebugRules()
                                                                     : nestedConfig->debugRules.text() == "true");
        lc.addTemps(nestedConfig->tempNames.value());
        result &= nestedConfig->applyRulesWithLog(lc, forEachMap);
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////

void AVRuleConfig::readFromElement(QDomElement& element)
{
    AVDomConfig::readFromElement(element);
    for (const QString &file_name : includeFiles.value())
    {
        auto nestedConfig = std::make_unique<AVRuleConfig>(file_name, tagName(), rules.tagName(), rules.elementTagName());
        AVASSERT(nestedConfig != 0);
        nestedConfig->readFromFile();
        if (nestedConfig->debugRules.text() == "parent")
            nestedConfig->m_parent_debug_rules = getDebugRules();
        nestedConfigs.push_back(std::move(nestedConfig));
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVRuleConfig::getDebugRules() const
{
    return (debugRules.text() == "parent" ? m_parent_debug_rules : debugRules.text() == "true");
}

// End of file
