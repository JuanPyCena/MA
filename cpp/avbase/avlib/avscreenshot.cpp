///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright: AviBit data processing GmbH, 2001-2008
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author Tom Leitner, t.leitner@avibit.com
    \author QT4-PORT: Karlheinz Wohlmuth, k.wohlmuth@avibit.com
    \brief Implements a class to create a screenshot into an image file.
*/


// required QT includes
#include <qapplication.h>
#include <QDesktopWidget>
#include <qmessagebox.h>
#include <qobject.h>
#include <qpainter.h>
#include <QPixmap>

// avlib includes
#include "avdeprecate.h"
#include "avlog.h"
#include "avmisc.h"
#include "avconfig.h"
#include "avtimereference.h"
#include "avenvironment.h"

// local includes
#include "avscreenshot.h"


///////////////////////////////////////////////////////////////////////////////

AVScreenShot::AVScreenShot() : m_containsDateTime(false),
                               m_inverted(false)
{

}

///////////////////////////////////////////////////////////////////////////////

AVScreenShot::~AVScreenShot()
{

}

///////////////////////////////////////////////////////////////////////////////

AVScreenShot AVScreenShot::capture(QWidget* widget)
{
    AVScreenShot newScreenshot;
    QWidget* modelWidget = widget;
    QDesktopWidget desktop;
    int screen = 0;
    do
    {
        // use the desktop widget if w is 0
        if (widget == 0) modelWidget = desktop.screen(screen);

        // grab and save
        QPixmap snapshot;
        if (needSpecialGrab(modelWidget))
            snapshot = recursiveGrab(modelWidget);
        else
            snapshot = grabWindow(modelWidget->winId(), 0, 0, -1, -1);

        newScreenshot.m_images.append(snapshot.toImage());

        screen++;
    }
    while (widget == 0 && screen < desktop.numScreens());

    return newScreenshot;
}

///////////////////////////////////////////////////////////////////////////////

uint AVScreenShot::numImages() const
{
    return m_images.count();
}

///////////////////////////////////////////////////////////////////////////////

QList<QImage> AVScreenShot::images() const
{
    return m_images;
}

///////////////////////////////////////////////////////////////////////////////

QList<QPixmap> AVScreenShot::pixmaps() const
{
    QList<QPixmap> pixmaps;
    for (QList<QImage>::const_iterator it = m_images.begin(); it != m_images.end(); ++it)
    {
        pixmaps.append(QPixmap::fromImage(*it));
    }

    return pixmaps;
}

///////////////////////////////////////////////////////////////////////////////

