///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - AviBit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2010
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author    Wolfgang Eder, w.eder@avibit.com
    \author    QT4-Port: Christian Muschick, c.muschick@avibit.com
    \brief     This file declares classes to parse configuration files in XML
               format.
*/

#ifndef AVDOMVARIABLES_H
#define AVDOMVARIABLES_H

// Qt includes
#include <QColor>
#include <QByteArray>
#include <QHash>
#include <QtXml/qdom.h>
#include <qobject.h>
#include <QList>
#include <QRegExp>
#include <QStringList>

// DIFLIS includes
#include "avlib_export.h"
#include "avexpressions.h"
#include "avlog.h"
#include "avconfig.h" // TODO CM remove after QTP-10 has been resolved
#include "avmacros.h"

class AVDomObject; // forward declaration

///////////////////////////////////////////////////////////////////////////////

/*! \page avdomvariables Parsing XML using AVDomObject

    The class AVDomObject, and its related classes based on AVDomVariable,
    can be used to parse an XML file and access its contents using data members.
    The data can also be modified, and the modified file can be saved.
    Unknown tags in the XML file are left unchanged when writing the file.

    To parse a file, you need to create a subclass of AVDomConfig, as
    illustrated in example 1.

    \code
    #include "avdomvariables.h"

    class TestDomConfig1 : public AVDomConfig
    {
        Q_OBJECT
    public:
        // Constructor
        TestDomConfig1()
            : AVDomConfig("TestDomConfig1"),
            string(this, "string", true, 0, "default")
        {
            setFileName("testdomconfig1");
        }
    private:
        // Copy-constructor: defined but not implemented
        TestDomConfig1(const TestDomConfig1& rhs);
        // Assignment operator: defined but not implemented
        TestDomConfig1& operator=(const TestDomConfig1& rhs);
    public:
        AVDomString string;
    };

    // Use the class to read the file "testdomconfig1.xfg"

    TestDomConfig1 config;
    config.readFromFile();
    \endcode

    This class can be used to read and write a file with the following
    structure:

    \verbatim
    <?xml version = '1.0' encoding = 'utf-8'?>
    <TestDomConfig1>
     <string>value</string>
    </TestDomConfig1>
    \endverbatim

    The class AVDomConfig can be combined with AVDomConfigSingleton to implement
    a singleton for the configuration.

    The first parameter for all AVDomVariable constructors is the parent object,
    usually "this".
    The second parameter is the tag name for the variable. Each variable needs
    its unique tag name.
    The third parameter is a flag that indicates whether the tag should be
    created automatically when it is found missing in the file. This is relevant
    only when the file is written back, and can be used to suppress empty tags
    for rarely used variables.
    The remaining parameters depend on the type of the variable.

    Example 2 demonstrates all the supported data types, as well as some
    advanced features of the framework. By looking at the code and the example
    data file, it should be obvious how the framework can be used.

    \code
    #include "avdomvariables.h"
    #include "avexplicitsingleton.h"

    class TestDomObject2;     // see below
    class TestDomObjectList2; // see below
    class TestDomObjectDict2; // see below

    class TestDomConfig2
        : public AVDomConfig, public AVDomConfigSingleton<TestDomConfig2>
    {
        Q_OBJECT
    public:
        // Initialize the global configuration data instance
        static TestDomConfig2& initializeSingleton(const QString& fileName = "testdomconfig2")
            { return setSingleton(new (LOG_HERE) TestDomConfig2(fileName)); }
    public:
        // Constructor
        explicit TestDomConfig2(const QString& fileName)
            : AVDomConfig("TestDomConfig2"),
            string  (this, "string",   true, SLOT(slotStringChanged()), "default"),
            regexp  (this, "regexp",   true, "default"),
            color   (this, "color",    true, "red"),
            boolean (this, "bool",     true, true),
            integer (this, "int",      true, 42),
            strings1(this, "strings1", true, ","),
            strings2(this, "strings2", true, "string", true),
            size    (this, "size",     true, QSize(10, 10)),
            object  (this, "Object",   true),
            list    (this, "list",     true, "Object", true),
            dict    (this, "dict",     true, "Object", true)
        {
            setFileName(fileName);
        }
    protected slots:
        // This slot gets called whenever the value of the
        // string data member changes.
        void slotStringChanged() { stringAsInt = string.toInt(); }
    private:
        // Copy-constructor: defined but not implemented
        TestDomConfig2(const TestDomConfig2& rhs);
        // Assignment operator: defined but not implemented
        TestDomConfig2& operator=(const TestDomConfig2& rhs);
    public:
        AVDomString        string;
        int                stringAsInt;
        AVDomRegExp        regexp;
        AVDomColor         color;
        AVDomBool          boolean;
        AVDomInt           integer; // there is also AVDomUInt and AVDomDouble
        AVDomStringListSep strings1;
        AVDomStringList    strings2;
        AVDomSize          size;
        TestDomObject2     object;
        TestDomObjectList2 list;
        TestDomObjectDict2 dict;
    };
    \endcode

    Here is an example configuration file that can be read and written by the
    above class:

    \verbatim
    <?xml version = '1.0' encoding = 'utf-8'?>
    <!--
        Comments can be placed anywhere in the file,
        but after the inital xml tag.
    -->
    <TestDomConfig2>
     <string>value</string>
     <regexp>.*</regexp>
     <color>#00ff00</color>
     <bool>false</bool>
     <int>17</int>
     <strings1 sep=";">A;B;C</stringList1>
     <strings2>
      <string>A</string>
      <string>B</string>
      <string>C</string>
     </strings2>
     <size>2,4</size>
     <Object>
      <string>test</string>
     </Object>
     <list>
      <Object>
       <string>test1</string>
      </Object>
      <Object>
       <string>test2</string>
      </Object>
     </list>
     <dict>
      <Object>
       <string>test3</string>
      </Object>
      <Object>
       <string>test4</string>
      </Object>
     </dict>
    </TestDomConfig2>
    \endverbatim

    For completeness, here is the implementation of the remaining classes
    for the example:

    \code
    class TestDomObject2 : public AVDomObject
    {
        Q_OBJECT
    public:
        // Constructor
        TestDomObject2(AVDomVariable *parent, const QString& tagName, bool autoCreate)
            : AVDomObject(parent, tagName, autoCreate),
            string(this, "string", true) { }
        // the key for this object used by AVDomDict
        const QString& dictKey() const { return string.textNotNull(); }
    private:
        // Copy-constructor: defined but not implemented
        TestDomObject2(const TestDomObject2& rhs);
        // Assignment operator: defined but not implemented
        TestDomObject2& operator=(const TestDomObject2& rhs);
    public:
        AVDomString string;
    };

    class TestDomObjectList2 : public AVDomList<TestDomObject2>
    {
    public:
        // Constructor
        TestDomObjectList2(
            AVDomVariable *parent, const QString& tagName, bool autoCreate,
            const QString& elementTagName)
        : AVDomList<TestDomObject2>(parent, tagName, autoCreate, elementTagName) { }
    };

    class TestDomObjectDict2 : public AVDomDict<TestDomObject>
    {
    public:
        // Constructor
        TestDomObjectDict2(
            AVDomVariable *parent, const QString& tagName, bool autoCreate,
            const QString& elementTagName)
        : AVDomDict<TestDomObject2>(parent, tagName, autoCreate, elementTagName) { }
    };
    \endcode
*/

