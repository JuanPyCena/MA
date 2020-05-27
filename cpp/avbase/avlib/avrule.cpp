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
    \author  Wolfgang Eder, w.eder@avibit.com
    \author  Alexander Terbu, a.terbu@avibit.com
    \author  QT4-PORT: Peter Draxler, p.draxler@avibit.com
    \brief   This file defines the class that represents a configuration rule.
*/

// AviBit common includes
#include "avconfig2.h"
#include "avconfig2interfaces.h"
#include "avconfig2metadata.h"
#include "avlog.h"
#include "avmisc.h"
#include "avrule.h"

AVExpressionParser& AVRule::getParser()
{
    static AVExpressionParser parser;
    return parser;
}

///////////////////////////////////////////////////////////////////////////////

ForEachList::ForEachList(
    AVDomVariable *parent, const QString& tagName, bool autoCreate, const QString& sep,
    const QString& eachName)
    : AVDomStringListSep(parent, tagName, autoCreate, sep), m_eachName(eachName)
{
}

/////////////////////////////////////////////////////////////////////////////

void ForEachList::setEachName(const QString& name)
{
    m_eachName = name;
    setChanged(true);
    element().setAttribute("name", m_eachName);
}

/////////////////////////////////////////////////////////////////////////////

void ForEachList::computeValue(const QString& text, QDomElement& element)
{
    m_eachName = element.attribute("name", m_eachName);
    AVDomStringListSep::computeValue(text, element);
}

///////////////////////////////////////////////////////////////////////////////

AVRule::AVRule(AVDomVariable *parent, const QString& tagName, bool autoCreate)
    : AVDomObject(parent, tagName, autoCreate),
    description(this, "description", false),
    forEachList(this, "forEach",     false, ",", "each"),
    ifString   (this, "if",          true,  SLOT(slotComputeIfExpression())),
    thenString (this, "then",        false, SLOT(slotComputeThenExpression())),
    elseString (this, "else",        false, SLOT(slotComputeElseExpression())),
    thenRules  (this, "thenIf",      false, tagName),
    elseRules  (this, "elseIf",      false, tagName)
{
}

///////////////////////////////////////////////////////////////////////////////

AVRule::~AVRule()
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVRule::isIfValid() const
{
    return (ifString.isEmpty() || m_ifExpression != 0);
}

///////////////////////////////////////////////////////////////////////////////

bool AVRule::isThenValid() const
{
    return (thenString.isEmpty() || m_thenExpression != 0);
}

///////////////////////////////////////////////////////////////////////////////

bool AVRule::isElseValid() const
{
    return (elseString.isEmpty() || m_elseExpression != 0);
}

///////////////////////////////////////////////////////////////////////////////

