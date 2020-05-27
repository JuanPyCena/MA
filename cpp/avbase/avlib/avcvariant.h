///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author    Dr. Thomas Leitner, t.leitner@avibit.com
    \author    QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief     Network transparent configuration management classes.

    This file contains the AVConfig class as well as all associated helper
    classes.
*/

#if !defined AVCVARIANT_H_INCLUDED
#define AVCVARIANT_H_INCLUDED

// System includes
#include <iostream>    // solaris 10 needs this for cxxxx includes to work
#include <cstdlib>

// Qt includes
#include <QColor>
#include <QDataStream>
#include <QFont>
#include <QMap>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QList>
#include <QtDebug>

//#include "avconfiglist.h"

#include "avlib_export.h"
#include "avenvironment.h"
#include "avlog.h"

//! the AVIntList used in the AVCVariant to store integer lists.
typedef QList<int> AVIntList;

///////////////////////////////////////////////////////////////////////////////
//! Holds references to different config data types
/*! In the AVConfigBase class it is possible to save and retrieve
    configuration data consisting of many different variable types. In order
    to facilitate the treatment of all those variable types as one entity, the
    AVCVariant class is used. It basically can hold variables as different as
    ints, Bools, Strings or Fonts. Which variable it holds, depends on the
    constructor which was used to create the AVCVariant object.
*/

class AVLIB_EXPORT AVCVariant
{
public:
    //! Defines the different variable types we support here.
    enum Type { Undef, Int, UInt, Double, Bool, String, Rect, Color, Size,
                Point, StringList, IntList, Font, Directory, File };

    //! constructs an empty, untyped variant.
    AVCVariant()               { t = Undef;      va = 0; ah = false;   }
    //! destroys the variant and deletes any dynamic data
    virtual ~AVCVariant()      { if (va != 0 && ah) delObject();       }

    //parasoft suppress item init-06
    //! constructs a variant holding an int.
    explicit AVCVariant(int *v)         { setAddr(v); }
    //! constructs a variant holding an uint.
    explicit AVCVariant(uint *v)        { setAddr(v); }
    //! constructs a variant holding a double.
    explicit AVCVariant(double *v)      { setAddr(v); }
    //! constructs a variant holding a bool.
    explicit AVCVariant(bool *v)        { setAddr(v); }
    //! constructs a variant holding a QString.
    explicit AVCVariant(QString *v, Type tt=String)     { setAddr(v,tt); }
    //! constructs a variant holding a QRect.
    explicit AVCVariant(QRect *v)       { setAddr(v); }
    //! constructs a variant holding a QColor.
    explicit AVCVariant(QColor *v)      { setAddr(v); }
    //! constructs a variant holding a QSize.
    explicit AVCVariant(QSize *v)       { setAddr(v); }
    //! constructs a variant holding a QPoint.
    explicit AVCVariant(QPoint *v)      { setAddr(v); }
    //! constructs a variant holding a QStringList.
    explicit AVCVariant(QStringList *v) { setAddr(v); }
    //! constructs a variant holding a AVInstList.
    explicit AVCVariant(AVIntList *v)   { setAddr(v); }
    //! constructs a variant holding a QFont.
    explicit AVCVariant(QFont *v)       { setAddr(v); }
    //parasoft on

    //! create a variant as a copy of an existing variant. The source variant
    //! must have the same type as the target variant unless the target variant
    //! is untyped.
    AVCVariant(const AVCVariant &s) : t(Undef), va(0), ah(false) { copy(s); }
    //! assign a variant to another variant by copying data. The source variant
    //! must have the same type as the target variant unless the target
    //! variant is untyped.
    AVCVariant &operator=(const AVCVariant &s)  {
        if (&s == this) return *this;
        copy(s);
        return *this;
    }
    //! used by the copy constructor and the assignment operator.
    /*! it basically copies the specified variant to the local variant.
        \note the source variant must have the same type as the target variant
        unless the target variant is untyped.
    */
    void copy(const AVCVariant &s);

    //! compare two AVCVariant's
    /*! Two AVCVariants are equal, if
        - both data pointers are 0 or
        - their type is the same and (asXXX == s.asXXX) is true
        - both types are Undef
     */
    bool compare(const AVCVariant &s) const;

