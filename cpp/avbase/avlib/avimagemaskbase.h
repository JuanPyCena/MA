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

#ifndef __AVIMAGEMASKBASE_H__
#define __AVIMAGEMASKBASE_H__

// Qt includes
#include <QLinkedList>
#include <QStringList>

// AVLib includes
#include "avlib_export.h"

// forward declarations

///////////////////////////////////////////////////////////////////////////////

//!
/*! more details ...
 */
class AVLIB_EXPORT AVImageMaskBase
{
    friend class AVImageMask;

public:

    //! Standard Constructor
    AVImageMaskBase();

    //! Destructor
    virtual ~AVImageMaskBase();

    //! copy-constructor
    AVImageMaskBase(const AVImageMaskBase& other);

    //! assignment operator
    const AVImageMaskBase& operator= (const AVImageMaskBase& other);

    //! create a mask from a string list (mainly for tests)
    /*! The format of each entry of the list is "ROW;FROM;TO". The list must be
        sorted ascending first by ROW and then by FROM. Multiple entries for the same row
        are allowed, but the FROM:TO ranges are not allowed to overlap (not checked).
        \return true is successful, false if not (e.g. negativ coordinates in the list)
     */
    virtual bool set(QStringList &list, int width = -1, int height = -1,
                     int subwidth = -1, int subheight = -1);

    //! creates a deep copy of another image mask, but limit width and height to the given
    //! max values. Mask width and height is set to the max values.
    virtual bool deepCopy(const AVImageMaskBase& other, int max_width, int max_height);

    //! reduces the mask to the area where this is set and clip_mask is set.
    /*! Keeps the width and height unchanged.
     */
    virtual bool clip(const AVImageMaskBase& clip_mask);

    //! clips the mask to given min and max values (both inclusive)
    /*! \note width and height of image is not changed. If the new width or height (from
              width_min/max and height_min/max <=0 the sub-mask size is set to 0.
     */
    virtual bool clip(int width_min, int height_min, int width_max, int height_max);

    //! compares 2 masks. returns true, if the content of all members is the same.
    virtual bool compareMask(const AVImageMaskBase& other) const;

    virtual int width() const {
        return m_width;
    }

    virtual int height() const {
        return m_height;
    }

    virtual int offsetX() const {
        return m_sub_offset_x;
    }

    virtual int offsetY() const {
        return m_sub_offset_y;
    }

    virtual int subWidth() const {
        return m_sub_image_width;
    }

    virtual int subHeight() const {
        return m_sub_image_height;
    }

    bool isEmpty() const {
        if (m_row_index_list.count() == 0) return true;
        return false;
    }

    virtual QString infoString() const;

    //! moves the entire mask by the given offsets (positive offsets move to right/down)
    /*! \note the image width and height is only adjusted if they are increased by the offsets
     */
    virtual bool applyOffset(int width_offset, int height_offset);

    //! for unit tests only
    virtual bool checkMaskStructure() const;

    virtual bool writePNGFile(const QString &fn) const;

protected:

    typedef struct RowData_ {
        qint32 row;
        qint32 col_start;
        qint32 col_end;
    } RowData;

    qint32    m_width;             //!< image width (# of columns)
    qint32    m_height;            //!< image height (# of lines)
    qint32    m_sub_offset_x;      //!< x (column) offset for m_row_index_list indices
    qint32    m_sub_offset_y;      //!< y (row) offset for m_row_index_list indices
    qint32    m_sub_image_width;   //!< max. x (column) in m_row_index_list indices
    qint32    m_sub_image_height;  //!< max. y (row) in m_row_index_list indices

    QLinkedList<RowData> m_row_index_list; //!< row descriptions. RowData::row must be ascending
                                          //!< in the list!

    //! reset all entries
    void reset();

    //! calculates m_sub_offset_x/y and m_sub_image_width/height from m_row_index_list
    void calcLimitsFromList();

};

#endif /* __AVIMAGEMASKBASE_H__ */

// End of file