///////////////////////////////////////////////////////////////////////////////

//! This abstract class represents a variable that is connected to a
//! QDomElement. It supports reading its value from the QDomElement, and setting
//! its value to the QDomElement. It can optionally create the QDomElement, so
//! that saving the DOM tree creates the corresponding nodes automatically.

class AVLIB_EXPORT AVDomVariable
{
public:
    //! Construct a new instance, and add it to its parent's list of variables
    /*! \param parent     the parent variable that owns this variable
        \param tagName    the XML tag name
        \param autoCreate this flag indicates whether a QDomElement should be
                          created automatically
    */
    AVDomVariable(AVDomVariable *parent, const QString& tagName, bool autoCreate);
    //! Destroy the instance, and remove the QDomElement from the DOM tree
    virtual ~AVDomVariable();
    //! Accessor for the parent AVDomVariable
    AVDomVariable *parentVariable() { return m_parent; }
    //! Accessor for the XML tag name
    const QString& tagName() const { return m_tagName; }
    //! This flag indicates whether a QDomElement should be created
    //! automatically.
    bool autoCreate() const { return m_autoCreate; }
    //! Accessor for the QDomElement
    QDomElement& element() { return m_element; }
    //! Const accessor for the QDomElement
    const QDomElement& element() const { return m_element; }
    //! When a new instance is created, this method will be called for the
    //! parent, allowing it to register its new child instance.
    virtual void addVariable(AVDomVariable&) { }
    //! Read the variable's value from the QDomElement.
    virtual void readFromElement(QDomElement& element);
    //! Create the QDomElement for this variable
    /*! \param parent the parent variable's QDomElement
        \param prev   the predecessor variable's QDomElement. This is used to
                      create the QDomElement at the right position within the
                      DOM tree.
    */
    virtual void createElement(QDomElement& parent, QDomElement& prev);
    //! This method gets called when the value changes. It propagates the change
    //! to the parent.
    virtual void setChanged(bool changed);
    //! Downcast method
    virtual AVDomObject *toDomObject() { return 0; }
    virtual void readFromDocument(QDomDocument& doc);
    void readFromString(const QString& xml);
protected:
    //! Set accessor for the QDomElement
    void setElement(const QDomElement& element) { m_element = element; }
    //! This method asserts the parent/child relationship.
    static void checkParent(QDomNode& child, QDomNode& parent);
    //! This method asserts that m_element is a child of parent.
    void checkParent(QDomNode& parent);
    //! This method removes the child node from the parent.
    static void removeFromParent(QDomNode& child, QDomNode& parent);
    //! This method removes the child node from its parent.
    void removeFromParent(QDomNode& child);
    //! This method removes m_element from its parent.
    void removeFromParent();
    //! This method sets the element's text, and removes its child nodes.
    static void setElementText(QDomElement& element, const QString& text);
    //! This method sets m_element's text, and removes its child nodes.
    void setElementText(const QString& text);
    //! This method is called to indicate that a QDomNode is ignored while
    //! parsing a XML file.
    static void ignore(QDomNode& n);
    //! Calls the slotSelection() method on the parent variable.
    void signalSelection();
    //! This method gets called when a list changes the selectedObject.
    virtual void slotSelection() { }
private:
    //! Copy-constructor: defined but not implemented
    AVDomVariable(const AVDomVariable& rhs);
    //! Assignment operator: defined but not implemented
    AVDomVariable& operator=(const AVDomVariable& rhs);
private:
    AVDomVariable *m_parent;  //!< the parent variable that owns this variable
    QString        m_tagName; //!< the XML tag name
    //! this flag indicates whether a QDomElement should be created
    //! automatically
    bool           m_autoCreate;
    QDomElement    m_element; //!< the QDomElement
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents a string variable that is connected to a QDomElement.

class AVLIB_EXPORT AVDomString : public QObject, public AVDomVariable
{
    Q_OBJECT
public:
    //! Construct a new instance, and add it to its parent's list of variables
    AVDomString(
        AVDomVariable *parent, const QString& tagName, bool autoCreate,
        const char *member = 0, const QString& defaultText = QString());
    //! Accessor for the variable's string value
    const QString& text() const { return m_text; }
    //! Accessor for the variable's string value, answers an empty string if text is null.
    const QString& textNotNull() const;
    //! Accessor for the variable's string value without config parameter substitution
    const QString configText() const;
    //! Set accessor for the variable's string value
    virtual void setText(const QString& text);
    //! Read the variable's value from the QDomElement.
    void readFromElement(QDomElement& element) override;
    //! Create the QDomElement for this variable
    void createElement(QDomElement& parent, QDomElement& prev) override;
    //! Answer whether the text is null
    bool isNull() const { return m_text.isNull(); }
    //! Answer whether the text is empty
    bool isEmpty() const { return m_text.isEmpty(); }
    //! Answer the length of text
    uint length() const { return m_text.length(); }
    //! Answer whether the variable contains the specified text
    bool contains(const QString& txt) const { return m_text.contains(txt); }
    //! Cast operator, required to automatically coerce a AVDomString to a
    //! QString.
    // Removed because it violates sa-40
    //operator const QString&() const { return text(); }
    //! Assignment operator for a AVDomString rhs.
    AVDomString& operator=(const AVDomString& rhs) { setText(rhs.text()); return *this; }
    //! Assignment operator for a QString rhs.
    AVDomString& operator=(const QString& rhs) { setText(rhs); return *this; }
protected:
    //! This method allows subclasses to compute a value from the specified text
    virtual void computeValue(const QString& text, QDomElement& element);
signals:
    //! This signal gets emitted when the text changes, and any dependent values
    //! need to be computed.
    void signalComputeDependentValue();
private:
    //! Copy-constructor: defined but not implemented
    AVDomString(const AVDomString& rhs);
private:
    QString m_text; //!< the string value
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents a QRegExp variable that is connected to a QDomElement.

class AVLIB_EXPORT AVDomRegExp : public AVDomString
{
    Q_OBJECT
public:
    //! Construct a new instance, and add it to its parent's list of variables
    AVDomRegExp(AVDomVariable *parent, const QString& tagName, bool autoCreate, const QString& defaultPattern = ".*");
    //! Accessor for the variable's QRegExp value
    const QRegExp& value() const { return m_regExp; }
    //! Answer whether the specified string matches the QRegExp
    bool exactMatch(const QString& string) const { return m_regExp.exactMatch(string); }
    //! Cast operator, required to automatically coerce an AVDomColor to a
    //! QColor.
    // Removed because it violates sa-40
    //operator const QColor&() const { return value(); }
protected:
    //! Compute the color value from the specified text.
    void computeValue(const QString& text, QDomElement& element) override;

private:
    //! Copy-constructor: defined but not implemented
    AVDomRegExp(const AVDomRegExp& rhs);
    //! Assignment operator: defined but not implemented
    AVDomRegExp& operator=(const AVDomRegExp& rhs);
private:
    QRegExp m_regExp; //!< the QRegExp value
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents a color variable that is connected to a QDomElement.

class AVLIB_EXPORT AVDomColor : public AVDomString
{
    Q_OBJECT
public:
    //! Construct a new instance, and add it to its parent's list of variables
    AVDomColor(AVDomVariable *parent, const QString& tagName, bool autoCreate, const QColor& defaultValue = QColor());
    //! Accessor for the variable's color value
    const QColor& value() const { return m_color; }
    //! Set accessor for the variable's color value
    void setValue(const QColor& value);
    //! Cast operator, required to automatically coerce an AVDomColor to a
    //! QColor.
    // Removed because it violates sa-40
    //operator const QColor&() const { return value(); }
    //! Assignment operator for a QColor rhs.
    AVDomColor& operator=(const QColor& rhs) { setValue(rhs); return *this; }
protected:
    //! Compute the color value from the specified text.
    void computeValue(const QString& text, QDomElement& element) override;

private:
    //! Copy-constructor: defined but not implemented
    AVDomColor(const AVDomColor& rhs);
    //! Assignment operator: defined but not implemented
    AVDomColor& operator=(const AVDomColor& rhs);
private:
    QColor m_color; //!< the color value
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents a bool variable that is connected to a QDomElement.

class AVLIB_EXPORT AVDomBool : public AVDomString
{
    Q_OBJECT
public:
    static QString boolToString(bool value);
public:
    //! Construct a new instance, and add it to its parent's list of variables
    AVDomBool(AVDomVariable *parent, const QString& tagName, bool autoCreate, bool defaultValue = false);
    //! Accessor for the variable's bool value
    bool value() const { return m_value; }
    //! Set accessor for the variable's bool value
    void setValue(bool value);
    //! Cast operator, required to automatically coerce a AVDomBool to a bool.
    // Removed because it violates sa-40
    //operator bool() const { return value(); }
    //! Assignment operator for a bool rhs.
    AVDomBool& operator=(bool rhs) { setValue(rhs); return *this; }
protected:
    //! Compute the bool value from the specified text.
    void computeValue(const QString& text, QDomElement& element) override;

private:
    //! Copy-constructor: defined but not implemented
    AVDomBool(const AVDomBool& rhs);
    //! Assignment operator: defined but not implemented
    AVDomBool& operator=(const AVDomBool& rhs);
private:
    bool m_value; //!< the bool value
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents a signed int variable that is connected to a
//! QDomElement.

class AVLIB_EXPORT AVDomInt : public AVDomString
{
    Q_OBJECT
public:
    //! Construct a new instance, and add it to its parent's list of variables
    AVDomInt(AVDomVariable *parent, const QString& tagName, bool autoCreate, int defaultValue = 0);
    //! Accessor for the variable's signed int value
    int value() const { return m_value; }
    //! Set accessor for the variable's signed int value
    void setValue(int value);
    //! Cast operator, required to automatically coerce a AVDomInt to a int.
    // Removed because it violates sa-40
    //operator int() const { return value(); }
    //! Assignment operator for a int rhs.
    AVDomInt& operator=(int rhs) { setValue(rhs); return *this; }
protected:
    //! Compute the int value from the specified text.
    void computeValue(const QString& text, QDomElement& element) override;

private:
    //! Copy-constructor: defined but not implemented
    AVDomInt(const AVDomInt& rhs);
    //! Assignment operator: defined but not implemented
    AVDomInt& operator=(const AVDomInt& rhs);
private:
    int m_value; //!< the signed int value
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents an unsigned int variable that is connected to a
//! QDomElement.

class AVLIB_EXPORT AVDomUInt : public AVDomString
{
    Q_OBJECT
public:
    //! Construct a new instance, and add it to its parent's list of variables
    AVDomUInt(AVDomVariable *parent, const QString& tagName, bool autoCreate, uint defaultValue = 0);
    //! Accessor for the variable's unsigned int value
    uint value() const { return m_value; }
    //! Set accessor for the variable's unsigned int value
    void setValue(uint value);
    //! Cast operator, required to automatically coerce a AVDomUInt to a uint.
    // Removed because it violates sa-40
    //operator uint() const { return value(); }
    //! Assignment operator for a uint rhs.
    AVDomUInt& operator=(uint rhs) { setValue(rhs); return *this; }
protected:
    //! Compute the uint value from the specified text.
    void computeValue(const QString& text, QDomElement& element) override;

private:
    //! Copy-constructor: defined but not implemented
    AVDomUInt(const AVDomUInt& rhs);
    //! Assignment operator: defined but not implemented
    AVDomUInt& operator=(const AVDomUInt& rhs);
private:
    uint m_value; //!< the unsigned int value
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents a double variable that is connected to a
//! QDomElement.

class AVLIB_EXPORT AVDomDouble : public AVDomString
{
    Q_OBJECT
public:
    //! Construct a new instance, and add it to its parent's list of variables
    AVDomDouble(AVDomVariable *parent, const QString& tagName, bool autoCreate, double defaultValue = 0.0);
    //! Accessor for the variable's unsigned int value
    double value() const { return m_value; }
    //! Set accessor for the variable's unsigned int value
    void setValue(double value);
    //! Cast operator, required to automatically coerce a AVDomDouble to a uint.
    // Removed because it violates sa-40
    //operator double() const { return value(); }
    //! Assignment operator for a double rhs.
    AVDomDouble& operator=(double rhs) { setValue(rhs); return *this; }
protected:
    //! Compute the uint value from the specified text.
    void computeValue(const QString& text, QDomElement& element) override;

private:
    //! Copy-constructor: defined but not implemented
    AVDomDouble(const AVDomDouble& rhs);
    //! Assignment operator: defined but not implemented
    AVDomDouble& operator=(const AVDomDouble& rhs);
private:
    double m_value; //!< the unsigned int value
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents a string list variable that is connected to a
//! QDomElement.

class AVLIB_EXPORT AVDomStringListSep : public AVDomString
{
    Q_OBJECT
public:
    //! Construct a new instance, and add it to its parent's list of variables
    AVDomStringListSep(
        AVDomVariable *parent, const QString& tagName, bool autoCreate,
        const QString& sep = " ", const char *member = 0);
    //! Accessor for the variable's QStringList value
    const QStringList& value() const { return m_value; }
    //! Set accessor for the variable's QStringList value
    void setValue(const QStringList& value);
    //! Answer the number of elements in the list
    uint count() const { return m_value.count(); }
    //! Answer whether the string list is empty
    bool isEmpty() const { return m_value.isEmpty(); }
    //! Cast operator, required to automatically coerce an AVDomStringListSep to
    //! a QStringList.
    // Removed because it violates sa-40
    //operator const QStringList&() const { return value(); }
    //! Answer the string used to separate list entries
    const QString& separator() const { return m_sep; }
protected:
    //! Compute the QStringList value from the specified text.
    void computeValue(const QString& text, QDomElement& element) override;

private:
    //! Copy-constructor: defined but not implemented
    AVDomStringListSep(const AVDomStringListSep& rhs);
    //! Assignment operator: defined but not implemented
    AVDomStringListSep& operator=(const AVDomStringListSep& rhs);
private:
    QString     m_sep;   //!< the separator used to split the text
    QStringList m_value; //!< the string list
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents a point variable that is connected to a QDomElement.

class AVLIB_EXPORT AVDomPoint : public AVDomString
{
    Q_OBJECT
public:
    static QString pointToString(const QPoint& value);
public:
    //! Construct a new instance, and add it to its parent's list of variables
    AVDomPoint(AVDomVariable *parent, const QString& tagName, bool autoCreate, const QPoint& defaultValue = QPoint());
    //! Accessor for the variable's point value
    const QPoint& value() const { return m_point; }
    //! Set accessor for the variable's point value
    void setValue(const QPoint& value);
    //! Accessor for x
    int x() const { return m_point.x(); }
    //! Accessor for y
    int y() const { return m_point.y(); }
    //! Cast operator, required to automatically coerce a AVDomPoint to a QPoint.
    // Removed because it violates sa-40
    //operator const QPoint&() const { return value(); }
    //! Assignment operator for a QPoint rhs.
    AVDomPoint& operator=(const QPoint& rhs) { setValue(rhs); return *this; }
protected:
    //! Compute the point value from the specified text.
    void computeValue(const QString& text, QDomElement& element) override;

private:
    //! Copy-constructor: defined but not implemented
    AVDomPoint(const AVDomPoint& rhs);
    //! Assignment operator: defined but not implemented
    AVDomPoint& operator=(const AVDomPoint& rhs);
private:
    QPoint m_point; //!< the QPoint value
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents a size variable that is connected to a QDomElement.

class AVLIB_EXPORT AVDomSize : public AVDomString
{
    Q_OBJECT
public:
    static QString sizeToString(const QSize& value);
public:
    //! Construct a new instance, and add it to its parent's list of variables
    AVDomSize(AVDomVariable *parent, const QString& tagName, bool autoCreate, const QSize& defaultValue = QSize());
    //! Accessor for the variable's size value
    const QSize& value() const { return m_size; }
    //! Set accessor for the variable's size value
    void setValue(const QSize& value);
    //! Accessor for the width
    int width() const { return m_size.width(); }
    //! Accessor for the height
    int height() const { return m_size.height(); }
    //! Cast operator, required to automatically coerce a AVDomSize to a QSize.
    // Removed because it violates sa-40
    //operator const QSize&() const { return value(); }
    //! Assignment operator for a AVDomSize rhs.
    // do not use setValue since it requires a valid size
    AVDomSize& operator=(const AVDomSize& rhs) { setText(rhs.text()); return *this; }
    //! Assignment operator for a QSize rhs.
    AVDomSize& operator=(const QSize& rhs) { setValue(rhs); return *this; }
protected:
    //! Compute the size value from the specified text.
    void computeValue(const QString& text, QDomElement& element) override;

private:
    //! Copy-constructor: defined but not implemented
    AVDomSize(const AVDomSize& rhs);
private:
    QSize m_size; //!< the QSize value
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents a AVHashString variable that is connected to a QDomElement.

class AVLIB_EXPORT AVDomHashString : public AVDomString
{
    Q_OBJECT
public:
    //! Construct a new instance, and add it to its parent's list of variables
    AVDomHashString(
        AVDomVariable *parent, const QString& tagName, bool autoCreate,
        const char *member = 0, const QString& defaultText = "");
    //! Destructor
    ~AVDomHashString() override { delete m_string; }
    //! Accessor for the variable's size value
    const AVHashString& value() const;
    //! Assignment operator for a AVDomHashString rhs.
    AVDomHashString& operator=(const AVDomHashString& rhs) { setText(rhs.text()); return *this; }
    //! Assignment operator for a QString rhs.
    AVDomHashString& operator=(const QString& rhs) { setText(rhs); return *this; }
protected:
    //! Compute the AVHashString value from the specified text.
    void computeValue(const QString& text, QDomElement& element) override;

private:
    //! Copy-constructor: defined but not implemented
    AVDomHashString(const AVDomHashString& rhs);
private:
    AVHashString *m_string; //!< the AVHashString value
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents an object that is connected to a QDomElement. It
//! contains a list of child variables, which are connected to the children of
//! the QDomElement.

class AVLIB_EXPORT AVDomObject : public QObject, public AVDomVariable
{
    Q_OBJECT
public:
    //! Construct a new instance, and add it to its parent's list of variables
    AVDomObject(AVDomVariable *parent, const QString& tagName, bool autoCreate);
    //! When a new instance is created, this method will be called for the
    //! parent, allowing it to register it's new child instance.
    void addVariable(AVDomVariable& var) override;
    //! Read the object's value from the QDomElement.
    void readFromElement(QDomElement& element) override;
    //! Create the QDomElement for this variable
    void createElement(QDomElement& parent, QDomElement& prev) override;
    //! This method gets called when the value of an owned variable changes. It
    //! emits signalChanged() and propagates the change to the parent.
    void setChanged(bool changed) override;
    //! Downcast method
    AVDomObject* toDomObject() override { return this; }

protected:
    //! This method creates the QDomElements for all child variables whose
    //! autoCreate flag is true.
    void autoCreateElements();
signals:
    void signalChanged();
private:
    //! Copy-constructor: defined but not implemented
    AVDomObject(const AVDomObject& rhs);
    //! Assignment operator: defined but not implemented
    AVDomObject& operator=(const AVDomObject& rhs);
private:
    QList<AVDomVariable*> m_variables; //!< the list of child variables
};

///////////////////////////////////////////////////////////////////////////////

//! This is a subclass of AVDomObject that can read/write itself from an XML
//! configuration file. It can be combined with the template class
//! AVDomConfigSingleton from "avexplicitsingleton.h".

class AVLIB_EXPORT AVDomConfig : public AVDomObject
{
    Q_OBJECT
public:
    explicit AVDomConfig(const QString& tagName, bool autoCreate = false);
    const QString& fileName() const { return m_fileName; }
    //! \param fileName the name of the config *without* ".xfg" extension!
    virtual void setFileName(const QString& fileName);
    const QString& fullPath() const { return m_fullPath; }
    bool changed() const { return m_changed; }
    void           readFromDocument(QDomDocument& doc) override;
    bool fileIsReadable() const;

