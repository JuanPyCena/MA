///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIBDEPRECATED - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Kevin Krammer, kevin.krammer@kdab.com
    \brief   provides some convenient functions for Qt3 compatible behavior
*/

#if !defined(AVQT3COMPAT_H_INCLUDED)
#define AVQT3COMPAT_H_INCLUDED

// Qt includes
#include <QString>
#include <QSize>
#include <QPen>

#include <cmath>

// local includes
#include "avlibdeprecated_export.h"

// forward declarations
class QAction;
class QColor;
class QDateTime;
class QIcon;
class QKeySequence;
class QObject;
class QPen;
class QTabletEvent;
class QTime;
class QWidget;
class QMouseEvent;
class QPixmap;
class QComboBox;
class QRect;

///////////////////////////////////////////////////////////////////////////////

namespace AVQt3Compat {

//! Returns \c true if Qt3's QTime would have been valid
/*! Qt3 QTime::isValid() was \c true for QTime(), a.k.a. QTime::isNull().
 *  Qt4 and above QTime().isValid() is \c false
 */
AVLIBDEPRECATED_EXPORT bool isValid(const QTime& time);

/*! In Qt3 pens were "comsmetic", i.e. a pen with a given line width always drew
 *  with that width independent of the QPainter's scaling.
 *  This function ensures that the Qt4/Qt5 pen is set to "cosmetic".
 */
//! Returns a cosmetic default pen
AVLIBDEPRECATED_EXPORT QPen createPen();
//! Returns a cosmetic copy of the given \p pen
AVLIBDEPRECATED_EXPORT QPen createPen(const QPen& pen);
//! Returns a cosmetic pen with the given properties
AVLIBDEPRECATED_EXPORT QPen createPen(Qt::PenStyle penStyle);
//! Returns a cosmetic pen with the given properties
AVLIBDEPRECATED_EXPORT QPen createPen(const QColor& color, uint width = 0, Qt::PenStyle penStyle = Qt::SolidLine);
//! Returns a cosmetic pen with the given properties
AVLIBDEPRECATED_EXPORT QPen createPen(const QColor& color, uint width, Qt::PenStyle penStyle, Qt::PenCapStyle capStyle, Qt::PenJoinStyle joinStyle);

AVLIBDEPRECATED_EXPORT QString qstringSection(const QString &text, const QString &separator, int start, int end = -1, QString::SectionFlags flags = QString::SectionDefault);

//! Returns a Qt3 behavior compatible value for QString::mid()
/*! Qt3 QString's mid() returned a null string if the string it was called on was empty.
 *  Qt5's QString::mid() only returns a null string if position is out of range.
 *  The rule processing framework relies on that, e.g. in AVStripField::computeFieldValueAndOverflow()
 *  when determining the "overflowText"
 */
AVLIBDEPRECATED_EXPORT QString qstringMid(const QString &text, int position, int n = -1);

//! Returns \c false when one of the two strings is null but the other isn't
/*! In Qt3 QString::operator==() would return \c false when isNull() of the two
 *  strings wasn't the same. From Qt4 on a null string and an empty string
 *  are considered to be equal.
 */
AVLIBDEPRECATED_EXPORT bool qstringEqual(const QString &s1, const QString &s2);
AVLIBDEPRECATED_EXPORT bool qstringEqual(const QString &s1, const QLatin1String &s2);
AVLIBDEPRECATED_EXPORT bool qstringEqual(const QString &s1, const char *s2);

/*! In Qt3 multiplying or dividing a QSize or a QPoint with a floating-point value
 * would use truncation to int rather than rounding, leading to a possible 1-pixel difference.
 * Using rounding (like Qt5 does) is for sure more correct, of course. Example:
 * anyone would expect (QSize(241,38) * 289.0 / 241) to return QSize(289, 46),
 * not QSize(288, 45). However the rounding leads to drawing artefacts in StripWidget,
 * so porting away from this requires more investigation/refactoring of the drawing.
 */
//! Equivalent to Qt3's size * factor
AVLIBDEPRECATED_EXPORT inline QSize multiplySize(const QSize &size, double factor)
{
    return QSize(qint32(size.width() * factor), qint32(size.height() * factor));
}
//! Equivalent to Qt3's size / factor
AVLIBDEPRECATED_EXPORT inline QSize divideSize(const QSize &size, double factor)
{
    return QSize(qint32(size.width() / factor), qint32(size.height() / factor));
}
//! Equivalent to Qt3's point * factor
AVLIBDEPRECATED_EXPORT inline QPoint multiplyPoint(const QPoint &point, double factor)
{
    return QPoint(qint32(point.x() * factor), qint32(point.y() * factor));
}
//! Equivalent to Qt3's point / factor
AVLIBDEPRECATED_EXPORT inline QPoint dividePoint(const QPoint &point, double factor)
{
    return QPoint(qint32(point.x() / factor), qint32(point.y() / factor));
}

//! Helper function to close and delete a widget
/*! In Qt3 QWidget::close() could be called with \c true as its argument
 *  which would delete the widget after closing it.
 *  The widget is only deleted if QWidget::close() returned \c true
 *  \return the value returned by QWidget::close()
 */
AVLIBDEPRECATED_EXPORT bool closeAndDelete(QWidget *w);
AVLIBDEPRECATED_EXPORT QWidget *parentWidgetSameWindow(QWidget *w);

//! Returns the tablet pressure as a scaled int
/*! In the patched Qt3 pressure was an int between 0 and 255, in Qt5 it is a qreal between 0.0 and 1.0
 *  Returns the "denormalized" value of the event's pressure
 */
AVLIBDEPRECATED_EXPORT int tabletPressure(QTabletEvent *e);

AVLIBDEPRECATED_EXPORT int mouseEventState(QMouseEvent *event);

//! Returns a string representation of the given datetime as needed for AVMsgEntry::printEntry() and writeTextEntry()
/*! In Qt3 null QDateTime was printed as all fields 0, in Qt5 it is empty
 */
AVLIBDEPRECATED_EXPORT QString printDateTime(const QDateTime &dt);

AVLIBDEPRECATED_EXPORT void setPaletteBackgroundPixmap(QWidget *w, const QPixmap &pix);

//! Creates a QAction with various preset parameters
/*! Compat helper for the following Qt3 constructor
 *  QAction ( const QString & text, const QString & menuText, QKeySequence accel, QObject * parent, const char * name = 0, bool toggle = FALSE )
 */
AVLIBDEPRECATED_EXPORT QAction *createAction(const QString& text, const QString& menuText,
                                             const QKeySequence& accel, QObject *parent, const char *name = 0, bool toggle = false);

//! Creates a QAction with various preset parameters
/*! Compat helper for the following Qt3 constructor
 *  QAction ( const QString & text, const QIconSet & icon, const QString & menuText, QKeySequence accel, QObject * parent, const char * name = 0, bool toggle = FALSE )
 */
AVLIBDEPRECATED_EXPORT QAction *createAction(const QString& text, const QIcon& icon, const QString& menuText,
                                             const QKeySequence& accel, QObject *parent, const char *name = 0, bool toggle = false);
//! Helper function to define current text in combobox as in qt3
AVLIBDEPRECATED_EXPORT void comboBoxSetCurrentText(QComboBox *combo, const QString &text);

//! Helper function to simulate QRect.unite method in qt3
/*! Returns:
 *  - rect1 if rect2 is invalid 
 *  - rect2 if rect1 is invalid and rect2 is valid
 *  - the uniting rectangle otherwise
 */
AVLIBDEPRECATED_EXPORT QRect uniteRectangles(const QRect &rect1, const QRect &rect2);


//! Helper function to simulate QRect.normalizeRect as in qt3
/*! Returns:
 *  - the normalized rect as in qt3
 */
AVLIBDEPRECATED_EXPORT QRect normalizeRect(const QPoint &topLeft, const QPoint &bottomRight);

//! This is an overloaded method
/*! Returns:
 *  - the normalized rect as in qt3
 */
AVLIBDEPRECATED_EXPORT QRect normalizeRect(const QRect &r);

}

#endif

// End of file
