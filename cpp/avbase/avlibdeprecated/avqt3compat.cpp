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
    \brief  provides some convenient functions for Qt3 compatible behavior
*/


// system includes

// Qt includes
#include <QAction>
#include <QPen>
#include <QTabletEvent>
#include <QTime>
#include <QWidget>
#include <QMouseEvent>
#include <QComboBox>
// local includes
#include "avqt3compat.h"

namespace AVQt3Compat {

///////////////////////////////////////////////////////////////////////////////

bool isValid(const QTime& time)
{
    return time.isNull() || time.isValid();
}

///////////////////////////////////////////////////////////////////////////////

QPen createPen()
{
    QPen p;
    p.setWidth(0);
    p.setCosmetic(true);
    p.setCapStyle(Qt::FlatCap);
    p.setJoinStyle(Qt::MiterJoin);
    return p;
}

QPen createPen(const QPen &pen)
{
    QPen p(pen);
    p.setCosmetic(true);
    p.setCapStyle(Qt::FlatCap);
    p.setJoinStyle(Qt::MiterJoin);
    return p;
}

QPen createPen(Qt::PenStyle penStyle)
{
    QPen p(penStyle);
    p.setWidth(0);
    p.setCosmetic(true);
    p.setCapStyle(Qt::FlatCap);
    p.setJoinStyle(Qt::MiterJoin);
    return p;
}

QPen createPen(const QColor &color, uint width, Qt::PenStyle penStyle)
{
    QPen p(color, width, penStyle, Qt::FlatCap, Qt::MiterJoin);
    p.setCosmetic(true);
    return p;
}

QPen createPen(const QColor &color, uint width, Qt::PenStyle penStyle, Qt::PenCapStyle capStyle, Qt::PenJoinStyle joinStyle)
{
    QPen p(color, width, penStyle, capStyle, joinStyle);
    p.setCosmetic(true);
    return p;
}

///////////////////////////////////////////////////////////////////////////////

QString qstringSection(const QString &text, const QString &separator, int start, int end, QString::SectionFlags flags)
{
    QString section = text.section(separator, start, end, flags);
    while (section.endsWith(separator))
        section.chop(separator.size());
    return section;
}

///////////////////////////////////////////////////////////////////////////////

QString qstringMid(const QString &text, int position, int n)
{
    if (text.isEmpty()) return QString();

    return text.mid(position, n);
}

///////////////////////////////////////////////////////////////////////////////

bool qstringEqual(const QString &s1, const QString &s2)
{
    if (s1.isNull() != s2.isNull()) return false;

    return s1 == s2;
}

bool qstringEqual(const QString &s1, const QLatin1String &s2)
{
    if (s1.isNull() != (s2.data() == 0)) return false;

    return s1 == s2;
}

bool qstringEqual(const QString &s1, const char *s2)
{
    if (s1.isNull() != (s2 == 0)) return false;

    return s1 == s2;
}

///////////////////////////////////////////////////////////////////////////////

bool closeAndDelete(QWidget *w)
{
    const bool close = w->close();
    if (close) {
        delete w;
        w = nullptr;
    }
    return close;
}

QWidget *parentWidgetSameWindow(QWidget *w)
{
    if (w->isWindow())
        return nullptr;
    return w->parentWidget();
}

///////////////////////////////////////////////////////////////////////////////

int tabletPressure(QTabletEvent *e)
{
    return trunc(e->pressure() * 255.0);
}

int mouseEventState(QMouseEvent *event)
{
    Qt::MouseButtons buttons = event->buttons();

    // Note: QMouseEvent::state() in Qt3 did not include the button that caused MousePress or DblClick
    // https://doc.qt.io/archives/3.3/qmouseevent.html#state
    // http://doc.qt.io/qt-4.8/qmouseevent.html#buttons
    if (event->type() == QEvent::MouseButtonDblClick || event->type() == QEvent::MouseButtonPress)
        buttons &= ~event->button();

    return buttons | event->modifiers();
}

///////////////////////////////////////////////////////////////////////////////

QString printDateTime(const QDateTime &dt)
{
    if (dt.isNull()) {
        return QStringLiteral("0000/00/00 00:00:00.000");
    }

    return dt.toString(QString("yyyy/MM/dd HH:mm:ss.zzz"));
}

///////////////////////////////////////////////////////////////////////////////

void setPaletteBackgroundPixmap(QWidget *w, const QPixmap &pixmap)
{
    QPalette pal = w->palette();
    pal.setBrush(w->backgroundRole(), pixmap);
    w->setPalette(pal);
    w->setAutoFillBackground(true);
}

///////////////////////////////////////////////////////////////////////////////

QAction *createAction(const QString &text, const QString &menuText, const QKeySequence &accel, QObject *parent, const char *name, bool toggle)
{
    QAction *action = new QAction(menuText, parent);
    if (name) action->setObjectName(QString::fromUtf8(name));
    action->setCheckable(toggle);
    action->setStatusTip(text);
    action->setShortcut(accel);

    return action;
}

QAction *createAction(const QString &text, const QIcon &icon, const QString &menuText, const QKeySequence &accel, QObject *parent, const char *name, bool toggle)
{
    QAction *action = new QAction(icon, menuText, parent);
    if (name) action->setObjectName(QString::fromUtf8(name));
    action->setCheckable(toggle);
    action->setStatusTip(text);
    action->setShortcut(accel);

    return action;
}

///////////////////////////////////////////////////////////////////////////////

void comboBoxSetCurrentText(QComboBox *combo, const QString &text)
{
    int i = combo->findText(text);
    if (i != -1)
        combo->setCurrentIndex(i);
    else if (combo->isEditable())
        combo->setEditText(text);
    else
       combo->setItemText(combo->currentIndex(), text);
}

///////////////////////////////////////////////////////////////////////////////

QRect uniteRectangles(const QRect &rect1, const QRect &rect2)
{
    if (!rect2.isValid())
        return rect1;
    else if (!rect1.isValid())
        return rect2;
    else
        return rect1.united(rect2);
}

///////////////////////////////////////////////////////////////////////////////

QRect normalizeRect(const QPoint &topLeft, const QPoint &bottomRight)
{
    QPoint tl = topLeft;
    QPoint br = bottomRight;

    if (bottomRight.x() < topLeft.x())
    {
         tl.setX(bottomRight.x());
         br.setX(topLeft.x());
    }

    if (bottomRight.y() < topLeft.y())
    {
         tl.setY(bottomRight.y());
         br.setY(topLeft.y());
    }

    return QRect(tl, br);
}

///////////////////////////////////////////////////////////////////////////////
//
QRect normalizeRect(const QRect &r)
{
    return normalizeRect(r.topLeft(), r.bottomRight());
}

} // end of namespace

// End of file