    virtual void readFromFile();
    //! Failure to read from the current file is non-fatal with this function.
    //! \return True if reading succeeded, false otherwise (internal state is unchanged).
    bool readFromFileChecked();
    void readFromFile(const QString& fileName);

    QString writeToString();
    QByteArray writeToByteArray();
    void writeToFile();
    void writeToFile(const QString& fileName);
    //! This method gets called when the value of an owned variable changes. It
    //! stores the changed flag in m_changed, emits signalChanged(), and
    //! propagates the change to the parent.
    void setChanged(bool changed) override;

private:
    //! Copy-constructor: defined but not implemented
    AVDomConfig(const AVDomConfig& rhs);
    //! Assignment operator: defined but not implemented
    AVDomConfig& operator=(const AVDomConfig& rhs);
protected:
    QString      m_fileName;
    QString      m_fullPath;
private:
    QDomDocument m_doc;
    bool         m_changed;
};

///////////////////////////////////////////////////////////////////////////////

//! This template class is a QList of AVDomVariables that is able to sort
//! its elements. An arbitrary child tag (QDomElement) of the element can be
//! used as a sort criteria.
//! This class is meant to be private to AVDomList, and should not be used
//! otherwise.

template<class T>
class AVSortableDomVariableList : public QList<T *>
{
public:
    //! Sort the list using the specified sort criteria.
    //! \param listElement is the QDomElement of the list itself
    //! \param tagName     is the tagName to be used for sorting
    void sortByElementTagName(QDomElement& listElement, const QString& tagName)
    {
        AVASSERT(!tagName.isNull());
        AVASSERT(m_tagName.isNull());
        m_tagName = tagName;
        qSort(this->begin(), this->end(), [this](T* item1, T* item2) { return this->itemsLessThan(item1, item2); });
        sortDomElements(listElement);
        m_tagName = QString();
    }