    //! returns the value of the variant as an int
    int &asInt()
    {
        if (!checkType(Int))
            AVLogger->Write(LOG_ERROR, "AVCVariant:asInt");
        return *(reinterpret_cast<int*>(va));
    }

    //! returns the value of the variant as an uint
    uint &asUInt()
    {
        if (!checkType(UInt))
            AVLogger->Write(LOG_ERROR, "AVCVariant:asUInt");
        return *(reinterpret_cast<uint*>(va));
    }

    //! returns the value of the variant as a double
    double &asDouble()
    {
        if (!checkType(Double))
            AVLogger->Write(LOG_ERROR, "AVCVariant:asDouble");
        return *(reinterpret_cast<double*>(va));
    }

    //! returns the value of the variant as a bool
    bool &asBool()
    {
        if (!checkType(Bool))
            AVLogger->Write(LOG_ERROR, "AVCVariant:asBool");
        return *(reinterpret_cast<bool*>(va));
    }

    //! returns the value of the variant as a string
    QString &asString()
    {
        if (!checkType(String))
            AVLogger->Write(LOG_ERROR, "AVCVariant:asString");
        return *(reinterpret_cast<QString*>(va));
    }

    //! returns the value of the variant as a QRect
    QRect &asRect()
    {
        if (!checkType(Rect))
            AVLogger->Write(LOG_ERROR, "AVCVariant:asRect");
        return *(reinterpret_cast<QRect*>(va));
    }

    //! returns the value of the variant as a QColor
    QColor &asColor()
    {
        if (!checkType(Color))
            AVLogger->Write(LOG_ERROR, "AVCVariant:asColor");
        return *(reinterpret_cast<QColor*>(va));
    }

    //! returns the value of the variant as a QSize
    QSize &asSize()
    {
        if (!checkType(Size))
            AVLogger->Write(LOG_ERROR, "AVCVariant:asSize");
        return *(reinterpret_cast<QSize*>(va));
    }

    //! returns the value of the variant as a QPoint
    QPoint &asPoint()
    {
        if (!checkType(Point))
            AVLogger->Write(LOG_ERROR, "AVCVariant:asPoint");
        return *(reinterpret_cast<QPoint*>(va));
    }

    //! returns the value of the variant as a QStringList
    QStringList &asStringList()
    {
        if (!checkType(StringList))
            AVLogger->Write(LOG_ERROR, "AVCVariant:asStringList");
        return *(reinterpret_cast<QStringList*>(va));
    }

    //! returns the value of the variant as a AVIntList
    AVIntList &asIntList()
    {
        if (!checkType(IntList))
            AVLogger->Write(LOG_ERROR, "AVCVariant:asIntList");
        return *(reinterpret_cast<AVIntList*>(va));
    }

    //! returns the value of the variant as a QFont
    QFont &asFont()
    {
        if (!checkType(Font))
            AVLogger->Write(LOG_ERROR, "AVCVariant:asFont");
        return *(reinterpret_cast<QFont*>(va));
    }

    //! returns the value of the variant as a QString
    QString &asDirectory()
    {
        if (!checkType(Directory))
            AVLogger->Write(LOG_ERROR, "AVCVariant:asDirectory");
        return *(reinterpret_cast<QString*>(va));
    }

    //! returns the value of the variant as a QString
    QString &asFile()
    {
        if (!checkType(File))
            AVLogger->Write(LOG_ERROR, "AVCVariant:asFile");
        return *(reinterpret_cast<QString*>(va));
    }

    //! returns the value of the variant as an int
    /*! \note the behaviour is different from the non-const getter. If the type
              of the variant (e.g. Undef) is not the same as returned by this
              getter function the function fails with an AVASSERT.
     */
    const int &asIntConst() const
    {
        AVASSERT(type() == Int);
        return *(reinterpret_cast<int*>(va));
    }

