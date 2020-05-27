///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author   Wolfgang Eder, w.eder@avibit.com
    \author   QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief    This file defines the classes to represent expressions.
*/

// Qt includes
#include <QRegExp>

// AviBit common includes
#include "avexpressions.h"
#include "avlog.h"
#include "avmisc.h"

std::unique_ptr<AVHashString::ValueHashes> AVHashString::m_valueHashes;
uint AVHashString::m_instances           = 0;
uint AVHashString::m_expressionInstances = 0;
uint AVHashString::m_leftDicts           = 0;
uint AVHashString::m_midDicts            = 0;
uint AVHashString::m_valueHashHit        = 0;
uint AVHashString::m_valueHashMiss       = 0;
uint AVHashString::m_hashHit             = 0;
uint AVHashString::m_hashMiss            = 0;
uint AVHashString::m_hashClash           = 0;

const QString AVComputable::m_trueString  = "true";
const QString AVComputable::m_falseString = "false";

//! TODO CM change this to fatal
AVExpression::FAILURE_MODE AVExpression::m_failureMode = AVExpression::FM_IGNORE;

///////////////////////////////////////////////////////////////////////////////

AVHashString::AVHashString(const QString& name, bool expression)
    : m_name(name), m_expression(expression), m_nameHash(0), m_left(0), m_mid(0)
{
    m_latin1 = m_name.toLocal8Bit();
    if (!m_latin1.isNull()) m_nameHash = computeHash(m_latin1.constData());
    m_instances++;
    if (expression) m_expressionInstances++;
}

///////////////////////////////////////////////////////////////////////////////

AVHashString::~AVHashString()
{
    if (m_left != 0)
        qDeleteAll(*m_left);
    delete m_left;
    if (m_mid != 0)
        qDeleteAll(*m_mid);
    delete m_mid;
}

///////////////////////////////////////////////////////////////////////////////

bool AVHashString::operator==(const char *value) const
{
    //KDAB disabled this optimization.
    //Fdp2OldiTest::testAvHashableString shows a bug when this optimization is activated.

#if 0
    AVASSERT(value != 0);
    if (m_valueHashes.get() == 0) {
        m_valueHashes.reset(new ValueHashes());
        m_valueHashes->reserve(1031); // TODO make configurable
    }
    uint valueHash;
    ValueHashes::iterator iter = m_valueHashes->find(reinterpret_cast<void *>(const_cast<char *>(value)));
    if (iter == m_valueHashes->end()) {
        valueHash = computeHash(value);
        m_valueHashes->insert(reinterpret_cast<void *>(const_cast<char *>(value)), valueHash);
        m_valueHashMiss++;
    } else {
        valueHash = iter.value();
        m_valueHashHit++;
    }
    if (m_nameHash != valueHash) {
        m_hashHit++;
        return false;
    } else {
        m_hashMiss++;
        bool result = (qstrcmp(m_latin1.constData(), value) == 0);
        if (!result) m_hashClash++;
        return result;
    }
#else
    return AVExpression::qstringEqual(m_name, QLatin1String(value));
#endif
}

///////////////////////////////////////////////////////////////////////////////

int AVHashString::find(char c) const
{
    return m_name.indexOf(c);
}

///////////////////////////////////////////////////////////////////////////////

const AVHashString& AVHashString::left(uint len) const
{
    if (m_left == 0) {
        m_left = new LeftMidDict;
        AVASSERT(m_left != 0);
        m_leftDicts++;
    }
    AVHashString *result = m_left->value(len, 0);
    if (result == 0) {
        QString value = m_name.left(len);
        if (value.isNull()) value = EmptyQString;
        result = new AVHashString(value, m_expression);
        AVASSERT(result != 0);
        m_left->insert(len, result);
    }
    return *result;
}

///////////////////////////////////////////////////////////////////////////////

const AVHashString& AVHashString::mid(uint index) const
{
    if (m_mid == 0) {
        m_mid = new LeftMidDict;
        AVASSERT(m_mid != 0);
        m_midDicts++;
    }
    AVHashString *result = m_mid->value(index, 0);
    if (result == 0) {
        QString value = m_name.mid(index);
        if (value.isNull()) value = EmptyQString;
        result = new AVHashString(value, m_expression);
        AVASSERT(result != 0);
        m_mid->insert(index, result);
    }
    return *result;
}

///////////////////////////////////////////////////////////////////////////////

// see QGDict::hashKeyAscii()