bool AVScreenShot::addDateTime(const QDateTime& timestamp)
{
    if (m_containsDateTime)
    {
        AVLogger->Write(LOG_WARNING,
                        "AVScreenShot::addDateTime: Screenshot already contains Date/Time");
        return false;
    }

    QString currentDateTimeString = timestamp.isValid() ?
                                    AVPrintDateTimeHuman(timestamp) :
                                    AVPrintDateTimeHuman(AVTimeReference::currentDateTime());

    AVLogger->Write(LOG_INFO, "AVScreenShot:doScreenShot: adding date/time text");
    for (QList<QImage>::const_iterator it = m_images.constBegin(); it != m_images.constEnd(); ++it)
    {
        QPixmap pixmap;
        pixmap.convertFromImage(*it);

        QPainter pix_painter(&pixmap);
        QFont font = pix_painter.font();
        font.setPointSize(16);
        pix_painter.setFont(font);
        QFontMetrics font_metrics = pix_painter.fontMetrics();
        QRect font_size_rect = font_metrics.boundingRect(currentDateTimeString);
        int font_width  = 5 + (font_size_rect.width());
        int font_height = 5 + (abs(font_size_rect.top()) + font_size_rect.height());
        QRect font_rect = QRect((*it).width() - font_width, 0, font_width, font_height);
        pix_painter.setBrush(Qt::black);
        pix_painter.drawRect(font_rect);
        pix_painter.setPen(Qt::white);
        pix_painter.drawText(font_rect, Qt::AlignLeft, currentDateTimeString);
        pix_painter.end();
    }

    m_containsDateTime = true;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVScreenShot::invert()
{
    if (m_inverted)
    {
        AVLogger->Write(LOG_WARNING,
                        "AVScreenShot::invert: Screenshot is already inverted");
        return false;
    }

    for (QList<QImage>::iterator it = m_images.begin(); it != m_images.end(); ++it)
    {
        (*it).invertPixels();
    }

    m_inverted = true;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

QList<QFileInfo> AVScreenShot::save(const QString& fileNamePrefix)
{
    QString fileName = fileNamePrefix;
    if (fileName.isEmpty()) fileName = AVPrintDateTime(AVTimeReference::currentDateTime());

    // if no absolute path is specified in the file, setup the defined
    // screenshot directory in $APP_HOME/screenshots and make sure that the
    // directory exists
    QFileInfo fileInfo(fileName);
    if (fileInfo.isRelative()) {
        QString directoryName = AVEnvironment::getApplicationHome() + "/screenshots";
        QDir directory(directoryName);
        if (!directory.exists())
        {
            AVLogger->Write(LOG_INFO, "Creating directory %s", qPrintable(directoryName));
            if (!directory.mkdir(directoryName))
            {
                AVLogger->Write(LOG_ERROR,
                                "Could not create directory %s", qPrintable(directoryName));
            }
        }

        if (directory.exists())
        {
            fileName = directoryName + "/" + fileName;
        }
    }

    QList<QFileInfo> savedFileInfos;
    int screen = 0;
    for (QList<QImage>::const_iterator it = m_images.constBegin(); it != m_images.constEnd(); ++it)
    {
        ++screen;
        QString saveFileName = fileName + (m_images.count() == 1 ?
                                            QString("") : QString(".%1").arg(screen))+ ".png";
        if ((*it).save(saveFileName, "PNG", 20))
        {
            savedFileInfos.append(QFileInfo(saveFileName));
            AVLogger->Write(LOG_INFO, "Saved screenshot to %s", qPrintable(saveFileName));
        }
    }

    return savedFileInfos;
}

///////////////////////////////////////////////////////////////////////////////

bool AVScreenShot::doScreenShot(QWidget *w, const QString &filename, bool ad,
                                QString *ofilename, QList<QPixmap>* screenshots,
                                bool add_date_time_text_on_screenshot, bool invert)
{
    AVDEPRECATE("AVScreenShot::doScreenShot");

    // construct a filename from the date/time if no filename is specified
    QString fn = filename;
    QString current_dt_string = AVPrintDateTimeHuman(AVTimeReference::currentDateTime());
    if (fn.isNull() || fn.isEmpty()) fn = AVPrintDateTime(AVTimeReference::currentDateTime());

    // if no absolute path is specified in the file, setup the defined
    // screenshot directory in $APP_HOME/screenshots and make sure that the
    // directory exists
    QFileInfo fi(fn);
    if (fi.isRelative()) {
        QString dn = AVEnvironment::getApplicationHome() + "/screenshots";
        QDir d(dn);
        if (!d.exists())
        {
            AVLogger->Write(LOG_INFO, "Creating directory %s", qPrintable(dn));
            if (!d.mkdir(dn))
            {
                AVLogger->Write(LOG_ERROR, "Could not create directory %s", qPrintable(dn));
            }
        }
        if (d.exists())
        {
            fn = dn + "/" + fn;
        }
    }

    // if the desktop consists of multiple screens, save each screen into
    // a separate file
    int screen = 0;
    QWidget *scw = w;
    QDesktopWidget desktop;
    QString fname;
    do
    {
        // use the desktop widget if w is 0
        if (w == 0) scw = desktop.screen(screen);

        // grab and save
        QPixmap snapshot;
        if (needSpecialGrab(scw))
            snapshot = recursiveGrab(scw);
        else
            snapshot = grabWindow(scw->winId(), 0, 0, -1, -1);

        if (screenshots != 0) screenshots->append(snapshot);

        fname = fn + (desktop.numScreens() == 1 ? QString("") : QString(".%1").arg(screen))+ ".png";

        // draw the filename onto the screenshot

        if (add_date_time_text_on_screenshot)
        {
            AVLogger->Write(LOG_INFO, "AVScreenShot:doScreenShot: adding date/time text");
            QPainter pix_painter(&snapshot);
            QFont font = pix_painter.font();
            font.setPointSize(16);
            pix_painter.setFont(font);
            QFontMetrics font_metrics = pix_painter.fontMetrics();
            QRect font_size_rect = font_metrics.boundingRect(current_dt_string);
            int font_width  = 5 + (font_size_rect.width());
            int font_height = 5 + (abs(font_size_rect.top()) + font_size_rect.height());
            QRect font_rect = QRect(snapshot.width() - font_width, 0, font_width, font_height);
            pix_painter.setBrush(Qt::black);
            pix_painter.drawRect(font_rect);
            pix_painter.setPen(Qt::white);
            pix_painter.drawText(font_rect, Qt::AlignLeft, current_dt_string);
            pix_painter.end();
        }

        // invert snapshot
        if (invert)
        {
            QImage snapshot_img = snapshot.toImage();
            snapshot_img.invertPixels();
            if (!snapshot.convertFromImage(snapshot_img))
            {
                AVLogger->Write(LOG_ERROR, "Inverting screenshot failed!");
            }
            else
            {
                AVLogger->Write(LOG_INFO, "Inverting screenshot.");
            }
        }

        snapshot.save(fname, "PNG", 20);
        AVLogger->Write(LOG_INFO, "Saved screenshot to %s", qPrintable(fname));
        screen++;
    } while (w == 0 && screen < desktop.numScreens());

    // display dialog
    if (ad)
    {
        QMessageBox::information(w, "Screenshot complete",
                                 "Screenshot to file " + fname + " complete!");
    }

    // setup output filename
    if (ofilename != 0)
    {
        QFileInfo f(fname);
        *ofilename = f.baseName() + "." + f.suffix();
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVScreenShot::needSpecialGrab(QWidget* widget)
{
    Q_UNUSED(widget);
    return false;
}

QPixmap AVScreenShot::grabWindow(WId window, int x, int y, int w, int h)
{
    return QPixmap::grabWindow(window, x, y, w, h);
}

///////////////////////////////////////////////////////////////////////////////

QPixmap AVScreenShot::recursiveGrab(QWidget* window)
{
    Q_UNUSED(window);
    return QPixmap();
}

// End of file
