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
    \brief Defines a class to create an inverted screenshot into an image file.
*/

#if !defined(AVSCREENSHOT_H)
#define AVSCREENSHOT_H

// Local includes
#include "avlib_export.h"
#include "avdatetime.h"

// required QT includes
#include <qimage.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qwidget.h>
#include <qdatetime.h>
#include <qfileinfo.h>

///////////////////////////////////////////////////////////////////////////////
//! class to create a screenshot into an image file.

class AVLIB_EXPORT AVScreenShot
{
public:
    //! Constructs an empty screenshot without any images.
    AVScreenShot();

    //! Destructs the screenshot instance.
    ~AVScreenShot();

    /*!
     * Takes a screenshot of the given widget or the whole desktop if widget == 0.
     * If there are multiple screens, one image per screen is captured.
     * \param widget Widget to take the screenshot of.
     * \return A new instance containing the taken screenshot images.
     */
    static AVScreenShot capture(QWidget* widget = 0);

    /**!
     * Returns the number of images in the screenshot.
     * \return The number of images in the screenshot.
     */
    uint numImages() const;

    /**!
     * Returns the list of captured images.
     * \return The list of captured images.
     */
    QList<QImage> images() const;

    /**!
     * Converts the captured images to pixmaps and returns them.
     * \return Pixmaps of the captured images.
     */
    QList<QPixmap> pixmaps() const;

    /**
     * Adds date/time to the screenshots.
     * \param timestamp Date/Time to add.
     * \return True, if date/time has been added, false if date/time was already present.
     */
    bool addDateTime(const QDateTime& timestamp = AVDateTime());

    /**!
     * Inverts the captured screenshots.
     * \return True, if the images could be inverted, false if they were already inverted before.
     */
    bool invert();

    /**
     * Saves the captured images to PNG files using the given filename prefix.
     * \param fileNamePrefix Prefix to use, timestamp string is used if empty.
     * \return List of the fileinfos of the written files. Empty if no files could be written.
     */
    QList<QFileInfo> save(const QString& fileNamePrefix = "");

    //! makes a inverted screenshot
    /*! \param w the widget to make the screenhot of, or 0 for the complete desktop
        \param filename the filename for the screenshot or an empty string
                        to construct a filename from the current date/time.
                        If no path is specified, the file will be created
                        under $APP_HOME/screenshots (the directory will be
                        created if it does not exist yet.)
        \param ad display an information dialog when screenshots have been saved
        \param ofilename if != 0, will receive the screenshot file created
        \param screenshots if != 0 all grabbed pixmaps will be appended instead of saved
        \param add_date_time_text_on_screenshot When true prints a date/time string
               into the upper right corner of the sceenshot.
        \param invert When true inverts the snapshot using QImage's invertPixels function.
        \return true if successful, false if not
    */
    static bool doScreenShot(QWidget *w = 0,
                             const QString &filename = QString::null,
                             bool ad = false,
                             QString *ofilename = 0,
                             QList<QPixmap>* screenshots = 0,
                             bool add_date_time_text_on_screenshot = false,
                             bool invert = false);

protected:
    QList<QImage> m_images;
    bool m_containsDateTime;
    bool m_inverted;

private:
    static bool needSpecialGrab(QWidget* widget);

    static QPixmap grabWindow(WId window, int x, int y, int w, int h);

    static QPixmap recursiveGrab(QWidget* window);
};

#endif

// End of file
