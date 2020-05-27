///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - AviBit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author   Wolfgang Eder, w.eder@avibit.com
    \author    QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief    This file defines classes to parse configuration files in XML
              format.
*/


// Qt includes
#include <QDir>
#include <QFileInfo>
#include <QtXml/QDomProcessingInstruction>
#include <QTextCodec>

// AviBit common includes
#include "avconfig.h" // TODO CM remove after QTP-10 has been resolved
#include "avconfig2.h"
#include "avconfig2adapters.h"
#include "avdomvariables.h"
#include "avmisc.h"


///////////////////////////////////////////////////////////////////////////////

AVDomVariable::AVDomVariable(
    AVDomVariable *parent, const QString& tagName, bool autoCreate)
    : m_parent(parent), m_tagName(tagName), m_autoCreate(autoCreate)
{
    if (parent != 0) parent->addVariable(*this);
}

///////////////////////////////////////////////////////////////////////////////

AVDomVariable::~AVDomVariable()
{
    if (!m_element.isNull()) removeFromParent();
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariable::readFromElement(QDomElement& element)
{
    setElement(element);
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariable::createElement(QDomElement& parent, QDomElement& prev)
{
    AVASSERT(!parent.isNull());
    AVASSERT(m_element.isNull());
    QDomDocument doc = parent.ownerDocument();
    AVASSERT(!doc.isNull());
    m_element = doc.createElement(m_tagName);
    if (!prev.isNull()) checkParent(prev, parent);
    parent.insertAfter(m_element, prev);
    checkParent(m_element, parent);
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariable::setChanged(bool changed)
{
    if (m_parent != 0) m_parent->setChanged(changed);
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariable::checkParent(QDomNode& child, QDomNode& parent)
{
    AVASSERT(!child.isNull());
    AVASSERT(!parent.isNull());
    QDomNode node = child.parentNode();
    AVASSERT(!node.isNull());
    AVASSERT(node == parent);
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariable::checkParent(QDomNode& parent)
{
    AVASSERT(!parent.isNull());
    AVASSERT(!m_element.isNull());
    checkParent(m_element, parent);
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariable::removeFromParent(QDomNode& child, QDomNode& parent)
{
    AVASSERT(!child.isNull());
    AVASSERT(!parent.isNull());
    checkParent(child, parent);
    QDomNode old = parent.removeChild(child);
    AVASSERT(!old.isNull());
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariable::removeFromParent(QDomNode& child)
{
    AVASSERT(!child.isNull());
    QDomNode parent = child.parentNode();
    if (!parent.isNull()) removeFromParent(child, parent);
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariable::removeFromParent()
{
    AVASSERT(!m_element.isNull());
    removeFromParent(m_element);
    m_element.clear();
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariable::setElementText(QDomElement& element, const QString& text)
{
    AVASSERT(!element.isNull());
    while (element.hasChildNodes()) {
        QDomNode child = element.firstChild();
        AVASSERT(!child.isNull());
        removeFromParent(child, element);
    }
    if (!text.isEmpty()) {
        QDomDocument doc = element.ownerDocument();
        AVASSERT(!doc.isNull());
        QDomText child = doc.createTextNode(text);
        AVASSERT(!child.isNull());
        element.appendChild(child);
        checkParent(child, element);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariable::setElementText(const QString& text)
{
    if (m_element.isNull()) {
        AVASSERT(m_parent != 0);
        QDomElement prev;
        createElement(m_parent->element(), prev);
        AVASSERT(!m_element.isNull());
    }
    QString textWithoutCR = text;
    textWithoutCR.remove('\r');
    setElementText(m_element, textWithoutCR);
    setChanged(true);
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariable::ignore(QDomNode& n)
{
    if (n.nodeType() == QDomNode::CommentNode) {
        LOGGER_AVCOMMON_AVLIB.Write(LOG_DEBUG, "ignoring comment");
        return;
    }
    LOGGER_AVCOMMON_AVLIB.Write(LOG_WARNING, "ignoring %s %s", qPrintable(n.nodeName()),
                                                               qPrintable(n.nodeValue()));
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariable::signalSelection()
{
    if (m_parent != 0) m_parent->slotSelection();
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariable::readFromDocument(QDomDocument& doc)
{
    QDomElement element = doc.documentElement();
    readFromElement(element);
    setChanged(false);
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariable::readFromString(const QString& xml)
{
    QDomDocument doc;
    AVASSERT(doc.setContent(xml));
    readFromDocument(doc);
}

///////////////////////////////////////////////////////////////////////////////

AVDomString::AVDomString(
    AVDomVariable *parent, const QString& tagName, bool autoCreate, const char* member,
    const QString& defaultText)
    : AVDomVariable(parent, tagName, autoCreate), m_text(defaultText)
{
    if (member != 0) {
        AVASSERT(parent != 0);
        AVDomObject *obj = parent->toDomObject();
        AVASSERT(obj != 0);
        AVDIRECTCONNECT(this, SIGNAL(signalComputeDependentValue()), obj, member);
    }
}

/////////////////////////////////////////////////////////////////////////////

const QString& AVDomString::textNotNull() const
{
    return (m_text.isNull() ? EmptyQString : m_text);
}

/////////////////////////////////////////////////////////////////////////////

const QString AVDomString::configText() const
{
    return element().text();
}

/////////////////////////////////////////////////////////////////////////////

void AVDomString::setText(const QString& text)
{
    if (AVConfig2Global::isSingletonInitialized()) {
        AVConfig2XmlAdapter adapter(AVConfig2Global::singleton());
        m_text = adapter.replaceLoadedParameters(text);
    } else
        m_text = text;
    computeValue(m_text, element());
    setElementText(text);
}

///////////////////////////////////////////////////////////////////////////////

void AVDomString::readFromElement(QDomElement& element)
{
    AVDomVariable::readFromElement(element);
    QString text = element.text();
    if (AVConfig2Global::isSingletonInitialized()) {
        AVConfig2XmlAdapter adapter(AVConfig2Global::singleton());
        m_text = adapter.replaceLoadedParameters(text);
    } else
        m_text = text;
    computeValue(m_text, element);
    setElementText(text);
}

///////////////////////////////////////////////////////////////////////////////

void AVDomString::createElement(QDomElement& parent, QDomElement& prev)
{
    AVDomVariable::createElement(parent, prev);
    setElementText(m_text);
}

///////////////////////////////////////////////////////////////////////////////

void AVDomString::computeValue(const QString&, QDomElement&)
{
    emit signalComputeDependentValue();
}

///////////////////////////////////////////////////////////////////////////////

AVDomBool::AVDomBool(
    AVDomVariable *parent, const QString& tagName, bool autoCreate, bool defaultValue)
    : AVDomString(parent, tagName, autoCreate, 0, boolToString(defaultValue)),
    m_value(defaultValue)
{
}

/////////////////////////////////////////////////////////////////////////////

void AVDomBool::computeValue(const QString& text, QDomElement& element)
{
    m_value = (text == "true");
    AVDomString::computeValue(text, element);
}

/////////////////////////////////////////////////////////////////////////////

QString AVDomBool::boolToString(bool value)
{
    return (value?"true":"false");
}

/////////////////////////////////////////////////////////////////////////////

void AVDomBool::setValue(bool value)
{
    setText(boolToString(value));
    AVASSERT(m_value == value);
}

///////////////////////////////////////////////////////////////////////////////

AVDomRegExp::AVDomRegExp(
    AVDomVariable *parent, const QString& tagName, bool autoCreate, const QString& defaultPattern)
    : AVDomString(parent, tagName, autoCreate, 0, defaultPattern),
    m_regExp(defaultPattern)
{
}

/////////////////////////////////////////////////////////////////////////////

void AVDomRegExp::computeValue(const QString& text, QDomElement& element)
{
    m_regExp.setPattern(text);
    AVDomString::computeValue(text, element);
}

///////////////////////////////////////////////////////////////////////////////

AVDomColor::AVDomColor(
    AVDomVariable *parent, const QString& tagName, bool autoCreate, const QColor& defaultValue)
    : AVDomString(parent, tagName, autoCreate, 0, defaultValue.name()),
    m_color(defaultValue)
{
}

/////////////////////////////////////////////////////////////////////////////

void AVDomColor::computeValue(const QString& text, QDomElement& element)
{
    m_color = QColor(text);
    AVDomString::computeValue(text, element);
}

/////////////////////////////////////////////////////////////////////////////

void AVDomColor::setValue(const QColor& value)
{
    setText(value.name());
}

///////////////////////////////////////////////////////////////////////////////

AVDomPoint::AVDomPoint(
    AVDomVariable *parent, const QString& tagName, bool autoCreate, const QPoint& defaultValue)
    : AVDomString(parent, tagName, autoCreate, 0, pointToString(defaultValue)),
    m_point(defaultValue)
{
}

/////////////////////////////////////////////////////////////////////////////

void AVDomPoint::computeValue(const QString& text, QDomElement& element)
{
    QStringList list = text.split(',', QString::SkipEmptyParts);
    if (list.count() < 2)
        m_point = QPoint();
    else {
        m_point.setX(list[0].toInt());
        m_point.setY(list[1].toInt());
    }
    AVDomString::computeValue(text, element);
}

/////////////////////////////////////////////////////////////////////////////

QString AVDomPoint::pointToString(const QPoint& value)
{
    return QString("%1,%2").arg(value.x()).arg(value.y());
}

/////////////////////////////////////////////////////////////////////////////

void AVDomPoint::setValue(const QPoint& value)
{
    setText(pointToString(value));
    AVASSERT(m_point == value);
}

///////////////////////////////////////////////////////////////////////////////

AVDomSize::AVDomSize(
    AVDomVariable *parent, const QString& tagName, bool autoCreate, const QSize& defaultValue)
    : AVDomString(parent, tagName, autoCreate, 0, sizeToString(defaultValue)),
    m_size(defaultValue)
{
}

/////////////////////////////////////////////////////////////////////////////

void AVDomSize::computeValue(const QString& text, QDomElement& element)
{
    QStringList list = text.split(',', QString::SkipEmptyParts);
    if (list.count() < 2)
        m_size = QSize();
    else {
        m_size.setWidth(list[0].toInt());
        m_size.setHeight(list[1].toInt());
    }
    AVDomString::computeValue(text, element);
}

/////////////////////////////////////////////////////////////////////////////

QString AVDomSize::sizeToString(const QSize& value)
{
    if (!value.isValid()) return "";
    return QString("%1,%2").arg(value.width()).arg(value.height());
}

/////////////////////////////////////////////////////////////////////////////

void AVDomSize::setValue(const QSize& value)
{
    AVASSERT(value.isValid());
    setText(sizeToString(value));
    AVASSERT(m_size == value);
}

/////////////////////////////////////////////////////////////////////////////

AVDomHashString::AVDomHashString(
    AVDomVariable *parent, const QString& tagName, bool autoCreate, const char *member,
    const QString& defaultText)
    : AVDomString(parent, tagName, autoCreate, member, defaultText),
    m_string(0)
{
    m_string = new AVHashString(defaultText);
    AVASSERT(m_string != 0);
}

///////////////////////////////////////////////////////////////////////////////

const AVHashString& AVDomHashString::value() const
{
    AVASSERT(m_string != 0);
    return *m_string;
}

///////////////////////////////////////////////////////////////////////////////

void AVDomHashString::computeValue(const QString& text, QDomElement& element)
{
    delete m_string;
    m_string = new AVHashString(text);
    AVASSERT(m_string != 0);
    AVDomString::computeValue(text, element);
}

///////////////////////////////////////////////////////////////////////////////

AVDomInt::AVDomInt(
    AVDomVariable *parent, const QString& tagName, bool autoCreate, int defaultValue)
    : AVDomString(parent, tagName, autoCreate, 0, QString::number(defaultValue)),
    m_value(defaultValue)
{
}

/////////////////////////////////////////////////////////////////////////////

void AVDomInt::computeValue(const QString& text, QDomElement& element)
{
    bool ok;
    m_value = text.toInt(&ok);
    if (!ok) LOGGER_AVCOMMON_AVLIB.Write(LOG_WARNING, "invalid int " + text);
    AVDomString::computeValue(text, element);
}

/////////////////////////////////////////////////////////////////////////////

void AVDomInt::setValue(int value)
{
    setText(QString::number(value));
    AVASSERT(m_value == value);
}

///////////////////////////////////////////////////////////////////////////////

AVDomUInt::AVDomUInt(
    AVDomVariable *parent, const QString& tagName, bool autoCreate, uint defaultValue)
    : AVDomString(parent, tagName, autoCreate, 0, QString::number(defaultValue)),
    m_value(defaultValue)
{
}

/////////////////////////////////////////////////////////////////////////////

void AVDomUInt::computeValue(const QString& text, QDomElement& element)
{
    bool ok;
    m_value = text.toUInt(&ok);
    if (!ok) LOGGER_AVCOMMON_AVLIB.Write(LOG_WARNING, "invalid uint " + text);
    AVDomString::computeValue(text, element);
}

/////////////////////////////////////////////////////////////////////////////

void AVDomUInt::setValue(uint value)
{
    setText(QString::number(value));
    AVASSERT(m_value == value);
}

///////////////////////////////////////////////////////////////////////////////

AVDomDouble::AVDomDouble(
    AVDomVariable *parent, const QString& tagName, bool autoCreate, double defaultValue)
    : AVDomString(parent, tagName, autoCreate, 0, QString::number(defaultValue)),
    m_value(defaultValue)
{
}

/////////////////////////////////////////////////////////////////////////////

void AVDomDouble::computeValue(const QString& text, QDomElement& element)
{
    bool ok;
    m_value = text.toDouble(&ok);
    if (!ok) LOGGER_AVCOMMON_AVLIB.Write(LOG_WARNING, "invalid double %s" + text);
    AVDomString::computeValue(text, element);
}

/////////////////////////////////////////////////////////////////////////////

void AVDomDouble::setValue(double value)
{
    setText(QString::number(value, 'g', 12));
    AVASSERT(qFuzzyCompare(m_value, value));
}

///////////////////////////////////////////////////////////////////////////////

AVDomObject::AVDomObject(AVDomVariable *parent, const QString& tagName, bool autoCreate)
    : AVDomVariable(parent, tagName, autoCreate)
{
}

///////////////////////////////////////////////////////////////////////////////

void AVDomObject::addVariable(AVDomVariable& var)
{
    m_variables.append(&var);
}

///////////////////////////////////////////////////////////////////////////////

void AVDomObject::autoCreateElements()
{
    QDomElement prev;
    QMutableListIterator<AVDomVariable*> it (m_variables);

    while (it.hasNext())
    {
        AVDomVariable *var = it.next();
        if (var->element().isNull()) {
            if (var->autoCreate()) var->createElement(element(), prev);
        } else
            element().insertAfter(var->element(), prev);
        if (!var->element().isNull()) prev = var->element();
    }

//    while (AVDomVariable *var = it.value()) {
//        if (var->element().isNull()) {
//            if (var->autoCreate()) var->createElement(element(), prev);
//        } else
//            element().insertAfter(var->element(), prev);
//        if (!var->element().isNull()) prev = var->element();
//        it.next();
//    }
}

///////////////////////////////////////////////////////////////////////////////

void AVDomObject::readFromElement(QDomElement& element)
{
    AVDomVariable::readFromElement(element);
    QMutableListIterator<AVDomVariable*> it (m_variables);
    for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
        checkParent(n, element);
        QDomElement e = n.toElement();
        QString tag = e.tagName();
        AVDomVariable *var = 0;
        it.toFront();
        while (it.hasNext())
        {
            if ((var = it.next()) != 0) {
                if (var->tagName() == tag) {
                    var->readFromElement(e);
                    break;
                }
            }
        }
        if (var == 0) ignore(n);

//        while ((var = it.value()) != 0) {
//            if (var->tagName() == tag) {
//                var->readFromElement(e);
//                break;
//            }
//            it.next();
//        }
//        if (var == 0) ignore(n);
    }
    autoCreateElements();
}

///////////////////////////////////////////////////////////////////////////////

void AVDomObject::createElement(QDomElement& parent, QDomElement& prev)
{
    AVDomVariable::createElement(parent, prev);
    autoCreateElements();
}

///////////////////////////////////////////////////////////////////////////////

void AVDomObject::setChanged(bool changed)
{
    emit signalChanged();
    AVDomVariable::setChanged(changed);
}



/////////////////////////////////////////////////////////////////////////////

AVDomStringListSep::AVDomStringListSep(
    AVDomVariable *parent, const QString& tagName, bool autoCreate, const QString& sep,
    const char *member)
    : AVDomString(parent, tagName, autoCreate, member), m_sep(sep)
{
}

/////////////////////////////////////////////////////////////////////////////

void AVDomStringListSep::setValue(const QStringList& value)
{
    if (value == QStringList{""})
    {
        // AVDomStringListSep cannot contain a single empty string (ELSIS-1332)
        AVLogWarning << "AVDomStringListSep::setValue " << value << " is converted to an empty list";
        setValue(QStringList());
        return;
    }
    setText(value.join(m_sep));
    AVLogAssert(m_value == value) << "AVDomStringListSep::setValue " << value << " yields " << m_value;
}

/////////////////////////////////////////////////////////////////////////////

void AVDomStringListSep::computeValue(const QString& text, QDomElement& element)
{
    m_sep = element.attribute("sep", m_sep);
    m_value = text.isEmpty() ? QStringList() : text.split(m_sep, QString::KeepEmptyParts);
    AVDomString::computeValue(text, element);
}

/////////////////////////////////////////////////////////////////////////////////

// autoCreate is done here immediately because we know that an AVDomConfig
// is the root of the object hierarchy. In all other places, autoCreate is
// evaluated by the parent of the AVDomVariable.
// Thus we pass false to the AVDomObject constructor.

AVDomConfig::AVDomConfig(const QString& tagName, bool autoCreate)
    : AVDomObject(0, tagName, false), m_changed(false)
{
    if (autoCreate) {
        QDomProcessingInstruction pi =
            m_doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
        m_doc.appendChild(pi);
        setElement(m_doc.createElement(tagName));
        m_doc.appendChild(element());
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVDomConfig::setFileName(const QString& fileName)
{
    if (AVConfig2Global::isSingletonInitialized()) {
        m_fullPath = AVConfig2Global::singleton().findUserConfig(fileName, ".xfg");
        QFileInfo fileInfo(m_fullPath);
        m_fileName = fileInfo.fileName();
    } else {
        m_fileName = AVConfigBase::configFileMapping(fileName, ".xfg");
        QString path = AVEnvironment::getApplicationConfig();
        if (path.isEmpty()) {
            path = QDir::homePath();
            if (path.isEmpty()) path = ".";
        }
        AVASSERT(!path.startsWith("#"));
        m_fullPath = path + "/" + m_fileName;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVDomConfig::readFromDocument(QDomDocument& doc)
{
    m_doc = doc;
    AVDomObject::readFromDocument(doc);
}

///////////////////////////////////////////////////////////////////////////////

bool AVDomConfig::fileIsReadable() const
{
    QFileInfo fileInfo(m_fullPath);
    return (fileInfo.exists() && fileInfo.isReadable());
}

///////////////////////////////////////////////////////////////////////////////

void AVDomConfig::readFromFile()
{
    if (!readFromFileChecked())
    {
        AVConfig2Global::singleton().registerConfigurationError(
            QString("AVDomConfig::readFromFile: Failed to read %1").arg(m_fullPath));
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVDomConfig::readFromFileChecked()
{
    AVLogInfo << "AVDomConfig::readFromFileChecked " << m_fullPath;
    QFile file(m_fullPath);
    if (!file.open(QIODevice::ReadOnly)) {
        // The caller has to check the return value, failure to read can be expected here -> log info only.
        AVLogInfo << "AVDomConfig::readFromFileChecked: Unable to open file " << m_fullPath;
        return false;
    }
    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;
    bool result = doc.setContent(&file, &errorMsg, &errorLine, &errorColumn);
    file.close();
    if (!result) {
        AVLogError << "AVDomConfig::readFromFileChecked: Failure to parse document "
                   << m_fullPath << ": "
                   << errorMsg << " line " << errorLine << " column " << errorColumn;
        return false;
    }
    readFromDocument(doc);

    // This requires that at least one AVConfigBase instance has already been
    // created, and parseSpecialParams() has been called.

    if (AVConfigBase::option_save
        || (AVConfig2Global::isSingletonInitialized() && AVConfig2Global::singleton().isSaveRequested()))
            writeToFile();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void AVDomConfig::readFromFile(const QString& fileName)
{
    setFileName(fileName);
    readFromFile();
}

///////////////////////////////////////////////////////////////////////////////

QString AVDomConfig::writeToString()
{
    return m_doc.toString();
}

///////////////////////////////////////////////////////////////////////////////

QByteArray AVDomConfig::writeToByteArray()
{
    return m_doc.toByteArray();
}

///////////////////////////////////////////////////////////////////////////////

void AVDomConfig::writeToFile()
{
    LOGGER_AVCOMMON_AVLIB.Write(LOG_INFO, "writeToFile %s", qPrintable(m_fullPath));
    QFile file(m_fullPath);
    AVASSERT(file.open(QIODevice::WriteOnly));
    QTextStream stream(&file);
    stream.setCodec(QTextCodec::codecForName("UTF-8"));
    stream << m_doc.toString();
    file.close();
    setChanged(false);
}

///////////////////////////////////////////////////////////////////////////////

void AVDomConfig::writeToFile(const QString& fileName)
{
    setFileName(fileName);
    writeToFile();
}

///////////////////////////////////////////////////////////////////////////////

void AVDomConfig::setChanged(bool changed)
{
    if (m_changed != changed)
        LOGGER_AVCOMMON_AVLIB.Write(
            LOG_INFO, "%s: setChanged %s", this->metaObject()->className(),
                                          (changed?"true":"false"));
    m_changed = changed;
    AVDomObject::setChanged(changed);
}

///////////////////////////////////////////////////////////////////////////////

void AVDomStringList::readFromElement(QDomElement& element)
{
    AVDomList<AVDomString>::readFromElement(element);
    m_value.clear();
    QListIterator<AVDomString*> it(list());
    while (it.hasNext())
    {
        AVDomString *each = it.next();
        QString str = each->text();
        m_value.append(str);
    }
}

// End of file
