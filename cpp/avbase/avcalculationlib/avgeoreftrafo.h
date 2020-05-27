///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Dr. Thomas Leitner, t.leitner@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Defines a coordinate transformation class based on AVGeoRef for
             arbitrary projections and transformations based on tie points.
*/

#if !defined(AVGEOREFTRAFO_H_INCLUDED)
#define AVGEOREFTRAFO_H_INCLUDED

// AVLib includes
#include "avcalculationlib_export.h"
#include "avcoordinatetransformbase.h"

// forward declarations
class AVGeoRef;

///////////////////////////////////////////////////////////////////////////////
//! A transformation class based on AVGeoRef for arbitrary transformations
//! based on tie points.

class AVCALCULATIONLIB_EXPORT AVGeoRefTrafo : public AVCoordinateTransformBase {
public:
    //! creates a AVGeoRefTrafo object
    /*! The filename of the tie-point file is called "avgeoreftrafo.udl"
        and should be located in the application "maps" directory
        (e.g. astos/maps)
    */
    AVGeoRefTrafo();

    //! creates a AVGeoRefTrafo object
    /*! Uses an AVGeoRef instance provided by the caller.

        \param georef the AVGeoRef instance to use for transformation. Must not be \c 0
        \param delete_here if \c true the trafo will delete the \p georef, if \c false
               the caller stays the owner and has to delete it
    */
    explicit AVGeoRefTrafo(AVGeoRef* georef, bool delete_here = true);

    //! destroys the AVGeoRefTrafo object
    ~AVGeoRefTrafo() override;

    //! convert x/y system coordinates in meter to lat/long (in degrees)
    //! based on WGS84 with the origin defined in the constructor
    bool xy2latlong(double x, double y, double &la, double &lo) const override;

    //! convert lat/long in degrees to x/y system coordinates in meter,
    //! with the origin defined in the constructor
    bool latlong2xy(double la, double lo, double &x, double &y) const override;

    //! return x/y to m resolution scaling factor (meters per pixel)
    bool getResFactor(double &res) const override;

    //! returns the unique trafo name
    //! the trafo name is built as follows:
    //!       trafo name + geo-ref-file-name + md5 sum of the geo ref file,
    //!       whereas the trafo name can be set using the setName() method.
    QString uniqueName() const override;

protected:
    //! main geo referencing class
    AVGeoRef* m_georef;

    //! wether the AVGeoRef is owned by the trafo and has to be deleted
    bool m_delete_georef;

private:
    //! Hidden copy constructor
    AVGeoRefTrafo(const AVGeoRefTrafo&);

    //! Hidden assignment operator
    AVGeoRefTrafo& operator=(const AVGeoRefTrafo&);
};

#endif

// End of file
