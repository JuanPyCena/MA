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
    \author  Dietmar G�sseringer, d.goesseringer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com

    String dictionary helper class.
*/

#if !defined(AVSTRINGMAP_H_INCLUDED)
#define AVSTRINGMAP_H_INCLUDED

// Qt includes
#include <QStringList>
#include <QMap>
#include <QString>

#include "avlib_export.h"

//////////////////////////////////////////////////////////////////////////////

typedef QMap<QString, QString>::ConstIterator AVStringMapConstIterator;
typedef QMap<QString, QString>::Iterator AVStringMapIterator;

//////////////////////////////////////////////////////////////////////////////

//! The AVStringMap class provides a dictionary of string elements.
/*!
    Inherits QMap <QString, QString>

    The class defines a dictionary with both keys and values of type QString.

    AVStringMap objects can be created from QStrings with a given separator,
    or from QStringLists. Odd elements are taken as keys, even elements as values.

    Example:

    \code
        QString str = "lat;42.9343;lon;12.1423;trackid;SDPS:120928:002";
        AVStringMap map = AVStringMap::from( ";", str );

        // Output:
        //  map["lat"] = "42.9343"
        //  map["lon"] = "12.1423"
        //  map["trackid"] = "SDPS:120928:002"
    \endcode
 */
class AVLIB_EXPORT AVStringMap : public QMap <QString, QString>
{

public:

    //! Constructs an empty string dictionary
    AVStringMap () { };

    //! Desytroys a string dictionary.
    ~AVStringMap () { };

    /*! \brief Creates and returns a new AVStringMap object from the given \p str
        string, using the given separator \p sep to identify the data elements.

        Odd data elements are taken as keys, even elements as values.
     */
    static AVStringMap from ( const QString &sep, const QString &str )
    {
        QStringList lst = str.split(sep, QString::KeepEmptyParts);
        return from(lst);
    }

    /*! \brief Creates and returns a new AVStringMap object from the given \p lst string list.

        Odd data elements are taken as keys, even elements as values.
     */
    static AVStringMap from ( const QStringList &lst )
    {
        AVStringMap map;

        QString tmp;
        for ( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ) {
            tmp = (*(it++));
            if (it == lst.end()) {
                map[tmp] = QString::null;
                break;
            }
            if (map.contains(tmp)) {
                AVLogger->Write(LOG_WARNING,
                    "AVStringMap::from: "
                    "Key '%s' already existent. Replacing old value '%s' with new '%s'...",
                    qPrintable(tmp), qPrintable(map[tmp]), qPrintable(*it));
            }
            map[tmp] = (*(it++));
        }
        return map;
    }

    /*! \brief Returns this AVStringMap as QString.

        \p sep1 is the separator used between the keyx and the value, \p sep2 is the
        separator used between key-value pairs.

        Example:

        \code
            QString str1 = "lat;42.9343;lon;12.1423;trackid;SDPS:120928:002";
            AVStringMap map = AVStringMap::from( ";", str1 );
            QString str2 = map.toString(",","|");

            // Output:
            //  str2 = "lat,42.9343|lon,12.1423|trackid,SDPS:120928:002|
        \endcode
     */
    QString toString ( const QString &sep1 = ";", const QString &sep2 = ";" ) const
    {
        QString str;

        for ( AVStringMapConstIterator it = begin(); it != end(); ++it)
        {
            str += it.key() + sep1 + it.value() + sep2;
        }

        return str;
    }

    /*! \brief Returns a AVStringMap construsted from a QString as QString.
     */
    static QString asString (
        const QString &str,
        const QString &insep = ";",
        const QString &outsep1 = ";",
        const QString &outsep2 = ";" )
    {
        QStringList lst = str.split(insep, QString::KeepEmptyParts);

        QString ret;
        uint count = 0;
        for (QStringList::ConstIterator it = lst.begin(); it != lst.end(); )
        {
            ret += *it;
            if (++it == lst.end()) break;
            if (++count % 2)
                ret += outsep1;
            else
                ret += outsep2;
        }

        return ret;
    }
};

#endif

// End of file
