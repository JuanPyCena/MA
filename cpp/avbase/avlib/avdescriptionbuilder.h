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
    \author   Alexander Terbu, a.terbu@avibit.com
    \author   Stefan Kunz, skunz@avibit.com
    \author   Kevin Krammer, k.krammer@avibit.com
    \author   QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief    This file declares the description builder.
*/

#ifndef AVDESCRIPTIONBUILDER_H
#define AVDESCRIPTIONBUILDER_H

// Qt includes
#include <QString>
#include <QStringList>

// AVLIB includes
#include "avlib_export.h"
#include "avmacros.h"

// forward declarations
class QDateTime;
class QTime;

///////////////////////////////////////////////////////////////////////////////

//! This class declares and interface for label/value pair formatters
class AVLIB_EXPORT AVDescriptionBuilder
{
    AVDISABLECOPY(AVDescriptionBuilder);
public:
    //! Creates a new AVDescriptionBuilder
    /*! \param omitEmptyFields if set to true, all empty values are ommitted.
     */
    explicit AVDescriptionBuilder(bool omitEmptyFields = false);

    virtual ~AVDescriptionBuilder() {}

    //! Append the description of one field value to the result.
    /*! \param label     the name of the field
        \param value     the value of the field
    */
    virtual void describe(const QString& label, const QString& value);

    //! Append the description of one field value to the result.
    /*! \param label     the name of the field
        \param value     the value of the field
    */
    virtual void describe(const QString& label, const QDateTime& value);

    //! Append the description of one field value to the result.
    /*! Note: The constructor parameter omitEmptyFields is ignored in this method
        since a QTime is never uninitialized.
        \param label     the name of the field
        \param value     the value of the field
    */
    virtual void describe(const QString& label, const QTime& value);

    //! Append the description of one field value to the result.
    /*! \param label     the name of the field
        \param value     the value of the field
    */
    virtual void describe(const QString& label, bool value);

    //! Called when the directory has finished calling describe
    /*! Subclasses can override this to perfom transformation into their final
        formatting
    */
    virtual void finalize() {}

protected:
    bool m_omitEmptyFields;

protected:
    virtual void addValue(const QString& label, const QString& value) = 0;
};

///////////////////////////////////////////////////////////////////////////////

//! This class can be used to format label/value pairs in a tabular form
class AVLIB_EXPORT AVHtmlTableDescriptionBuilder : public AVDescriptionBuilder
{
public:
    //! Creates a new AVHtmlTableDescriptionBuilder
    /*! \param cols number of columns to use
        \param colorName name of color to be used in even row numbers,
               odd row numbers use the default color
        \param omitEmptyFields if set to true, all empty label/value
               pairs that are confirmed are ommitted.
     */
    AVHtmlTableDescriptionBuilder(
        uint cols, const QString& colorName, bool omitEmptyFields=false);

    //! Called when the directory has finished calling describe
    /*! Builds the HTML table string

        \sa description()
    */
    void finalize() override;

    //! Returns all label/value pairs in tabular form formatted in HTML
    /*! The label/value pairs must have been previously added using the
        describe() methods.

        \sa describe()
     */
    QString description() const { return m_description; }

protected:
    //! Returns the next HTML <TR> tag to be used
    QString trTag(uint row);

    void addValue(const QString& label, const QString& value) override;

private:
    //! Copy-constructor: defined but not implemented
    AVHtmlTableDescriptionBuilder(const AVHtmlTableDescriptionBuilder& rhs);
    //! Assignment operator: defined but not implemented
    AVHtmlTableDescriptionBuilder& operator=(const AVHtmlTableDescriptionBuilder& rhs);

protected:
    uint        m_cols;
    QString     m_colorName;
    QStringList m_list;
    QString     m_description;
};

#endif // AVDESCRIPTIONBUILDER_H

// End of file