    //! Sort the DOM elements such that the sequence matches the AVDomVariables in this list.
    void sortDomElements(QDomElement& listElement)
    {
        QDomElement prev;
        typename QList<T *>::const_iterator it(this->begin());
        while (it != this->end()) {
           T *obj = (*it);
            QDomElement& element = obj->element();
            listElement.insertAfter(element, prev);
            prev = element;
            ++it;
        }
    }
protected:
    //! For the specified AVDomVariable, answer the contents of the tag specified in m_tagName.
    QString textForItem(T *obj) {
        AVASSERT(obj != 0);
        QDomNodeList nodes = obj->element().elementsByTagName(m_tagName);
        AVASSERT(nodes.count() == 1);
        QDomNode node = nodes.item(0);
        QDomElement element = node.toElement();
        AVASSERT(!element.isNull());
        return element.text();
    }
    //! For the specified AVDomVariable, answer the contents of the tag specified in m_tagName.
    bool itemsLessThan(T* item1, T* item2) {

        if (m_tagName.isNull()) return item1 < item2;

        return textForItem(item1) < textForItem(item2);

    }
private:
    QString m_tagName;
};

///////////////////////////////////////////////////////////////////////////////

//! This template class represents a list of AVDomVariables that is connected
//! to a QDomElement.

template<class T>
class AVDomList : public AVDomVariable
{
public:
    //! Construct a new instance, and add it to its parent's list of variables
    AVDomList(
        AVDomVariable *parent, const QString& tagName, bool autoCreate,
        const QString& elementTagName)
        : AVDomVariable(parent, tagName, autoCreate),
        m_elementTagName(elementTagName),
        m_selectedObject(0)
    {
    }