    //! returns the value of the variant as an uint
    /*! \note the behaviour is different from the non-const getter. If the type
              of the variant (e.g. Undef) is not the same as returned by this
              getter function the function fails with an AVASSERT.
     */
    const uint &asUIntConst() const
    {
        AVASSERT(type() == UInt);
        return *(reinterpret_cast<uint*>(va));
    }

    //! returns the value of the variant as a double
    /*! \note the behaviour is different from the non-const getter. If the type
              of the variant (e.g. Undef) is not the same as returned by this
              getter function the function fails with an AVASSERT.
     */
    const double &asDoubleConst() const
    {
        AVASSERT(type() == Double);
        return *(reinterpret_cast<double*>(va));
    }

    //! returns the value of the variant as a bool
    /*! \note the behaviour is different from the non-const getter. If the type
              of the variant (e.g. Undef) is not the same as returned by this
              getter function the function fails with an AVASSERT.
     */
    const bool &asBoolConst() const
    {
        AVASSERT(type() == Bool);
        return *(reinterpret_cast<bool*>(va));
    }

    //! returns the value of the variant as a string
    /*! \note the behaviour is different from the non-const getter. If the type
              of the variant (e.g. Undef) is not the same as returned by this
              getter function the function fails with an AVASSERT.
     */
    const QString &asStringConst() const
    {
        AVASSERT(type() == String);
        return *(reinterpret_cast<QString*>(va));
    }

    //! returns the value of the variant as a QRect
    /*! \note the behaviour is different from the non-const getter. If the type
              of the variant (e.g. Undef) is not the same as returned by this
              getter function the function fails with an AVASSERT.
     */
    const QRect &asRectConst() const
    {
        AVASSERT(type() == Rect);
        return *(reinterpret_cast<QRect*>(va));
    }

    //! returns the value of the variant as a QColor
    /*! \note the behaviour is different from the non-const getter. If the type
              of the variant (e.g. Undef) is not the same as returned by this
              getter function the function fails with an AVASSERT.
     */
    const QColor &asColorConst() const
    {
        AVASSERT(type() == Color);
        return *(reinterpret_cast<QColor*>(va));
    }

    //! returns the value of the variant as a QSize
    /*! \note the behaviour is different from the non-const getter. If the type
              of the variant (e.g. Undef) is not the same as returned by this
              getter function the function fails with an AVASSERT.
     */
    const QSize &asSizeConst() const
    {
        AVASSERT(type() == Size);
        return *(reinterpret_cast<QSize*>(va));
    }

    //! returns the value of the variant as a QPoint
    /*! \note the behaviour is different from the non-const getter. If the type
              of the variant (e.g. Undef) is not the same as returned by this
              getter function the function fails with an AVASSERT.
     */
    const QPoint &asPointConst() const
    {
        AVASSERT(type() == Point);
        return *(reinterpret_cast<QPoint*>(va));
    }

    //! returns the value of the variant as a QStringList
    /*! \note the behaviour is different from the non-const getter. If the type
              of the variant (e.g. Undef) is not the same as returned by this
              getter function the function fails with an AVASSERT.
     */
    const QStringList &asStringListConst() const
    {
        AVASSERT(type() == StringList);
        return *(reinterpret_cast<QStringList*>(va));
    }

    //! returns the value of the variant as a AVIntList
    /*! \note the behaviour is different from the non-const getter. If the type
              of the variant (e.g. Undef) is not the same as returned by this
              getter function the function fails with an AVASSERT.
     */
    const AVIntList &asIntListConst() const
    {
        AVASSERT(type() == IntList);
        return *(reinterpret_cast<AVIntList*>(va));
    }

    //! returns the value of the variant as a QFont
    /*! \note the behaviour is different from the non-const getter. If the type
              of the variant (e.g. Undef) is not the same as returned by this
              getter function the function fails with an AVASSERT.
     */
    const QFont &asFontConst() const
    {
        AVASSERT(type() == Font);
        return *(reinterpret_cast<QFont*>(va));
    }

    //! returns the value of the variant as a QString
    /*! \note the behaviour is different from the non-const getter. If the type
              of the variant (e.g. Undef) is not the same as returned by this
              getter function the function fails with an AVASSERT.
     */
    const QString &asDirectoryConst() const
    {
        AVASSERT(type() == Directory);
        return *(reinterpret_cast<QString*>(va));
    }