void AVRule::slotComputeIfExpression()
{
    if (ifString.isEmpty()) m_ifExpression = 0;
    else {
        QString space = ifString.element().attribute("space");
        m_ifExpression = getParser().parseExpression(ifString.text(), space);
        if (m_ifExpression != 0 && !m_ifExpression->isEvaluateExpression())
            AVLogErrorTo(LOGGER_AVCOMMON_AVLIB_EXPR) << "AVRule: invalid if " << ifString.text();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVRule::slotComputeThenExpression()
{
    if (thenString.isEmpty()) m_thenExpression = 0;
    else {
        QString space = thenString.element().attribute("space");
        m_thenExpression = getParser().parseExpression(thenString.text(), space);
        if (m_thenExpression != 0 && !m_thenExpression->isAssignmentExpression())
            AVLogErrorTo(LOGGER_AVCOMMON_AVLIB_EXPR) << "AVRule: invalid then " << thenString.text();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVRule::slotComputeElseExpression()
{
    if (elseString.isEmpty()) m_elseExpression = 0;
    else {
        QString space = elseString.element().attribute("space");
        m_elseExpression = getParser().parseExpression(elseString.text(), space);
        if (m_elseExpression != 0 && !m_elseExpression->isAssignmentExpression())
            AVLogErrorTo(LOGGER_AVCOMMON_AVLIB_EXPR) << "AVRule: invalid else " << thenString.text();
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVRule::applyRule(AVComputable& c) const
{
    AVLogComputable lc(c);
    ForEachMap forEachMap;
    return applyRuleWithLog(lc, forEachMap);
}

///////////////////////////////////////////////////////////////////////////////

bool AVRule::applyRuleWithLog(AVLogComputable& c, const ForEachMap& forEachMap) const
{
    if (forEachList.isEmpty()) return doApplyRule(c, forEachMap);
    QString name = forEachList.eachName();
    if (forEachMap.contains(name)) {
        LOGGER_AVCOMMON_AVLIB_EXPR.Write(
            LOG_ERROR, "cannot process nested forEach lists with the same name: %s", name.toLocal8Bit().constData());
        return false;
    }
    QStringList list = forEachList.value();
    return applyRuleWithLog(list, c, forEachMap);
}

///////////////////////////////////////////////////////////////////////////////

bool AVRule::applyRuleWithLog(
    const QStringList& list, AVLogComputable& c, const ForEachMap& forEachMap) const
{
    AVASSERT(!forEachList.isEmpty());
    bool result = true;
    Q_FOREACH(QString value, list) {
        AVASSERT(!value.isEmpty());
        if (!applyRuleWithLog(value, c, forEachMap)) result = false;
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVRule::applyRuleWithLog(
    const QString& value, AVLogComputable& c, const ForEachMap& forEachMap) const
{
    AVASSERT(!forEachList.isEmpty());
    if (value[0] == '@')
    {
        // expand to config or listValueOfLHS() computation
        QStringList list;

        const QString CONFIG_PREFIX = "@config.";
        if (value.startsWith(CONFIG_PREFIX))
        {
            QString remaining = value.mid(CONFIG_PREFIX.length());

            const AVConfig2ImporterClient::LoadedParameter *param =
                AVConfig2Global::singleton().getLoadedParameter(remaining);
            if (param == nullptr)
            {
                AVLogError << "AVRule::applyRuleWithLog: config parameter not found " << remaining;
                return false;
            }
            QString type = param->getMetadata().m_type;
            if (type != AVConfig2Types::getTypeName<QStringList>())
            {
                AVLogError << "AVRule::applyRuleWithLog: only QStringList parameters are supported in forEach";
                return false;
            }
            if (!AVFromString(param->getValue(), list))
            {
                AVLogError << "AVRule::applyRuleWithLog: QStringList parse error: " << param->getValue();
                return false;
            }
        } else
        {
            list = c.listValueOfLHS(value.mid(1));
        }

        // TODO Wenn listValueOfLHS nichts findet, könnten wir folgendes machen:
        //   QString str = c.valueOfLHS(value.mid(1));
        //   list = QStringList::split(forEachList.sep, str);
        // Alternativ könnte das die default Implementierung von listValueOfLHS
        // machen, die braucht dann halt den sep als Parameter.

        return applyRuleWithLog(list, c, forEachMap);
    }
    QString name = forEachList.eachName();
    ForEachMap newMap(forEachMap);
    newMap.insert(name, value);
    return doApplyRule(c, newMap);
}

///////////////////////////////////////////////////////////////////////////////

bool AVRule::doApplyRule(AVLogComputable& c, const ForEachMap& forEachMap) const
{
    if (m_ifExpression == 0 || !m_ifExpression->isEvaluateExpression()) {
        if (c.debugRules) {
            QString str = replaceEach(ifString.text(), forEachMap);
            LOGGER_AVCOMMON_AVLIB_EXPR.Write(LOG_ERROR, "AVRule: invalid if %s", str.toLocal8Bit().constData());
        }
        return false;
    }
    if (c.debugRules) {
        QString str = replaceEach(ifString.text(), forEachMap);
        LOGGER_AVCOMMON_AVLIB_EXPR.Write(LOG_INFO, "AVRule: if %s", str.toLocal8Bit().constData());
    }
    bool value = m_ifExpression->evaluateExpression(c, forEachMap);
    bool result = value
        ? doApplyRule("then", thenString.text(), m_thenExpression.get(), thenRules, c, forEachMap)
        : doApplyRule("else", elseString.text(), m_elseExpression.get(), elseRules, c, forEachMap);
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVRule::doApplyRule(
    const char *thenOrElse, const QString& expressionString, const AVExpression *expression,
    const AVRuleList& nestedRules, AVLogComputable& c, const ForEachMap& forEachMap) const
{
    bool result = true;
    if (!expressionString.isEmpty()) {
        if (expression == 0 || !expression->isAssignmentExpression()) {
            if (c.debugRules) {
                QString str = replaceEach(expressionString, forEachMap);
                LOGGER_AVCOMMON_AVLIB_EXPR.Write(
                    LOG_ERROR, "AVRule: invalid %s %s", thenOrElse, str.toLocal8Bit().constData());
            }
            result = false;
        } else {
            if (c.debugRules) {
                QString str = replaceEach(expressionString, forEachMap);
                LOGGER_AVCOMMON_AVLIB_EXPR.Write(
                    LOG_INFO, "AVRule:   %s %s", thenOrElse, str.toLocal8Bit().constData());
            }
            c.clearLogMessage();
            result = expression->performAssignments(c, forEachMap);
            if (c.debugRules && !result)
                LOGGER_AVCOMMON_AVLIB_EXPR.Write(LOG_WARNING, "performAssignments failed");
            c.writeLogMessage();
        }
    }
    if (!nestedRules.applyRulesWithLog(c, forEachMap)) result = false;
    return result;
}

///////////////////////////////////////////////////////////////////////////////

AVRuleList::AVRuleList(AVDomVariable *parent, const QString& tagName, bool autoCreate, const QString& elementTagName)
    : AVDomList<AVRule>(parent, tagName, autoCreate, elementTagName)
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVRuleList::applyRules(AVComputable& c) const
{
    AVLogComputable lc(c);
    ForEachMap forEachMap;
    return applyRulesWithLog(lc, forEachMap);
}

///////////////////////////////////////////////////////////////////////////////

bool AVRuleList::applyRulesWithLog(AVLogComputable& c, const ForEachMap& forEachMap) const
{
    bool result = true;
    for(const AVRule* rule : list()) {
        if (!rule->applyRuleWithLog(c, forEachMap)) result = false;
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////

AVLogComputable::AVLogComputable(AVComputable& computable)
    : debugRules(false), m_computable(computable), m_logLevel(AVLog::LOG__OFF)
{
}

///////////////////////////////////////////////////////////////////////////////

void AVLogComputable::clearLogMessage()
{
    m_logLevel = AVLog::LOG__OFF;
    m_logMessage = QString::null;
}

///////////////////////////////////////////////////////////////////////////////

void AVLogComputable::writeLogMessage()
{
    if (m_logLevel != AVLog::LOG__OFF)
        LOGGER_AVCOMMON_AVLIB_EXPR.Write(LOG_HERE, m_logLevel, "%s", m_logMessage.toLocal8Bit().constData());
}

///////////////////////////////////////////////////////////////////////////////

void AVLogComputable::initializeTemps(const QStringList& tempNames)
{
    m_temps.clear();
    addTemps(tempNames);
}

///////////////////////////////////////////////////////////////////////////////

void AVLogComputable::addTemps(const QStringList& tempNames)
{
    QStringList::ConstIterator it = tempNames.begin();
    QStringList::ConstIterator end = tempNames.end();
    while (it != end) {
        QString tempName = *it;
        AVASSERT(!tempName.contains('.'));
        if (m_temps.contains(tempName))
            AVLogFatal << "duplicate temp " << tempName;
        m_temps.insert(tempName, EmptyQString);
        ++it;
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVLogComputable::valueOfLHS(const AVHashString& lhs) const
{
    static const QString trueString = "true";
    if (lhs == "alwaysTrue") return trueString;
    int pos = lhs.find('.');
    if (pos != -1) {
        QString name = lhs.left(pos).qstring();

        // "config.<param>" resolves to a config parameter
        if (name == "config") {
            QString remaining = lhs.mid(pos + 1).qstring();
            const AVConfig2ImporterClient::LoadedParameter *param =
                AVConfig2Global::singleton().getLoadedParameter(remaining);
            if (param == 0) {
                AVLogError << "parameter not found " << remaining;
                return QString::null;
            }
            QString type = param->getMetadata().m_type;
            QString result;
            if (type == "QString") {
                AVFromString(param->getValue(), result);
                if (result.isNull()) result = "null"; // this is the standard representation for rules
            } else if (type == "int") {
                int value;
                if (!AVFromString(param->getValue(), value)) {
                    AVLogError << "parameter " << remaining << " cannot be parsed: " << param->getValue();
                    return QString::null;
                }
                result = QString::number(value);
            } else if (type == "uint") {
                uint value;
                if (!AVFromString(param->getValue(), value)) {
                    AVLogError << "parameter " << remaining << " cannot be parsed: " << param->getValue();
                    return QString::null;
                }
                result = QString::number(value);
            } else if (type == "bool") {
                bool value;
                if (!AVFromString(param->getValue(), value)) {
                    AVLogError << "parameter " << remaining << " cannot be parsed: " << param->getValue();
                    return QString::null;
                }
                result = boolStr(value);
            } else {
                AVLogError << "parameter " << remaining << " type " << type << " not supported";
                return QString::null;
            }
            if (debugRules) {
                AVLogInfo << "parameter " << remaining << " evaluates to " << result;
            }
            return result;
        }

        // replace "TEMP.remaining" with "<contents of temp>.remaining"

        Temps::ConstIterator it = m_temps.find(name);
        if (it != m_temps.end()) {
            AVHashString newLHS(*it + lhs.mid(pos).qstring());
            return m_computable.valueOfLHS(newLHS);
        }
    } else {
        Temps::ConstIterator it = m_temps.find(lhs.qstring());
        if (it != m_temps.end()) return *it;
    }
    return m_computable.valueOfLHS(lhs);
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVLogComputable::listValueOfLHS(const QString& lhs) const
{
    if (lhs == "debugRules") return QStringList(boolStr(debugRules));
    return m_computable.listValueOfLHS(lhs);
}

///////////////////////////////////////////////////////////////////////////////

QString AVLogComputable::valueOfRHS(const AVHashString& rhs) const
{
    // Usually valueOfRHS does not need to be overridden. This is only
    // done here to make the unit tests work, because in the unit tests, the
    // TestComputable records all calls to valueOfRHS.

    if (rhs.left(1) == "@") return valueOfLHS(rhs.mid(1));
    //return rhs;
    return m_computable.valueOfRHS(rhs);
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogComputable::assignValue(const AVHashString& lhs, const QString& value)
{
    if (lhs == "debugRules") {
        debugRules = asBool(value);
        return true;
    }
    if (lhs == "logfatal") {
        m_logLevel = AVLog::LOG__FATAL;
        m_logMessage = value;
        return true;
    }
    if (lhs == "logerror") {
        m_logLevel = AVLog::LOG__ERROR;
        m_logMessage = value;
        return true;
    }
    if (lhs == "logwarning") {
        m_logLevel = AVLog::LOG__WARNING;
        m_logMessage = value;
        return true;
    }
    if (lhs == "loginfo") {
        m_logLevel = AVLog::LOG__INFO;
        m_logMessage = value;
        return true;
    }
    if (lhs == "logdebug") {
        m_logLevel = AVLog::LOG__DEBUG;
        m_logMessage = value;
        return true;
    }
    if (lhs == "arg") {
        m_logMessage = m_logMessage.arg(value);
        return true;
    }
    int pos = lhs.find('.');
    if (pos != -1) {

        // replace "TEMP.remaining" with "<contents of temp>.remaining"

        QString name = lhs.left(pos).qstring();
        Temps::ConstIterator it = m_temps.find(name);
        if (it != m_temps.end()) {
            AVHashString newLHS(*it + lhs.mid(pos).qstring());
            return m_computable.assignValue(newLHS, value);
        }
    } else {
        Temps::Iterator it = m_temps.find(lhs.qstring());
        if (it != m_temps.end()) {
            if (value.isNull()) return false;
            *it = value;
            return true;
        }
    }
    return m_computable.assignValue(lhs, value);
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogComputable::handleEvaluateEquals(
    const AVHashString& lhs, const AVHashString& rhs, bool& result) const
{
    return m_computable.handleEvaluateEquals(lhs, rhs, result);
}

///////////////////////////////////////////////////////////////////////////////

AVComputable &AVLogComputable::computable()
{
    return m_computable;
}

///////////////////////////////////////////////////////////////////////////////

const AVComputable &AVLogComputable::computable() const
{
    return m_computable;
}

// End of file