    //! Destructor
    ~AVDomList() override { qDeleteAll(m_list); }

    //! Returns the tag name of each object element
    QString elementTagName() const { return m_elementTagName; }

    //! Accessor for the list
    QList<T*>& list() { return m_list; }
    //! Const Accessor for the list.
    const QList<T*>& list() const { return m_list; }

    //! Answer whether the list is empty.
    bool isEmpty() const { return m_list.isEmpty(); }
    //! Answer the number of elements in the list.
    uint count() const { return m_list.count(); }

    //! Sort the list using the specified sort criteria.
    //! \param tagName is the tagName to be used for sorting
    void sortByElementTagName(const QString& tagName)
    {
        m_list.sortByElementTagName(element(), tagName);
        setChanged(true);
    }

    //! Sort the DOM elements such that the sequence matches the elements in this list.
    void sortDomElements()
    {
        m_list.sortDomElements(element());
        setChanged(true);
    }

    //! Create a new element. Please note that for AVDomDicts, the dictionary is
    //! not kept in sync when an element is added using this method.
    T *newObject()
    {
        T *obj = new T(this, m_elementTagName, true);
        AVASSERT(obj != 0);
        QDomElement& e = element();
        if (e.isNull()) {
            AVDomVariable *p = parentVariable();
            AVASSERT(p != 0);
            QDomElement prev;
            createElement(p->element(), prev);
            AVASSERT(!e.isNull());
        }
        QDomElement prev;
        obj->createElement(e, prev);
        addObject(obj);
        setChanged(true);
        return obj;
    }