uint AVHashString::computeHash(const char *key)
{
    AVASSERT(key != 0);
    const char *k = key;
    uint h = 0;
    uint g;
    while (*k) {
        h = (h << 4) + *k++;
        //parasoft suppress item pbugs-21
        if ((g = h & 0xf0000000)) h ^= g >> 24;
        //parasoft on
        h &= ~g;
    }
    return h;
}

///////////////////////////////////////////////////////////////////////////////

void AVHashString::printStats()
{
    LOGGER_AVCOMMON_AVLIB_EXPR.Write(LOG_INFO, "AVHashString stats");
    double percent = 100.0;
    if (m_instances > 0) percent = m_expressionInstances * 100.0 / m_instances;
    LOGGER_AVCOMMON_AVLIB_EXPR.Write(
        LOG_INFO, "  instances %6.2f%% %7u expr   %7u total",
        percent, m_expressionInstances, m_instances);
    percent = 100.0;
    if (m_instances > 0) percent = (m_leftDicts + m_midDicts) * 100.0 / m_instances;
    LOGGER_AVCOMMON_AVLIB_EXPR.Write(
        LOG_INFO, "            %6.2f%% %7u left   %7u mid",
        percent, m_leftDicts, m_midDicts);
    LOGGER_AVCOMMON_AVLIB_EXPR.Write(
        LOG_INFO, "  valueHash         %7u misses %7u hits %7u total",
        m_valueHashMiss, m_valueHashHit, m_valueHashMiss + m_valueHashHit);
    percent = 100.0;
    if (m_hashMiss + m_hashHit > 0) percent = m_hashMiss * 100.0 / (m_hashMiss + m_hashHit);
    LOGGER_AVCOMMON_AVLIB_EXPR.Write(
        LOG_INFO, "  hash      %6.2f%% %7u misses %7u hits %7u total %7u clashes",
        percent, m_hashMiss, m_hashHit, m_hashMiss + m_hashHit, m_hashClash);
}

///////////////////////////////////////////////////////////////////////////////

