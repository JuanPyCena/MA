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
    \author  Christian Muschick, c.muschick@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   Classes to handle config parameters which can be freely partitioned among config files.
 */

///////////////////////////////////////////////////////////////////////////////

#if !defined AVCONFIG2_ADAPTERS_H_INCLUDED
#define AVCONFIG2_ADAPTERS_H_INCLUDED

#include <QString>
#include <QVariant>

#include "avlib_export.h"
#include "avlog.h"

class AVConfig2Container;
class AVConfig2Metadata;

///////////////////////////////////////////////////////////////////////////////
/**
 *  Adapter class for config parameters used in xfg files (dom config and rules).
 */
class AVLIB_EXPORT AVConfig2XmlAdapter
{
public:
    AVConfig2XmlAdapter(const AVConfig2Container& container);

    //! Replaces all occurrences of {config:paramName} with the contents of the loaded parameter.
    //! If the replacement text contains nested parameters, these will be replaced as well.
    QString replaceLoadedParameters(const QString& text) const;

private:

    //! Answer a string representation that is suitable for parameter replacement in XML config files
    //! \return QString::null if the conversion was not successful (unknown data type or parsing error),
    //!         the parameter string representation suitable for rules otherwise.
    static QString getCanonicalString(const QString& value, const AVConfig2Metadata& metadata);

    const AVConfig2Container& m_container;
};

///////////////////////////////////////////////////////////////////////////////
/**
 *  Adapter class for config parameters used in QVariant based environments (qml, qtscript, etc.)
 */
class AVLIB_EXPORT AVConfig2QVariantAdapter
{
public:

    /*!
     *  Indicates if the searched parameter exists
     *  \return Returns true if the parameter exists, false otherwise
     */
    static bool hasParamValue(const QString& param_name);

    /*! Searches for the given param_name and, if found, returns the value as a variant. Nested Containers are returned
     *  as nested Qvariants.
     *  (e.g. QList<QMap<QString, QString> > is returned as QVariant<QList<QVariant<QMap<QString, QVariant> > > >)
     *  Note: Only built-in QVariant types are used as QVariant values.
     *  Example: QMap<QString, QStringList> will be converted to QMap<QString, QVariant> whereby the QVariant values
     *  in the map are itself of type QVariantList.
     *  Attention: This method exits with a LOG FATAL if the parameter is not existing. Use hasParamValue() if the
     *  requested parameter is configured optionally.
     */
    static QVariant getParamValue(const QString& param_name);

private:
    //! Internal method to convert the primitive types that fit into a QVariant
    template<class T>
    static bool convertParam(const QString& value, QVariant& result);

    // No instances needed
    AVConfig2QVariantAdapter();
    ~AVConfig2QVariantAdapter();
    Q_DISABLE_COPY(AVConfig2QVariantAdapter);
};

///////////////////////////////////////////////////////////////////////////////

template<class T>
bool AVConfig2QVariantAdapter::convertParam(const QString& value, QVariant& result)
{
    T typed_result;
    bool success = AVFromString(value, typed_result);
    if(success)
        result = QVariant(typed_result);
    return success;
}

#endif

// End of file