    //! returns the value of the variant as a QString
    /*! \note the behaviour is different from the non-const getter. If the type
              of the variant (e.g. Undef) is not the same as returned by this
              getter function the function fails with an AVASSERT.
     */
    const QString &asFileConst() const
    {
        AVASSERT(type() == File);
        return *(reinterpret_cast<QString*>(va));
    }

    //! sets the contents of the variant to the specified int
    void set(const int &v)
    {
        if (checkType(Int))
            *(reinterpret_cast<int*>(va)) = v;
        else
            AVLogError << "AVCVariant:set(Int): Could not set value "  <<  v;
    }

    //! sets the contents of the variant to the specified uint
    void set(const uint &v)
    {
        if (checkType(UInt))
            *(reinterpret_cast<uint*>(va)) = v;
        else
            AVLogError << "AVCVariant:set(uInt): Could not set value " << v;
    }

    //! sets the contents of the variant to the specified double
    void set(const double &v)
    {
        if (checkType(Double))
            *(reinterpret_cast<double*>(va)) = v;
        else
            AVLogError << "AVCVariant:set(Double): Could not set value " << v;
    }

    //! sets the contents of the variant to the specified bool
    void set(const bool &v)
    {
        if (checkType(Bool))
            *(reinterpret_cast<bool*>(va)) = v;
        else
            AVLogError << "AVCVariant:set(Bool): Could not set value " << v;
    }

    //! sets the contents of the variant to the specified QString
    void set(const QString &v, Type tt=String)
    {
        if (checkType(tt))
            *(reinterpret_cast<QString*>(va)) = v;
        else
            AVLogError << "AVCVariant:set(QString): Could not set value " << v;
    }

    //! sets the contents of the variant to the specified QRect
    void set(const QRect &v)
    {
        if (checkType(Rect))
            *(reinterpret_cast<QRect*>(va)) = v;
        else
            AVLogError << "AVCVariant:set(QRect): Could not set value";
    }

    //! sets the contents of the variant to the specified QColor
    void set(const QColor &v)
    {
        if (checkType(Color))
            *(reinterpret_cast<QColor*>(va)) = v;
        else
            AVLogError << "AVCVariant:set(QColor): Could not set value";
    }

    //! sets the contents of the variant to the specified QSize
    void set(const QSize &v)
    {
        if (checkType(Size))
            *(reinterpret_cast<QSize*>(va)) = v;
        else
            AVLogError << "AVCVariant:set(QSize): Could not set value";
    }

    //! sets the contents of the variant to the specified QPoint
    void set(const QPoint &v)
    {
        if (checkType(Point))
            *(reinterpret_cast<QPoint*>(va)) = v;
        else
            AVLogger->Write(LOG_ERROR, "AVCVariant:set(QPoint): Could not set value");
    }

    //! sets the contents of the variant to the specified QStringList
    void set(const QStringList &v)
    {
        if (checkType(StringList))
            *(reinterpret_cast<QStringList*>(va)) = v;
        else
            AVLogger->Write(LOG_ERROR, "AVCVariant:set(QStringList): Could not set value");
    }

    //! sets the contents of the variant to the specified AVIntList
    void set(const AVIntList &v)
    {
        if (checkType(IntList))
            *(reinterpret_cast<AVIntList*>(va)) = v;
        else
            AVLogger->Write(LOG_ERROR, "AVCVariant:set(AVIntList): Could not set value");
    }

    //! sets the contents of the variant to the specified QFont
    void set(const QFont &v)
    {
        if (checkType(Font))
            copyFont(*(reinterpret_cast<QFont*>(va)), v);
        else
            AVLogger->Write(LOG_ERROR, "AVCVariant:set(QFont): Could not set value");
    }