    //! Delete an existing element. Please note that for AVDomDicts, the
    //! dictionary is not kept in sync when an element is removed using this
    //! method.
    void deleteObject(T *obj)
    {
        removeObject(obj);
        setChanged(true);
    }

    void clear()
    {
        m_selectedObject = 0;
        qDeleteAll(m_list);
        m_list.clear();
        setChanged(true);
    }

    T *selectedObject()
    {
        return m_selectedObject;
    }

    void setSelectedObject(T *selectedObject)
    {
        m_selectedObject = selectedObject;
        signalSelection();
    }

    //! Read the list's elements from the QDomElement.
    void readFromElement(QDomElement& element) override
    {
        m_selectedObject = 0;
        qDeleteAll(m_list);
        m_list.clear();
        AVDomVariable::readFromElement(element);
        readList(element);
    }

    //! Read the list's elements from the QDomElement, without clearing the list first.
    //! The DOM element is not registered with the list, so editing/saving will not work.
    void readList(QDomElement& element)
    {
        QDomNode n = element.firstChild();
        while (!n.isNull()) {
            checkParent(n, element);
            QDomElement e = n.toElement();
            if (e.tagName() == m_elementTagName) {
                T *obj = newObject(e);
                AVASSERT(obj != 0);
            } else ignore(n);
            n = n.nextSibling();
        }
    }

