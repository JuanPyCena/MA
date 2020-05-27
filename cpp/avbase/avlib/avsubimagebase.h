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
    \author  DI Alexander Randeu, a.randeu@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief
*/

// Qt includes
#include <QDateTime>
#include <QString>

// local includes
#include "avlib_export.h"
#include "avimagemaskbase.h"
#include "avdatetime.h"

#ifndef __AVSUBIMAGEBASE_H__
#define __AVSUBIMAGEBASE_H__

///////////////////////////////////////////////////////////////////////////////

//!
/*! more details ...
 */
class AVLIB_EXPORT AVSubImageBase : public AVImageMaskBase
{
    friend class AVImageMask;

public:
    //! Currently these two image types are suppored.
    enum ImageTypes { Raw = 0,       //!< defined raw images
                      Filtered };    //!< defined filtered images

    //! Currently these two image states are suppored.
    enum ImageStates { Update = 0,    //!< sub image is an update
                       Replacement }; //!< sub image is an replacement

    //! Standard Constructor
    /*! \note Default Type = Raw, Default State = Update
     */
    AVSubImageBase();

    //! Destructor
    ~AVSubImageBase() override;

    //! Copy constructor
    AVSubImageBase(const AVSubImageBase& other);

    //! assignment operator
    AVSubImageBase& operator=(const AVSubImageBase& other);

    //! shallow copy. The image data array is only a shallow copy.
    AVSubImageBase& shallowCopy(const AVSubImageBase& other);

    qint8 imageNumber() const {
        return m_image_number;
    }

    void setImageNumber(qint8 n) {
        m_image_number = n;
    }

    const QDateTime &timestamp() const {
        return m_timestamp;
    }

    void setTimestamp(const QDateTime &dt) {
        m_timestamp = dt;
    }

    ImageTypes imageType() const {
        return static_cast<ImageTypes>(m_image_type);
    }

    void setImageType(const ImageTypes& type) {
        m_image_type = type;
    }

    ImageStates imageState() const {
        return static_cast<ImageStates>(m_image_state);
    }

    void setImageState(const ImageStates& state) {
        m_image_state = state;
    }

    void setBPP(int bpp) {
        m_bpp = bpp;
    }

    int getBPP() const {
        return m_bpp;
    }

    //! reduces the image to the area where this is set and clip_mask is set.
    /*! Keeps the width and height unchanged.
     */
    bool clip(const AVImageMaskBase& clip_mask) override;

    //! clips the mask to given min and max values (both inclusive)
    /*! \note width and height of image is not changed. If the new width or height (from
              width_min/max and height_min/max <=0 the sub-mask size is set to 0.
     */
    bool clip(int width_min, int height_min, int width_max, int height_max) override;

    //! compares 2 images (inkluding masks). returns true, if the content of all members
    //! is the same.
    virtual bool compareImage(const AVSubImageBase& other) const;

    //! create a sub-image from a string list (mainly for tests). The area is filled with 0.
    /*! The format of each entry of the list is "ROW;FROM;TO". The list must be
        sorted ascending first by ROW and then by FROM. Multiple entries for the same row
        are allowed, but the FROM:TO ranges are not allowed to overlap (not checked).
        \return true is successful, false if not (e.g. negativ coordinates in the list)
     */
    bool set(QStringList& list, int width = -1, int height = -1, int subwidth = -1, int subheight = -1) override;

    //! creates a sub-image from a mask and a data array
    /*! This is the same as assigning the whole image (data) and then clipping with the mask.
        Only image parts which are in the image and also in the mask are kept.
     */
    virtual bool set(const AVImageMaskBase &mask, const uchar* data,
                     int data_width, int data_height);

    //! creates a sub-image from a mask, sets the data to 0
    virtual bool set(const AVImageMaskBase &mask);

    //! creates a sub-image from data with a mask of width*height
    /*! \param data pointer to data
        \param width width of data
        \param height height of data
        \param width_offset start col of data to copy
        \param height_offset start row of data to copy
        \param sub_width width of data to copy
        \param sub_height height of data to copy
     */
    virtual bool set(const uchar* data, int width, int height,
                     int width_offset, int height_offset,
                     int sub_width, int sub_height);

    //! moves the entire mask by the given offsets (positive offsets move to right/down)
    /*! \note the image width and height is also adjusted by the offsets
     */
    bool applyOffset(int width_offset, int height_offset) override;

    bool writePNGFile(const QString& fn) const override;

    //! fills data with the actual data. positions outside the mask are set to 0.
    /*! \param data data pointer
        \param datasize size of data. Must be the same as width()*height()
     */
    virtual bool toImage(uchar* data, int datasize);

protected:

    qint8     m_bpp;            //!< bits per pixel (0 is undefined = old mode)
    AVDateTime m_timestamp;      //!< image timestamp
    qint8     m_image_number;   //!< image number from 0 to 7
    qint8     m_image_type;     //!< image type (raw or filtered)
    QByteArray m_image_data;     //!< storage for image
    qint8     m_image_state;    //!< image state (update or replacement)
};

#endif /* __AVSUBIMAGEBASE_H__ */

// End of file
