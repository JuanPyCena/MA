///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
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

// AVLib includes
#include "avlog.h"
#include "avmisc.h"

//local includes
#include "avimagemaskbase.h"

///////////////////////////////////////////////////////////////////////////////

AVImageMaskBase::AVImageMaskBase()
    : m_width(0),
      m_height(0),
      m_sub_offset_x(0),
      m_sub_offset_y(0),
      m_sub_image_width(0),
      m_sub_image_height(0)
{
}

///////////////////////////////////////////////////////////////////////////////

AVImageMaskBase::~AVImageMaskBase()
{
}

///////////////////////////////////////////////////////////////////////////////

AVImageMaskBase::AVImageMaskBase(const AVImageMaskBase& other)
{
    m_width            = other.m_width;
    m_height           = other.m_height;
    m_sub_offset_x     = other.m_sub_offset_x;
    m_sub_offset_y     = other.m_sub_offset_y;
    m_sub_image_width  = other.m_sub_image_width;
    m_sub_image_height = other.m_sub_image_height;
    m_row_index_list   = other.m_row_index_list;
}

///////////////////////////////////////////////////////////////////////////////

bool AVImageMaskBase::deepCopy(const AVImageMaskBase& other, int max_width, int max_height)
{
    AVImageMaskBase::reset();
    if (max_width < 1 || max_height < 1) {
//         AVLogger->Write(LOG_WARNING, "AVImageMaskBase::deepCopy: "
//                         "Cannot copy range (%d/%d)", max_width, max_height);
        return true;
    }

    QLinkedList<RowData>::const_iterator it = other.m_row_index_list.begin();
    while(it != other.m_row_index_list.end())
    {
        if ((*it).row < max_height) {
            if ((*it).col_start < max_width) {
                if ((*it).col_end < max_width) {
                    m_row_index_list.append(*it);
                } else {
                    RowData tmp;
                    tmp.row       = (*it).row;
                    tmp.col_start = (*it).col_start;
                    tmp.col_end   = max_width - 1;
                    m_row_index_list.append(tmp);
                }
            }
        } else {
            break;
        }

        ++it;
    }

    calcLimitsFromList();
    m_width  = max_width;
    m_height = max_height;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

const AVImageMaskBase& AVImageMaskBase::operator= (const AVImageMaskBase& other)
{
    if (this == &other) return *this;

    m_width            = other.m_width;
    m_height           = other.m_height;
    m_sub_offset_x     = other.m_sub_offset_x;
    m_sub_offset_y     = other.m_sub_offset_y;
    m_sub_image_width  = other.m_sub_image_width;
    m_sub_image_height = other.m_sub_image_height;
    m_row_index_list   = other.m_row_index_list;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

void AVImageMaskBase::reset()
{
    m_width            = 0;
    m_height           = 0;
    m_sub_offset_x     = 0;
    m_sub_offset_y     = 0;
    m_sub_image_width  = 0;
    m_sub_image_height = 0;
    m_row_index_list.clear();
}

///////////////////////////////////////////////////////////////////////////////

bool AVImageMaskBase::set(QStringList &list, int width, int height,
                          int subwidth, int subheight)
{
    reset();

    QStringList::Iterator it;
    for (it = list.begin(); it != list.end(); ++it )
    {
        QStringList sl = it->split(";");
        if (sl.count() != 3) {
            AVLogger->Write(LOG_ERROR, "AVImageMaskBase::set: "
                            "Cannot parse line '%s'", qPrintable(*it));
            return false;
        }
        bool ok = true;
        RowData new_row;
        new_row.row       = sl[0].toInt(&ok);
        new_row.col_start = sl[1].toInt(&ok);
        new_row.col_end   = sl[2].toInt(&ok);

        if (!ok) {
            AVLogger->Write(LOG_ERROR, "AVImageMaskBase::set: "
                            "Cannot parse line '%s'", qPrintable(*it));
            return false;
        }
        m_row_index_list.append(new_row);
    }

    calcLimitsFromList();
    if (m_sub_image_height == 0 || m_sub_image_width == 0) reset();

    if (width >= 0) {
        if (width >= m_width) m_width = width;
        else {
            AVLogger->Write(LOG_ERROR, "AVImageMaskBase::set: "
                            "Cannot override width (%d) with (%d)!", m_width, width);
            return false;
        }
    }
    if (height >= 0) {
        if (height >= m_height) m_height = height;
        else {
            AVLogger->Write(LOG_ERROR, "AVImageMaskBase::set: "
                            "Cannot override height (%d) with (%d)!", m_height, height);
            return false;
        }
    }
    if (subwidth >= 0) {
        if (subwidth >= m_sub_image_width &&
            m_sub_offset_x + m_sub_image_width - 1 < m_width) m_sub_image_width = subwidth;
        else {
            AVLogger->Write(LOG_ERROR, "AVImageMaskBase::set: "
                            "Cannot override subwidth (%d) with (%d)!",
                            m_sub_image_width, subwidth);
            return false;
        }
    }
    if (subheight >= 0) {
        if (subheight >= m_sub_image_height &&
            m_sub_offset_y + m_sub_image_height - 1 < m_height) m_sub_image_height = subheight;
        else {
            AVLogger->Write(LOG_ERROR, "AVImageMaskBase::set: "
                            "Cannot override subheight (%d) with (%d)!",
                            m_sub_image_height, subheight);
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVImageMaskBase::clip(const AVImageMaskBase& clip_mask)
{
    // both index lists have ascending row and start_col entries!!
    QLinkedList<RowData>::iterator it            = m_row_index_list.begin();
    QLinkedList<RowData>::const_iterator it_mask = clip_mask.m_row_index_list.begin();
    while(it != m_row_index_list.end())
    {
        // go to the first row in the mask which matches
        while (it_mask != clip_mask.m_row_index_list.end() &&
               (*it).row > (*it_mask).row) {
            ++it_mask;
        }
        if (it_mask == clip_mask.m_row_index_list.end()) {
            // no more mask lines, clear all remaining ranges of *this
            while (it != m_row_index_list.end()) {
                it = m_row_index_list.erase(it);
            }
            break;
        }

        if ((*it).row == (*it_mask).row) {
            bool do_remove = true;
            QLinkedList<RowData>::const_iterator it_mask_tmp = it_mask;
            while (it_mask_tmp != clip_mask.m_row_index_list.end() &&
                   (*it).row == (*it_mask_tmp).row) {
                // first check the range and split the entry at the mask limits
                int min = qMax((*it).col_start, (*it_mask_tmp).col_start);
                int max = qMin((*it).col_end, (*it_mask_tmp).col_end);

                if (max < min) {
                    // it is outside, do nothing here, because maby there are other mask entries
                    // for this row
                } else {
                    // adjust the current entry and create a new entry with the limits
                    // new_col_end to old_col_end. Insert this entry after the current one, it
                    // will be checked in the next loop.
                    do_remove = false;
                    int old_col_end = (*it).col_end;
                    (*it).col_start = min;
                    (*it).col_end   = max;
                    if (max < old_col_end) {
                        RowData new_data;
                        new_data.row       = (*it).row;
                        new_data.col_start = max + 1;
                        new_data.col_end   = old_col_end;
                        QLinkedList<RowData>::iterator it_insert = it;
                        ++it_insert;
                        // let it point to the new element, to do not change the current element
                        // any more
                        if (it_insert == m_row_index_list.end()) {
                            m_row_index_list.append(new_data);
                            it = m_row_index_list.end()-1;
                        } else {
                            it = m_row_index_list.insert(it_insert, new_data);
                        }
                        do_remove = true;
                    } else {
                        //we cut ot the entrie up to the end. Further mask entries will not change
                        //the entry any more
                        break;
                    }
                }

                ++it_mask_tmp;
            }
            if (do_remove) {
                it = m_row_index_list.erase(it);
            } else {
                ++it;
            }
        } else {
            // the current range is not in the mask, remove it
            it = m_row_index_list.erase(it);
        }
    }

    // update offsets and sub_image dimensions
    int w = m_width;
    int h = m_height;
    calcLimitsFromList();
    if (w  < offsetX() + subWidth())  m_width  = offsetX() + subWidth();
    else                              m_width  = w;
    if (h  < offsetY() + subHeight()) m_height = offsetY() + subHeight();
    else                              m_height = h;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVImageMaskBase::clip(int width_min, int height_min, int width_max, int height_max)
{
    int new_width  = width_max  - width_min  + 1;
    int new_height = height_max - height_min + 1;

    if (new_width > 0 && new_height > 0) {
        QLinkedList<RowData>::iterator it = m_row_index_list.begin();
        while (it != m_row_index_list.end()) {
            if ((*it).row < height_min || (*it).row > height_max) {
                it = m_row_index_list.erase(it);
                continue;
            }

            // clip inside range
            int min = qMax((*it).col_start, width_min);
            int max = qMin((*it).col_end, width_max);

            if (min > max) {
                it = m_row_index_list.erase(it);
            } else {
                (*it).col_start = min;
                (*it).col_end   = max;
                ++it;
            }
        }
        // update offsets and sub_image dimensions
        int w = m_width;
        int h = m_height;
        calcLimitsFromList();
        if (w  < offsetX() + subWidth())  m_width  = offsetX() + subWidth();
        else                              m_width  = w;
        if (h  < offsetY() + subHeight()) m_height = offsetY() + subHeight();
        else                              m_height = h;
    } else {
        // keep width and height of mask
        m_sub_offset_x     = 0;
        m_sub_image_width  = 0;
        m_sub_offset_y     = 0;
        m_sub_image_height = 0;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVImageMaskBase::compareMask(const AVImageMaskBase& other) const
{
    if (m_width            != other.m_width)            return false;
    if (m_height           != other.m_height)           return false;
    if (m_sub_offset_x     != other.m_sub_offset_x)     return false;
    if (m_sub_offset_y     != other.m_sub_offset_y)     return false;
    if (m_sub_image_width  != other.m_sub_image_width)  return false;
    if (m_sub_image_height != other.m_sub_image_height) return false;

    if (m_row_index_list.count() != other.m_row_index_list.count()) return false;

    QLinkedList<RowData>::const_iterator it       = m_row_index_list.begin();
    QLinkedList<RowData>::const_iterator it_other = other.m_row_index_list.begin();
    for (; it != m_row_index_list.end(); ++it,  ++it_other) {
        if ((*it).row       != (*it_other).row)       return false;
        if ((*it).col_start != (*it_other).col_start) return false;
        if ((*it).col_end   != (*it_other).col_end)   return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

QString AVImageMaskBase::infoString() const
{
    QString res;
    AVsprintf(res, "width=%d, height=%d, offset_x=%d, offset_y=%d, "
              "sub_width=%d, sub_height=%d", m_width, m_height,
              m_sub_offset_x, m_sub_offset_y, m_sub_image_width,
              m_sub_image_height);

    QLinkedList<RowData>::const_iterator it = m_row_index_list.begin();
    while(it != m_row_index_list.end())
    {
        AVsprintf(res, "%s\nrow=%d, start=%d, end=%d",qPrintable(res), (*it).row,
                  (*it).col_start, (*it).col_end);
        ++it;
    }
    return res;
}

///////////////////////////////////////////////////////////////////////////////

void AVImageMaskBase::calcLimitsFromList()
{
    // now get the new limits
    bool first = true;
    QLinkedList<RowData>::iterator it1 = m_row_index_list.begin();
    while(it1 != m_row_index_list.end())
    {
        if (first) {
            first = false;
            m_sub_offset_x     = (*it1).col_start;
            m_sub_offset_y     = (*it1).row;
            m_sub_image_width  = (*it1).col_end;
            m_sub_image_height = (*it1).row;
        } else {
            if ((*it1).row < m_sub_offset_y) m_sub_offset_y = (*it1).row;
            else if ((*it1).row > m_sub_image_height) m_sub_image_height = (*it1).row;

            if ((*it1).col_start < m_sub_offset_x) m_sub_offset_x = (*it1).col_start;
            if ((*it1).col_end > m_sub_image_width) m_sub_image_width = (*it1).col_end;
        }
        ++it1;
    }
    if (first == false) {
        m_sub_image_height = m_sub_image_height - m_sub_offset_y + 1;
        AVASSERT(m_sub_image_height >= 0);
        m_sub_image_width = m_sub_image_width - m_sub_offset_x + 1;
        AVASSERT(m_sub_image_width >= 0);
        m_width = m_sub_offset_x + m_sub_image_width;
        m_height = m_sub_offset_y + m_sub_image_height;
    } else {
        // no data
        m_sub_offset_x     = 0;
        m_sub_offset_y     = 0;
        m_sub_image_height = 0;
        m_sub_image_width  = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVImageMaskBase::applyOffset(int width_offset, int height_offset)
{
    if (width_offset >= width() || height_offset >= height()) {
        reset();
        return true;
    }

    if (height_offset != 0) {
        QLinkedList<RowData>::iterator it = m_row_index_list.begin();
        while (it != m_row_index_list.end()) {
            if ((*it).row + height_offset < 0) {
                it = m_row_index_list.erase(it);
                continue;
            } else {
                (*it).row = (*it).row + height_offset;
                ++it;
            }
        }
    }

    if (width_offset != 0) {
        QLinkedList<RowData>::iterator it = m_row_index_list.begin();
        while (it != m_row_index_list.end()) {
            if ((*it).col_start + width_offset < 0) {
                if ((*it).col_end + width_offset < 0) {
                    it = m_row_index_list.erase(it);
                    continue;
                } else {
                    (*it).col_start = 0;
                    (*it).col_end   = (*it).col_end + width_offset;
                    ++it;
                }
            } else {
                (*it).col_start = (*it).col_start + width_offset;
                (*it).col_end   = (*it).col_end   + width_offset;
                ++it;
            }
        }
    }

    // update offsets and sub_image dimensions
    int w = m_width;
    int h = m_height;
    calcLimitsFromList();
    if (w  < offsetX() + subWidth())  m_width  = offsetX() + subWidth();
    else                              m_width  = w;
    if (h  < offsetY() + subHeight()) m_height = offsetY() + subHeight();
    else                              m_height = h;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVImageMaskBase::checkMaskStructure() const
{
    QLinkedList<RowData>::const_iterator it_last = m_row_index_list.begin();
    QLinkedList<RowData>::const_iterator it = it_last;
    ++it;
    while (it != m_row_index_list.end()) {
        if ((*it_last).row > (*it).row) {
            AVLogger->Write(LOG_ERROR, "AVImageMaskBase::checkMaskStructure: "
                            "last row (%d) > act. row (%d)", (*it_last).row, (*it).row);
            return false;
        }
        if ((*it_last).row == (*it).row && (*it_last).col_start > (*it).col_start) {
            AVLogger->Write(LOG_ERROR, "AVImageMaskBase::checkMaskStructure: "
                            "last col_start (%d) > act. col_start (%d) (row=%d)",
                            (*it_last).col_start, (*it).col_start, (*it).row);
            return false;
        }

        if ((*it_last).row == (*it).row && (*it_last).col_end + 1 >= (*it).col_start) {
            AVLogger->Write(LOG_ERROR, "AVImageMaskBase::checkMaskStructure: "
                            "last col_end (%d) + 1 >= act. col_start (%d) (row=%d)",
                            (*it_last).col_end, (*it).col_start, (*it).row);
            return false;
        }

        ++it;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVImageMaskBase::writePNGFile(const QString &fn) const
{
    QImage image(width(), height(),QImage::Format_Indexed8);
    image.setColorCount(256);
    if (image.isNull())
    {
        AVLogger->Write(LOG_ERROR, "AVImageMaskBase::writePNGFile: "
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
        AVLogger->Write(LOG_ERROR, "AVImageMaskBase::writePNGFile: "
                        "can't get image data pointer");
        return false;
    }

    QLinkedList<RowData>::const_iterator it = m_row_index_list.begin();
    while (it != m_row_index_list.end()) {
        memset(image_data + (*it).row*image.bytesPerLine() + (*it).col_start, 255,
               (*it).col_end - (*it).col_start + 1);
        ++it;
    }

    if(!image.save(fn, "PNG", 100))
    {
        AVLogger->Write(LOG_ERROR, "AVImageMaskBase::writePNGFile: "
                        "can't save image to file '%s'", qPrintable(fn));
        return false;
    }

    AVLogger->Write(LOG_DEBUG, "AVImageMaskBase::writePNGFile: "
                    "File %s closed\n", qPrintable(fn));
    return true;
}

// End of file