    //! WARNING! Don't use this iterator to remove (elements will not be deleted)
    class Iterator : public QListIterator<T*>
    {
    public:
        explicit Iterator(const AVDomList<T>& list)
        : QListIterator<T*>(list.list()) { }
    };

protected:
    //! Create a new element and read its value from the specified QDomElement.
    T *newObject(QDomElement& element)
    {
        T *obj = new T(this, m_elementTagName, true);
        AVASSERT(obj != 0);
        obj->readFromElement(element);
        addObject(obj);
        return obj;
    }

    //! Add a new object to the list.
    virtual void addObject(T *obj)
    {
        AVASSERT(obj != 0);
        m_list.append(obj);
        if (m_selectedObject == 0) m_selectedObject = obj;
    }

    //! Remove an object from the list.
    virtual void removeObject(T *obj)
    {
        AVASSERT(obj != 0);
        if (m_selectedObject == obj) m_selectedObject = 0;
        int index = m_list.indexOf(obj);
        AVASSERT(index >= 0);
        delete m_list.takeAt(index);
    }
private:
    //! Copy-constructor: defined but not implemented
    AVDomList<T>(const AVDomList<T>& rhs);
    //! Assignment operator: defined but not implemented
    AVDomList<T>& operator=(const AVDomList<T>& rhs);
private:
    QString                       m_elementTagName; //!< the XML tag for the list's elements
    AVSortableDomVariableList<T>  m_list;
    T*                            m_selectedObject;
};

///////////////////////////////////////////////////////////////////////////////

//! This template class represents an dictionary of AVDomVariables that is
//! connected to a QDomElement. The method dictKey() is used to determine the
//! key of each AVDomVariable that is added to the dictionary.

template<class T>
class AVDomDict : public AVDomList<T>
{
public:
    //! Construct a new instance, and add it to its parent's list of variables
    AVDomDict(AVDomVariable *parent, const QString& tagName, bool autoCreate, const QString& elementTagName)
        : AVDomList<T>(parent, tagName, autoCreate, elementTagName) { }

