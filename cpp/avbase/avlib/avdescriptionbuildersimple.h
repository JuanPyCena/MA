///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author   Alexander Terbu, a.terbu@avibit.com
    \author   QT4-PORT: Karlheinz Wohlmuth, k.wohlmuth@avibit.com
    \brief    This file declares a simple description builder with values only.
*/

#ifndef AVDESCRIPTIONBUILDERSIMPLE_H
#define AVDESCRIPTIONBUILDERSIMPLE_H

// Qt includes
#include <qstring.h>
#include <qstringlist.h>

// AVCOMMON includes
#include "avlib_export.h"
#include "avdescriptionbuilder.h"

// forward declarations
class QDateTime;

///////////////////////////////////////////////////////////////////////////////

//! This class can be used to format values in a tabular form
class AVLIB_EXPORT AVDescriptionBuilderSimple : public AVDescriptionBuilder
{
public:
    //! Creates a new AVDescriptionBuilderSimple
    /*! \param cols number of columns to use
        \param groupCols number of values which should be grouped per column.
        \param colorName name of color to be used in rows,
               table frame uses the default color
        \param omitEmptyFields if set to true, all empty values are ommitted.
     */
    AVDescriptionBuilderSimple(
        uint cols, uint groupCols, const QString& colorName, bool omitEmptyFields=false);

    //! Called when the directory has finished calling describe
    /*! Builds the HTML table string

        \sa description()
    */
    void finalize() override;

    //! Returns all values in tabular form formatted in HTML
    /*! The values must have been previously added using the describe() methods.

        \sa describe()
     */
    QString description() const { return m_description; }

protected:
    void addValue(const QString& label, const QString& value) override;

private:
    //! Copy-constructor: defined but not implemented
    AVDescriptionBuilderSimple(const AVDescriptionBuilderSimple& rhs);
    //! Assignment operator: defined but not implemented
    AVDescriptionBuilderSimple& operator=(const AVDescriptionBuilderSimple& rhs);

protected:
    uint        m_cols;
    uint        m_groupCols;
    QString     m_colorName;
    QStringList m_list;
    QString     m_description;
};

#endif // AVDESCRIPTIONBUILDERSIMPLE_H

// End of file