template<> QString AVToString  (const AVHashString& arg, bool enable_escape)
{
    return AVToString(arg.qstring(), enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

bool AVComputable::asBool(const QString& string)
{
    if (string == m_trueString)  return true;
    if (string == m_falseString) return false;
    LOGGER_AVCOMMON_AVLIB_EXPR.Write(LOG_FATAL, "asBool invalid string %s", qPrintable(string));
    return false; // will never reach this point
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVComputable::listValueOfLHS(const QString& lhs) const
{
    Q_UNUSED(lhs);
    return EmptyQStringList;
}

///////////////////////////////////////////////////////////////////////////////

QString AVComputable::valueOfRHS(const AVHashString& rhs) const
{
    if (rhs.left(1) == "@") return valueOfLHS(rhs.mid(1));
    return rhs.qstring();
}

///////////////////////////////////////////////////////////////////////////////

bool AVComputable::performAssignment(const AVHashString& lhs, const AVHashString& rhs)
{
    QString value = valueOfRHS(rhs);
    if (AVExpression::getFailureMode() != AVExpression::FM_IGNORE && value.isNull()) {
        AVLogStream stream = AVExpression::getFailureLogstream();
        stream << "performAssignment " << lhs << "=" << rhs << " rhs is null";
    }
    bool result = assignValue(lhs, value);
    if (AVExpression::getFailureMode() != AVExpression::FM_IGNORE && !result) {
        AVLogStream stream = AVExpression::getFailureLogstream();
        stream << "assignValue " << lhs << "=" << rhs << " failed with rhs \"" << value << "\"";
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVComputable::performAppendAssignment(const AVHashString& lhs, const AVHashString& rhs)
{
    QString lhsValue = valueOfLHS(lhs);
    if (AVExpression::getFailureMode() != AVExpression::FM_IGNORE && lhsValue.isNull()) {
        AVLogStream stream = AVExpression::getFailureLogstream();
        stream << "performAppendAssignment " << lhs << "+=" << rhs << " lhs is null";
    }
    QString rhsValue = valueOfRHS(rhs);
    if (AVExpression::getFailureMode() != AVExpression::FM_IGNORE && rhsValue.isNull()) {
        AVLogStream stream = AVExpression::getFailureLogstream();
        stream << "performAppendAssignment " << lhs << "+=" << rhs << " rhs is null";
    }
    QString value = lhsValue + rhsValue;
    bool result = assignValue(lhs, value);
    if (AVExpression::getFailureMode() != AVExpression::FM_IGNORE && !result) {
        AVLogStream stream = AVExpression::getFailureLogstream();
        stream << "assignValue " << lhs << "+=" << rhs << " failed with rhs \"" << value << "\"";
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVComputable::handleEvaluateEquals(const AVHashString& lhs, const AVHashString& rhs, bool& result) const
{
    Q_UNUSED(lhs);
    Q_UNUSED(rhs);
    Q_UNUSED(result);
    return false;
}

///////////////////////////////////////////////////////////////////////////////

AVPropertyComputable::AVPropertyComputable() :
    AVComputable()
{
}

/////////////////////////////////////////////////////////////////////////////

AVPropertyComputable::~AVPropertyComputable()
{
}

/////////////////////////////////////////////////////////////////////////////

QString AVPropertyComputable::valueOfLHS(const AVHashString& lhs) const
{
    QMetaProperty metaProperty = getReadableMetaProperty(lhs);
    if (metaProperty.isValid())
    {
        const QVariant variant = readProperty(metaProperty);

        if (!variant.isValid())
        {
            AVLogWarning << "property " << lhs << " value is not valid";
            return EmptyQString;
        }

        if (variant.isNull())
        {
            AVLogWarning << "property " << lhs << " value is null";
            return EmptyQString;
        }

        return notNull(variant.toString());
    }

    const int pos = lhs.find('.');
    if (pos != -1)
    {
        const AVHashString& name = lhs.left(pos);
        const AVHashString& remaining = lhs.mid(pos + 1);

        metaProperty = getReadableMetaProperty(name);
        if (metaProperty.isValid())
        {
            const QVariant variant = readProperty(metaProperty);

            if (!variant.isValid())
            {
                AVLogWarning << "property " << name << " value is not valid";
                return QStringLiteral("invalid");
            }

            if (variant.isNull())
            {
                AVLogWarning << "property " << name << " value is null";
                return QStringLiteral("null");
            }

            const AVComputable* computable = variant.value<const AVComputable*>();
            if (computable)
                return computable->valueOfLHS(remaining);

            computable = variant.value<AVComputable*>();
            if (computable)
                return computable->valueOfLHS(remaining);

            AVLogWarning << "property " << name << " could not be converted to a AVComputable*";
        }
    }

    return EmptyQString;
}

/////////////////////////////////////////////////////////////////////////////

bool AVPropertyComputable::assignValue(const AVHashString& lhs, const QString& value)
{
    QMetaProperty metaProperty = getWriteableMetaProperty(lhs);
    if (metaProperty.isValid())
    {
        const bool success = writeProperty(metaProperty, value);
        if (!success)
            AVLogError << "property " << lhs << " could not be set!";
        return success;
    }

    const int pos = lhs.find('.');
    if (pos != -1)
    {
        const AVHashString& name = lhs.left(pos);
        const AVHashString& remaining = lhs.mid(pos + 1);

        metaProperty = getReadableMetaProperty(name);
        if (metaProperty.isValid())
        {
            const QVariant variant = readProperty(metaProperty);

            if (!variant.isValid())
            {
                AVLogWarning << "property " << name << " value is not valid";
                return false;
            }

            if (variant.isNull())
            {
                AVLogWarning << "property " << name << " value is null";
                return false;
            }

            AVComputable* const computable = variant.value<AVComputable*>();
            if (computable)
                return computable->assignValue(remaining, value);

            AVLogWarning << "property " << name << " could not be converted to a AVComputable*";
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////

QMetaProperty AVPropertyComputable::getMetaProperty(const AVHashString &name) const
{
    const QMetaObject* const meta = metaObject();

    const int id = meta->indexOfProperty(name.ascii());
    if (id == -1)
    {
        AVLogError << "property " << name << " could not be found";
        return QMetaProperty{};
    }

    const QMetaProperty metaProperty = meta->property(id);
    if (!metaProperty.isValid())
    {
        AVLogError << "property " << name << " is invalid";
        return QMetaProperty{};
    }

    if (!metaProperty.isScriptable())
    {
        AVLogError << "property " << name << " is not scriptable";
        return QMetaProperty{};
    }

    return metaProperty;
}

/////////////////////////////////////////////////////////////////////////////

QMetaProperty AVPropertyComputable::getReadableMetaProperty(const AVHashString &name) const
{
    const QMetaProperty metaProperty = getMetaProperty(name);
    if (!metaProperty.isValid())
        return QMetaProperty{};

    if (!metaProperty.isReadable())
    {
        AVLogError << "property " << name << " is not readable";
        return QMetaProperty{};
    }

    return metaProperty;
}

/////////////////////////////////////////////////////////////////////////////

QMetaProperty AVPropertyComputable::getWriteableMetaProperty(const AVHashString &name) const
{
    const QMetaProperty metaProperty = getMetaProperty(name);
    if (!metaProperty.isValid())
        return QMetaProperty{};

    if (metaProperty.isConstant())
    {
        AVLogError << "property " << name << " is constant";
        return QMetaProperty{};
    }

    if (!metaProperty.isWritable())
    {
        AVLogError << "property " << name << " is not writeable";
        return QMetaProperty{};
    }

    return metaProperty;
}

///////////////////////////////////////////////////////////////////////////////

QVariant AVPropertyComputable::readProperty(const QMetaProperty &metaProperty) const
{
    if (auto ptr = dynamic_cast<const QObject*>(this))
        return metaProperty.read(ptr);
    else
        return metaProperty.readOnGadget(this);
}

///////////////////////////////////////////////////////////////////////////////

bool AVPropertyComputable::writeProperty(const QMetaProperty &metaProperty, const QVariant &value)
{
    if (auto ptr = dynamic_cast<QObject*>(this))
        return metaProperty.write(ptr, value);
    else
        return metaProperty.writeOnGadget(this, value);
}

///////////////////////////////////////////////////////////////////////////////

bool AVExpressionNamespace::containsEach(
    const AVHashString& string, const ForEachMap& forEachMap) const
{
    if (forEachMap.isEmpty()) return false;
    ForEachMap::ConstIterator it = forEachMap.begin();
    ForEachMap::ConstIterator end = forEachMap.end();
    while (it != end) {
        QString name = it.key();
        if (string.qstring().contains(name)) return true;
        ++it;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

QString AVExpressionNamespace::replaceEach(
    const QString& string, const ForEachMap& forEachMap) const
{
    if (forEachMap.isEmpty()) return string;
    QString result = string;
    ForEachMap::ConstIterator it = forEachMap.begin();
    ForEachMap::ConstIterator end = forEachMap.end();
    while (it != end) {
        QString name = it.key();
        QString value = it.value();
        result.replace(name, value);
        ++it;
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVExpression::performAssignments(AVComputable& c, const ForEachMap& forEachMap) const
{
    Q_UNUSED(c);
    Q_UNUSED(forEachMap);
    AVLogErrorTo(LOGGER_AVCOMMON_AVLIB_EXPR)
        << "performAssignments invalid expression " << toString();
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void AVExpression::setFailureMode(FAILURE_MODE mode)
{
    m_failureMode = mode;
}

///////////////////////////////////////////////////////////////////////////////

AVExpression::FAILURE_MODE AVExpression::getFailureMode()
{
    return m_failureMode;
}

///////////////////////////////////////////////////////////////////////////////

AVLogStream AVExpression::getFailureLogstream()
{
    switch (m_failureMode)
    {
        case FM_WARNING:
            if (!LOGGER_AVCOMMON_AVLIB_EXPR.isLevelSuppressed(AVLog::LOG__WARNING))
                return AVLogStream(LOG_HERE, AVLog::LOG__WARNING, &LOGGER_AVCOMMON_AVLIB_EXPR);
        case FM_FATAL:
            return AVLogStream(LOG_HERE, AVLog::LOG__FATAL, &LOGGER_AVCOMMON_AVLIB_EXPR);
        default:
            break;
    }
    return AVLogStream(LOG_HERE, AVLog::LOG__DEBUG, &LOGGER_AVCOMMON_AVLIB_EXPR);
}

///////////////////////////////////////////////////////////////////////////////

bool AVExpression::qstringEqual(const QString &s1, const QString &s2)
{
    if (s1.isNull() != s2.isNull()) return false;
    return s1 == s2;
}

///////////////////////////////////////////////////////////////////////////////

bool AVExpression::qstringEqual(const QString &s1, const QLatin1String &s2)
{
    if (s1.isNull() != (s2.data() == 0)) return false;
    return s1 == s2;
}

///////////////////////////////////////////////////////////////////////////////

const QString& AVExpression::debugArgumentString(const QString& arg)
{
    static const QString nullString = "null";
    static const QString emptyString = "empty";
    if (arg.isNull()) return nullString;
    if (arg.isEmpty()) return emptyString;
    return arg;
}

///////////////////////////////////////////////////////////////////////////////

AVAndExpression::AVAndExpression(std::unique_ptr<AVExpression> lhs, std::unique_ptr<AVExpression> rhs)
    : m_lhs(std::move(lhs)), m_rhs(std::move(rhs))
{
    AVASSERT(m_lhs != 0);
    AVASSERT(m_rhs != 0);
}

///////////////////////////////////////////////////////////////////////////////

AVAndExpression::~AVAndExpression()
{
}

///////////////////////////////////////////////////////////////////////////////

QString AVAndExpression::toString() const
{
    AVASSERT(m_lhs != 0);
    AVASSERT(m_rhs != 0);
    QString result = "";
    bool parens = (m_lhs->type() == Or);
    if (parens) result += "(";
    result += m_lhs->toString();
    if (parens) result += ")";
    result += " and ";
    parens = m_rhs->type() == Or;
    if (parens) result += "(";
    result += m_rhs->toString();
    if (parens) result += ")";
    return result;
}

///////////////////////////////////////////////////////////////////////////////

QString AVAndExpression::debugString(int indent) const
{
    AVASSERT(m_lhs != 0);
    AVASSERT(m_rhs != 0);
    QString result;
    result.fill(' ', indent);
    result += "And\n";
    result += m_lhs->debugString(indent + 1);
    result += m_rhs->debugString(indent + 1);
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVAndExpression::evaluateExpression(const AVComputable& c, const ForEachMap& forEachMap) const
{
    AVASSERT(m_lhs != 0);
    AVASSERT(m_rhs != 0);
    return m_lhs->evaluateExpression(c, forEachMap) && m_rhs->evaluateExpression(c, forEachMap);
}

///////////////////////////////////////////////////////////////////////////////

bool AVAndExpression::performAssignments(AVComputable& c, const ForEachMap& forEachMap) const
{
    AVASSERT(m_lhs != 0);
    AVASSERT(m_rhs != 0);
    bool ok1 = m_lhs->performAssignments(c, forEachMap);
    bool ok2 = m_rhs->performAssignments(c, forEachMap);
    return ok1 && ok2;
}

///////////////////////////////////////////////////////////////////////////////

AVOrExpression::AVOrExpression(std::unique_ptr<AVExpression> lhs, std::unique_ptr<AVExpression> rhs)
    : m_lhs(std::move(lhs)), m_rhs(std::move(rhs))
{
    AVASSERT(m_lhs != 0);
    AVASSERT(m_rhs != 0);
}

///////////////////////////////////////////////////////////////////////////////

AVOrExpression::~AVOrExpression()
{
}

///////////////////////////////////////////////////////////////////////////////

QString AVOrExpression::toString() const
{
    AVASSERT(m_lhs != 0);
    AVASSERT(m_rhs != 0);
    QString result = "";
    bool parens = m_lhs->type() == And;
    if (parens) result += "(";
    result += m_lhs->toString();
    if (parens) result += ")";
    result += " or ";
    parens = m_rhs->type() == And;
    if (parens) result += "(";
    result += m_rhs->toString();
    if (parens) result += ")";
    return result;
}

///////////////////////////////////////////////////////////////////////////////

QString AVOrExpression::debugString(int indent) const
{
    AVASSERT(m_lhs != 0);
    AVASSERT(m_rhs != 0);
    QString result;
    result.fill(' ', indent);
    result += "Or\n";
    result += m_lhs->debugString(indent + 1);
    result += m_rhs->debugString(indent + 1);
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVOrExpression::evaluateExpression(const AVComputable& c, const ForEachMap& forEachMap) const
{
    AVASSERT(m_lhs != 0);
    AVASSERT(m_rhs != 0);
    return m_lhs->evaluateExpression(c, forEachMap) || m_rhs->evaluateExpression(c, forEachMap);
}

///////////////////////////////////////////////////////////////////////////////

QString AVEqualsExpression::toString() const
{
    return (m_lhs.qstring() + "==" + m_rhs.qstring());
}

///////////////////////////////////////////////////////////////////////////////

QString AVEqualsExpression::debugString(int indent) const
{
    QString result;
    result.fill(' ', indent);
    result += debugArgumentString(m_lhs);
    result += " Equals ";
    result += debugArgumentString(m_rhs);
    result += "\n";
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVEqualsExpression::evaluateExpression(const AVComputable& c, const ForEachMap& forEachMap) const
{
    if (m_first) {
        m_needToReplaceEach = (containsEach(m_lhs, forEachMap) || containsEach(m_rhs, forEachMap));
        m_first = false;
    }
    if (m_needToReplaceEach) {
        AVHashString lhs(replaceEach(m_lhs, forEachMap));
        AVHashString rhs(replaceEach(m_rhs, forEachMap));
        bool result;
        if (c.handleEvaluateEquals(lhs, rhs, result)) return result;
        QString lhsValue = c.valueOfLHS(lhs);
        QString rhsValue = c.valueOfRHS(rhs);
        if (m_failureMode != FM_IGNORE && (lhsValue.isNull() || rhsValue.isNull())) {
            AVLogStream stream = AVExpression::getFailureLogstream();
            stream << "evaluateExpression " << lhs << "==" << rhs
                << " failed with lhs \"" << lhsValue << "\" rhs \"" << rhsValue << "\"";
        }
        return AVExpression::qstringEqual(lhsValue, rhsValue);
    } else {
        bool result;
        if (c.handleEvaluateEquals(m_lhs, m_rhs, result)) return result;
        QString lhsValue = c.valueOfLHS(m_lhs);
        QString rhsValue = c.valueOfRHS(m_rhs);
        if (m_failureMode != FM_IGNORE && (lhsValue.isNull() || rhsValue.isNull()))
        {
            AVLogStream stream = AVExpression::getFailureLogstream();
            stream << "evaluateExpression " << m_lhs << "==" << m_rhs
                << " failed with lhs \"" << lhsValue << "\" rhs \"" << rhsValue << "\"";

        }
        return AVExpression::qstringEqual(lhsValue, rhsValue);
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVNotEqualsExpression::toString() const
{
    return (m_lhs.qstring() + "!=" + m_rhs.qstring());
}

///////////////////////////////////////////////////////////////////////////////

QString AVNotEqualsExpression::debugString(int indent) const
{
    QString result;
    result.fill(' ', indent);
    result += debugArgumentString(m_lhs);
    result += " NotEquals ";
    result += debugArgumentString(m_rhs);
    result += "\n";
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVNotEqualsExpression::evaluateExpression(const AVComputable& c, const ForEachMap& forEachMap) const
{
    if (m_first) {
        m_needToReplaceEach = (containsEach(m_lhs, forEachMap) || containsEach(m_rhs, forEachMap));
        m_first = false;
    }
    if (m_needToReplaceEach) {
        AVHashString lhs(replaceEach(m_lhs, forEachMap));
        AVHashString rhs(replaceEach(m_rhs, forEachMap));
        QString lhsValue = c.valueOfLHS(lhs);
        QString rhsValue = c.valueOfRHS(rhs);
        if (m_failureMode != FM_IGNORE && (lhsValue.isNull() || rhsValue.isNull())) {
            AVLogStream stream = AVExpression::getFailureLogstream();
            stream << "evaluateExpression " << lhs << "!=" << rhs
                << " failed with lhs \"" << lhsValue << "\" rhs \"" << rhsValue << "\"";
        }
        return !AVExpression::qstringEqual(lhsValue, rhsValue);
    } else {
        QString lhsValue = c.valueOfLHS(m_lhs);
        QString rhsValue = c.valueOfRHS(m_rhs);
        if (m_failureMode != FM_IGNORE && (lhsValue.isNull() || rhsValue.isNull())) {
            AVLogStream stream = AVExpression::getFailureLogstream();
            stream  << "evaluateExpression " << m_lhs << "!=" << m_rhs
                    << " failed with lhs \"" << lhsValue << "\" rhs \"" << rhsValue << "\"";
        }
        return !AVExpression::qstringEqual(lhsValue, rhsValue);
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVIntExpression::evaluateExpression(const AVComputable& c, const ForEachMap& forEachMap) const
{
    const QChar comma = '.';
    AVHashString lhs(replaceEach(m_lhs, forEachMap));
    AVHashString rhs(replaceEach(m_rhs, forEachMap));
    QString lhsValue = c.valueOfLHS(lhs);
    QString rhsValue = c.valueOfRHS(rhs);
    if (lhsValue.contains(' ')) {
        AVLogErrorTo(LOGGER_AVCOMMON_AVLIB_EXPR)
            << "evaluateExpression LHS contains whitespace " << lhs << "=\"" << lhsValue << "\"";
        return false;
    }
    if (rhsValue.contains(' ')) {
        AVLogErrorTo(LOGGER_AVCOMMON_AVLIB_EXPR)
            << "evaluateExpression RHS contains whitespace " << rhs << "=\"" << rhsValue << "\"";
        return false;
    }
    bool ok = false;
    int lhsInt = lhsValue.toInt(&ok);
    int lhsExponent = 0;
    if (!ok) {
        int pos = lhsValue.indexOf(comma);
        if (pos != -1) {
            lhsExponent = lhsValue.length() - pos - 1;
            lhsValue.remove(pos, 1);
            lhsInt = lhsValue.toInt(&ok);
        }
    }
    if (!ok) {
        AVLogErrorTo(LOGGER_AVCOMMON_AVLIB_EXPR)
            << "evaluateExpression LHS toInt failed " << lhs << "=\"" << lhsValue << "\"";
        return false;
    }
    int rhsInt = rhsValue.toInt(&ok);
    int rhsExponent = 0;
    if (!ok) {
        int pos = rhsValue.indexOf(comma);
        if (pos >= 0) {
            rhsExponent = rhsValue.length() - pos - 1;
            rhsValue.remove(pos, 1);
            rhsInt = rhsValue.toInt(&ok);
        }
    }
    if (!ok) {
        AVLogErrorTo(LOGGER_AVCOMMON_AVLIB_EXPR)
            << "evaluateExpression RHS toInt failed " << rhs << "=\"" << rhsValue << "\"";
        return false;
    }
    if (lhsExponent > rhsExponent)
        for(int i = lhsExponent - rhsExponent; i != 0; --i) rhsInt *= 10;
    else if (rhsExponent > lhsExponent)
        for(int i = rhsExponent - lhsExponent; i != 0; --i) lhsInt *= 10;
    return evaluateIntExpression(lhsInt, rhsInt);
}

///////////////////////////////////////////////////////////////////////////////

QString AVGreaterThanExpression::toString() const
{
    return (m_lhs.qstring() + ">" + m_rhs.qstring());
}

///////////////////////////////////////////////////////////////////////////////

QString AVGreaterThanExpression::debugString(int indent) const
{
    QString result;
    result.fill(' ', indent);
    result += debugArgumentString(m_lhs);
    result += " GreaterThan ";
    result += debugArgumentString(m_rhs);
    result += "\n";
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVGreaterThanExpression::evaluateIntExpression(int lhs, int rhs) const
{
    return (lhs > rhs);
}

///////////////////////////////////////////////////////////////////////////////

QString AVGreaterThanOrEqualExpression::toString() const
{
    return (m_lhs.qstring() + ">=" + m_rhs.qstring());
}

///////////////////////////////////////////////////////////////////////////////

QString AVGreaterThanOrEqualExpression::debugString(int indent) const
{
    QString result;
    result.fill(' ', indent);
    result += debugArgumentString(m_lhs);
    result += " GreaterThanOrEqual ";
    result += debugArgumentString(m_rhs);
    result += "\n";
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVGreaterThanOrEqualExpression::evaluateIntExpression(int lhs, int rhs) const
{
    return (lhs >= rhs);
}

///////////////////////////////////////////////////////////////////////////////

QString AVLessThanExpression::toString() const
{
    return (m_lhs.qstring() + "<" + m_rhs.qstring());
}

///////////////////////////////////////////////////////////////////////////////

QString AVLessThanExpression::debugString(int indent) const
{
    QString result;
    result.fill(' ', indent);
    result += debugArgumentString(m_lhs);
    result += " LessThan ";
    result += debugArgumentString(m_rhs);
    result += "\n";
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVLessThanExpression::evaluateIntExpression(int lhs, int rhs) const
{
    return (lhs < rhs);
}

///////////////////////////////////////////////////////////////////////////////

QString AVLessThanOrEqualExpression::toString() const
{
    return (m_lhs.qstring() + "<=" + m_rhs.qstring());
}

///////////////////////////////////////////////////////////////////////////////

QString AVLessThanOrEqualExpression::debugString(int indent) const
{
    QString result;
    result.fill(' ', indent);
    result += debugArgumentString(m_lhs);
    result += " LessThanOrEqual ";
    result += debugArgumentString(m_rhs);
    result += "\n";
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVLessThanOrEqualExpression::evaluateIntExpression(int lhs, int rhs) const
{
    return (lhs <= rhs);
}

///////////////////////////////////////////////////////////////////////////////

QString AVMatchesExpression::toString() const
{
    return (m_lhs.qstring() + "=~" + m_rhs.qstring());
}

///////////////////////////////////////////////////////////////////////////////

QString AVMatchesExpression::debugString(int indent) const
{
    QString result;
    result.fill(' ', indent);
    result += debugArgumentString(m_lhs);
    result += " Matches ";
    result += debugArgumentString(m_rhs);
    result += "\n";
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVMatchesExpression::evaluateExpression(const AVComputable& c, const ForEachMap& forEachMap) const
{
    AVHashString lhs(replaceEach(m_lhs, forEachMap));
    AVHashString rhs(replaceEach(m_rhs, forEachMap));
    QString lhsValue = c.valueOfLHS(lhs);
    QString rhsValue = c.valueOfRHS(rhs);
    if (m_failureMode != FM_IGNORE && (lhsValue.isNull() || rhsValue.isNull())) {
        AVLogStream stream = getFailureLogstream();
        stream << "evaluateExpression " << lhs << "=~" << rhs
            << " failed with lhs \"" << lhsValue << "\" rhs \"" << rhsValue << "\"";
    }
    //QRegExp regExp(rhsValue, true, rhs.contains('*'));
    QRegExp regExp(rhsValue);
    return regExp.exactMatch(lhsValue);
}

///////////////////////////////////////////////////////////////////////////////

QString AVNotMatchesExpression::toString() const
{
    return (m_lhs.qstring() + "!~" + m_rhs.qstring());
}

///////////////////////////////////////////////////////////////////////////////

QString AVNotMatchesExpression::debugString(int indent) const
{
    QString result;
    result.fill(' ', indent);
    result += debugArgumentString(m_lhs);
    result += " NotMatches ";
    result += debugArgumentString(m_rhs);
    result += "\n";
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVNotMatchesExpression::evaluateExpression(const AVComputable& c, const ForEachMap& forEachMap) const
{
    AVHashString lhs(replaceEach(m_lhs, forEachMap));
    AVHashString rhs(replaceEach(m_rhs, forEachMap));
    QString lhsValue = c.valueOfLHS(lhs);
    QString rhsValue = c.valueOfRHS(rhs);
    if (m_failureMode != FM_IGNORE && (lhsValue.isNull() || rhsValue.isNull())) {
        AVLogStream stream = getFailureLogstream();
        stream << "evaluateExpression " << lhs << "!~" << rhs
            << " failed with lhs \"" << lhsValue << "\" rhs \"" << rhsValue << "\"";
    }
    //QRegExp regExp(rhsValue, true, rhs.contains('*'));
    QRegExp regExp(rhsValue);
    return !regExp.exactMatch(lhsValue);
}

///////////////////////////////////////////////////////////////////////////////

QString AVAssignExpression::toString() const
{
    return (m_lhs.qstring() + "=" + m_rhs.qstring());
}

///////////////////////////////////////////////////////////////////////////////

QString AVAssignExpression::debugString(int indent) const
{
    QString result;
    result.fill(' ', indent);
    result += debugArgumentString(m_lhs);
    result += " Assign ";
    result += debugArgumentString(m_rhs);
    result += "\n";
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVAssignExpression::evaluateExpression(const AVComputable& c, const ForEachMap& forEachMap) const
{
    Q_UNUSED(c);
    Q_UNUSED(forEachMap);
    AVLogErrorTo(LOGGER_AVCOMMON_AVLIB_EXPR)
        << "evaluateExpression invalid expression " << toString();
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool AVAssignExpression::performAssignments(AVComputable& c, const ForEachMap& forEachMap) const
{
    if (m_first) {
        m_needToReplaceEach = (containsEach(m_lhs, forEachMap) || containsEach(m_rhs, forEachMap));
        m_first = false;
    }
    if (m_needToReplaceEach) {
        AVHashString lhs(replaceEach(m_lhs, forEachMap));
        AVHashString rhs(replaceEach(m_rhs, forEachMap));
        return c.performAssignment(lhs, rhs);
    } else
        return c.performAssignment(m_lhs, m_rhs);
}

///////////////////////////////////////////////////////////////////////////////

QString AVAppendExpression::toString() const
{
    return (m_lhs.qstring() + "+=" + m_rhs.qstring());
}

///////////////////////////////////////////////////////////////////////////////

QString AVAppendExpression::debugString(int indent) const
{
    QString result;
    result.fill(' ', indent);
    result += debugArgumentString(m_lhs);
    result += " Append ";
    result += debugArgumentString(m_rhs);
    result += "\n";
    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool AVAppendExpression::evaluateExpression(const AVComputable& c, const ForEachMap& forEachMap) const
{
    Q_UNUSED(c);
    Q_UNUSED(forEachMap);
    AVLogErrorTo(LOGGER_AVCOMMON_AVLIB_EXPR)
        << "evaluateExpression invalid expression " << toString();
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool AVAppendExpression::performAssignments(AVComputable& c, const ForEachMap& forEachMap) const
{
    if (m_first) {
        m_needToReplaceEach = (containsEach(m_lhs, forEachMap) || containsEach(m_rhs, forEachMap));
        m_first = false;
    }
    if (m_needToReplaceEach) {
        AVHashString lhs(replaceEach(m_lhs, forEachMap));
        AVHashString rhs(replaceEach(m_rhs, forEachMap));
        return c.performAppendAssignment(lhs, rhs);
    } else
        return c.performAppendAssignment(m_lhs, m_rhs);
}

// End of file