    //! Destructor
    //! TODO should not be necessary; only here to make Parasoft C++ Test happy
    virtual ~AVDomDict() { }

    //! Accessor for the dictionary
    QHash<QString, T*>& dict() { return m_dict; }

    //! Const Accessor for the dictionary.
    const QHash<QString, T*>& dict() const { return m_dict; }

    //! Read the list's elements from the QDomElement.
    virtual void readFromElement(QDomElement& element)
    {
        m_dict.clear();
        AVDomList<T>::readFromElement(element);
    }
protected:
    //! Add a new object to the list.
    //! Tries to add the object to the dictionary, too.
    virtual void addObject(T *obj)
    {
        AVASSERT(obj != 0);
        QString dictKey = obj->dictKey();
        if (!dictKey.isNull()) {
            if (!m_dict.contains(dictKey))
                m_dict.insert(dictKey, obj);
            else
                LOGGER_AVCOMMON_AVLIB.Write(
                    LOG_WARNING, "addObject %s already present", qPrintable(dictKey));
        } else
            LOGGER_AVCOMMON_AVLIB.Write(LOG_WARNING, "addObject dictKey is null");
        AVDomList<T>::addObject(obj);
    }

    //! Remove an object from the list.
    //! Tries to remove the object from the dictionary, too.
    virtual void removeObject(T *obj)
    {
        AVASSERT(obj != 0);
        QString dictKey = obj->dictKey();
        if (!m_dict.remove(dictKey))
            LOGGER_AVCOMMON_AVLIB.Write(LOG_WARNING, "removeObject %s not found", qPrintable(dictKey));
        AVDomList<T>::removeObject(obj);
    }
private:
    //! Copy-constructor: defined but not implemented
    AVDomDict<T>(const AVDomDict<T>& rhs);
    //! Assignment operator: defined but not implemented
    AVDomDict<T>& operator=(const AVDomDict<T>& rhs);
private:
    QHash<QString, T*> m_dict; //!< the dictionary of objects
};

///////////////////////////////////////////////////////////////////////////////

//! This class represents a list of AVDomStrings.

class AVLIB_EXPORT AVDomStringList : public AVDomList<AVDomString>
{
public:
    AVDomStringList(
        AVDomVariable *parent, const QString& tagName, bool autoCreate,
        const QString& elementTagName)
        : AVDomList<AVDomString>(parent, tagName, autoCreate, elementTagName) { }
    //! Accessor for the variable's QStringList value
    const QStringList& value() const { return m_value; }
    //! Answer whether the string list is empty
    bool isEmpty() const { return m_value.isEmpty(); }
    //! Cast operator, required to automatically coerce a AVDomStringList to
    //! a QStringList.
    // Removed because it violates sa-40
    //operator const QStringList&() const { return value(); }
    //! Read the variable's value from the QDomElement.
    void readFromElement(QDomElement& element) override;

private:
    QStringList m_value; //!< the string list
};

#endif // AVDOMVARIABLES_H

// End of file