    //! sets the contents of the variant to the specified variable represented
    //! as a string.
    /*! depending on the current variant type, the specified string is parsed
        and its content is saved in the variant.
    */
    void sets(const QString &v);
    //! deletes any dynamic data associated with the variant.
    void delObject();
    //! used by sets, this method parses one or more integers from a string
    void parseInt(const QString &v, int *v1, int *v2, int *v3 = 0, int *v4 = 0);
    //! used by sets, this method reads an interger list from a string
    void parseIntList(const QString &v, AVIntList &il);
    //! check if the current variant has got the specified type.
    /*! if there is a variant address currently specified, the method
        also outputs a warning message and returns false. If there is
        no variant address currently specified, memory for the new
        data type is allocated here.
        \param tt desired variant type to be checked or allocated.
        \return true if successful or false if not
    */
    bool checkType(Type tt);
    //! set the internal variable address to the specified int address.
    void setAddr(int *v)         { t = Int;        va = v; ah = false; }
    //! set the internal variable address to the specified uint address.
    void setAddr(uint *v)        { t = UInt;       va = v; ah = false; }
    //! set the internal variable address to the specified double address.
    void setAddr(double *v)      { t = Double;     va = v; ah = false; }
    //! set the internal variable address to the specified bool address.
    void setAddr(bool *v)        { t = Bool;       va = v; ah = false; }
    //! set the internal variable address to the specified QString address.
    void setAddr(QString *v, Type tt=String) { t = tt;     va = v; ah = false; }
    //! set the internal variable address to the specified QRect address.
    void setAddr(QRect *v)       { t = Rect;       va = v; ah = false; }
    //! set the internal variable address to the specified QColor address.
    void setAddr(QColor *v)      { t = Color;      va = v; ah = false; }
    //! set the internal variable address to the specified QSize address.
    void setAddr(QSize *v)       { t = Size;       va = v; ah = false; }
    //! set the internal variable address to the specified QPoint address.
    void setAddr(QPoint *v)      { t = Point;      va = v; ah = false; }
    //! set the internal variable address to the specified QStringList address.
    void setAddr(QStringList *v) { t = StringList; va = v; ah = false; }
    //! set the internal variable address to the specified AVIntList address.
    void setAddr(AVIntList *v)   { t = IntList;    va = v; ah = false; }
    //! set the internal variable address to the specified QFont address.
    void setAddr(QFont *v)       { t = Font;       va = v; ah = false; }

    //! write the variant to the specified datastream.
    /*! \return true if successful, false if not
     */
    bool write(QDataStream &s) const;
    //! read the variant from the specified datastream.
    /*! \return true if successful, false if not
     */
    bool read(QDataStream &s);
    //! write the variant to the specified textstream.
    /*! \return true if successful, false if not
     */
    bool write(QTextStream &s, bool skipType = false) const;
    //! write the variant to the specified textstream.
    /*! \return true if successful, false if not
     */
    bool write(QTextStream &s, AVCVariant &def, AVCVariant &min,
               AVCVariant &max, bool &defaultIsValid, QString &help,
               const QString &reference);
    //! read the variant from the specified textstream.
    /*! \return true if successful, false if not
     */
    bool read(QTextStream &s, AVCVariant &def, AVCVariant &min,
              AVCVariant &max, bool &defaultIsValid, QString &help,
              QString &reference);
    //! convert the value to a string
    QString toString() const;

    //! return the current type of the variant
    Type type() const { return t;   }

    //! return the address of the variable stored in the variant.
    const void *addr() const { return va;  }

    //! internal routine to read a line from a text stream.
    bool readLine(QTextStream &s, QString &l);

    //! internal routine to copy one QFont to another QFont.
    void copyFont(QFont &to, const QFont &from) { to = from; }

    //! returns the variant's type as a string
    inline const char * typeName() const { return typeName(t); }

    //! returns the given type as a string
    static const char* typeName(Type tt);

protected:
    Type t;         //!< the type of the current variant
    void *va;       //!< the address of the variable holding the data
    bool ah;        //!< true if the variable as allocated here.
};
//! writes the specified variant to the specified QDataStream
inline QDataStream &operator << (QDataStream &s, AVCVariant &msg ) {
    msg.write(s); return s;
}
//! reads the specified variant from the specified QDataStream
inline QDataStream &operator >> (QDataStream &s, AVCVariant &msg ) {
    msg.read(s); return s;
}

#endif

// End of file
