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
#include <QImage>
#include <QLinkedList>

#include "avlog.h"

#include "avsubimagebase.h"

///////////////////////////////////////////////////////////////////////////////

AVSubImageBase::AVSubImageBase()
    : m_bpp(0),
      m_image_number(0),
      m_image_type(Raw),
      m_image_state(Update)
{
}

///////////////////////////////////////////////////////////////////////////////

AVSubImageBase::~AVSubImageBase()
{
}

///////////////////////////////////////////////////////////////////////////////

AVSubImageBase::AVSubImageBase(const AVSubImageBase& other)
    : AVImageMaskBase(other)
{
    m_bpp              = other.m_bpp;
    m_timestamp        = other.m_timestamp;
    m_image_number     = other.m_image_number;
    m_image_type       = other.m_image_type;
    m_image_data       = other.m_image_data;
    m_image_state      = other.m_image_state;
}

///////////////////////////////////////////////////////////////////////////////

AVSubImageBase& AVSubImageBase::operator=(const AVSubImageBase& other)
{
    if (this == &other) return *this;

    AVImageMaskBase::operator=(other);

    m_bpp              = other.m_bpp;
    m_timestamp        = other.m_timestamp;
    m_image_number     = other.m_image_number;
    m_image_type       = other.m_image_type;
    m_image_data       = other.m_image_data;
    m_image_state      = other.m_image_state;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

AVSubImageBase& AVSubImageBase::shallowCopy(const AVSubImageBase& other)
{
    if (this == &other) return *this;

    AVImageMaskBase::operator=(other);

    m_bpp              = other.m_bpp;
    m_timestamp        = other.m_timestamp;
    m_image_number     = other.m_image_number;
    m_image_type       = other.m_image_type;
    m_image_data       = other.m_image_data; // shallow copy
    m_image_state      = other.m_image_state;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSubImageBase::clip(const AVImageMaskBase& clip_mask)
{
    // store the old data area
    int off_x      = offsetX();
    int off_y      = offsetY();
    int sub_width  = subWidth();
    int sub_height = subHeight();

    if (!AVImageMaskBase::clip(clip_mask)) return false;

    if (subHeight() <= 0 || subWidth() <= 0) {
        m_image_data.resize(0);
        return true;
    }

    // the new range is always smaller or equal the old one
    if (off_x != offsetX() || off_y != offsetY() ||
        sub_width != subWidth() || sub_height != subHeight()) {
        QByteArray new_data(subWidth()*subHeight(),'\0');

        int deltax = offsetX() - off_x;
        AVASSERT(deltax >= 0);
        AVASSERT(deltax + subWidth() <= sub_width);
        int deltay = offsetY() - off_y;
        AVASSERT(deltay >= 0);
        AVASSERT(deltay + subHeight() <= sub_height);
        for (int i = 0; i < subHeight(); ++i) {
            int org_offset = (deltay + i)*sub_width + deltax;
            int new_offset = i*subWidth();
            memcpy(new_data.data() + new_offset, m_image_data.data() + org_offset, subWidth());
        }

        m_image_data = new_data; // explic. shared!
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSubImageBase::clip(int width_min, int height_min, int width_max, int height_max)
{
    // store the actual sub image data
    int sub_width  = subWidth();
    int sub_height = subHeight();
    int offset_x   = offsetX();
    int offset_y   = offsetY();

    // change mask
    AVImageMaskBase::clip(width_min, height_min, width_max, height_max);

    // no change in data size
    if (sub_width == subWidth() && sub_height == subHeight()) {
        return true;
    }

    // change image data
    if (subWidth() == 0 || subHeight() == 0) {
        m_image_data.resize(0);
        return true;
    }

    // to adjust the image data use the mask values!
    // mask can be smaller than (width_max - width_min) etc.!!
    QByteArray ba(subWidth()*subHeight(),'\0');

    // check for a change of the offsets
    int oxb = offsetX() - offset_x;
    AVASSERT(oxb < 0);
    int oxe = (offset_x + sub_width) - (offsetX() + subWidth());
    AVASSERT(oxe < 0);

    int oyb = offsetY() - offset_y;
    AVASSERT(oyb < 0);
    int oye = (offset_y + sub_height) - (offsetY() + subHeight());
    AVASSERT(oye < 0);

    AVASSERT(sub_width - oxb - oxe == subWidth());
    AVASSERT(sub_height - oyb - oye == subHeight());

    for (int y = oyb, yn = 0; y < sub_height - oye && yn < subHeight(); ++y, ++yn) {
        memcpy(ba.data()+yn*subWidth(), m_image_data.data()+y*sub_width+oxb, subWidth());
    }

    m_image_data = ba;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSubImageBase::compareImage(const AVSubImageBase &other) const
{
    if (this == &other) return true;

    if (!compareMask(other)) return false;

    if (m_bpp          != other.m_bpp)          return false;
    if (m_timestamp    != other.m_timestamp)    return false;
    if (m_image_number != other.m_image_number) return false;
    if (m_image_type   != other.m_image_type)   return false;
    if (m_image_data   != other.m_image_data)   return false;
    if (m_image_state  != other.m_image_state)  return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSubImageBase::set(QStringList &list, int width, int height,
                         int subwidth, int subheight)
{
    if (!AVImageMaskBase::set(list, width, height, subwidth, subheight)) return false;
    m_image_data.fill(0, subWidth()*subHeight());
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSubImageBase::set(const AVImageMaskBase &mask, const uchar* data,
                         int data_width, int data_height)
{
    if (data == 0) {
        AVLogger->Write(LOG_ERROR, "AVSubImageBase::set: "
                        "Cannot work with data=0!");
        return  false;
    }

    // first set the mask
    if (!AVImageMaskBase::deepCopy(mask, data_width, data_height)) return false;

    // now copy the needed part of the image
    m_image_data.resize(subWidth()*subHeight());
    if (m_image_data.size() > 0) {
        for (int y = 0; y < subHeight(); ++y) {
            memcpy(m_image_data.data()+y*subWidth(), data+(offsetY()+y)*data_width+offsetX(),
                   subWidth());
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSubImageBase::set(const AVImageMaskBase &mask)
{
    // first set the mask
    if (!AVImageMaskBase::deepCopy(mask, mask.width(), mask.height())) return false;

    // now copy the needed part of the image
    m_image_data.resize(subWidth()*subHeight());
    m_image_data.fill(0);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSubImageBase::set(const uchar* data, int width, int height,
                         int width_offset, int height_offset,
                         int sub_width, int sub_height)
{
    reset();

    if (data == 0) {
        AVLogger->Write(LOG_ERROR, "AVSubImageBase::set: "
                        "Cannot work with data=0!");
        return  false;
    }

    if (width_offset + sub_width > width || height_offset + sub_height > height) {
        AVLogger->Write(LOG_ERROR, "AVSubImageBase::set: "
                        "Dimensions out of range (w=%d,h=%d,ox=%d,oy=%d,sw=%d,sh=%d)",
                        width, height, width_offset, height_offset, sub_width, sub_height);
        return false;
    }

    m_width            = width;
    m_height           = height;
    m_sub_offset_x     = width_offset;
    m_sub_offset_y     = height_offset;
    m_sub_image_width  = sub_width;
    m_sub_image_height = sub_height;

    if (sub_height == 0 || sub_width == 0) {
        m_sub_offset_x     = 0;
        m_sub_offset_y     = 0;
        m_sub_image_width  = 0;
        m_sub_image_height = 0;
        return true;
    }

    // create mask;
    for (int i = m_sub_offset_y; i < m_sub_offset_y + m_sub_image_height; ++i) {
        RowData rd;
        rd.row = i;
        rd.col_start = m_sub_offset_x;
        rd.col_end   = m_sub_offset_x + m_sub_image_width - 1;
        m_row_index_list.append(rd);
    }

    // now copy the needed part of the image
    m_image_data.resize(sub_width*sub_height);
    if (m_image_data.size() > 0) {
        for (int y = 0 + height_offset, y1 = 0; y < height && y1 < sub_height; ++y, ++y1) {
            memcpy(m_image_data.data()+y1*sub_width, data+y*width+width_offset, sub_width);
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSubImageBase::applyOffset(int width_offset, int height_offset)
{
    // remeber the sub image data
    int sub_width  = subWidth();
    int sub_height = subHeight();
    int offset_x   = offsetX();
    int offset_y   = offsetY();

    // change mask
    AVImageMaskBase::applyOffset(width_offset, height_offset);

    // no change in data size
    if (sub_width == subWidth() && sub_height == subHeight()) {
        return true;
    }

    // change image data
    if (subWidth() == 0 || subHeight() == 0) {
        m_image_data.resize(0);
        return true;
    }

    QByteArray ba(subWidth()*subHeight(), '\0');
    int ox = offset_x + width_offset;
    if (ox > 0) ox = 0;
    else ox=-ox;
    int oy = offset_y + height_offset;
    if (oy > 0) oy = 0;
    else oy=-oy;

    AVASSERT(sub_width-ox == subWidth());
    AVASSERT(sub_height-oy == subHeight());

    for (int y = oy, yn = 0; y < sub_height && yn < subHeight(); ++y, ++yn) {
        memcpy(ba.data()+yn*subWidth(), m_image_data.data()+y*sub_width+ox, subWidth());
    }

    m_image_data = ba;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSubImageBase::writePNGFile(const QString &fn) const
{
    QImage image(width(), height(),QImage::Format_Indexed8);
    image.setColorCount(256);
    if (!image.isNull())
    {
        AVLogger->Write(LOG_ERROR, "AVSubImageBase::writePNGFile: "
                        "can't create a %d by %d QImage", width(), height());
        return false;
    }

    // set colors to gray
    for(int i = 0; i < 256; ++i)
        image.setColor( i, qRgb(i,i,i) );

    // create image data
    image.fill(0);

    uchar *image_data = image.bits();
    if (image_data == 0) {
        AVLogger->Write(LOG_ERROR, "AVSubImageBase::writePNGFile: "
                        "can't get image data pointer");
        return false;
    }

    QLinkedList<RowData>::const_iterator it = m_row_index_list.begin();
    while (it != m_row_index_list.end()) {
        memcpy(image_data + (*it).row*image.bytesPerLine() + (*it).col_start,
               m_image_data.data() + ((*it).row - offsetY())*subWidth() +
               ((*it).col_start - offsetX()),
               (*it).col_end - (*it).col_start + 1);
        ++it;
    }

    if(!image.save(fn, "PNG", 100))
    {
        AVLogger->Write(LOG_ERROR, "AVSubImageBase::writePNGFile: "
                        "can't save image to file '%s'", qPrintable(fn));
        return false;
    }

    AVLogger->Write(LOG_DEBUG, "AVSubImageBase::writePNGFile: "
                    "File %s closed\n", qPrintable(fn));
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSubImageBase::toImage(uchar* data, int datasize)
{
    if (datasize != width()*height()) {
        AVLogger->Write(LOG_ERROR, "AVSubImageBase::toImage: "
                        "given datasize not suitable (shoud be %d but is %d)",
                        width()*height(), datasize);
        return false;
    }

    // clear image
    memset(data, 0, datasize);

    QLinkedList<RowData>::const_iterator it = m_row_index_list.begin();
    while (it != m_row_index_list.end()) {
        memcpy(data + (*it).row*width() + (*it).col_start,
               m_image_data.data() + ((*it).row - offsetY())*subWidth() +
               ((*it).col_start - offsetX()),
               (*it).col_end - (*it).col_start + 1);
        ++it;
    }

    return true;
}

// End of file


