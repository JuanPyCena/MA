///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Andreas Schuller, a.schuller@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Declaration of the trafo factory
*/

#if !defined(TRAFOFACTORY_H_INCLUDED)
#define TRAFOFACTORY_H_INCLUDED

// Qt includes
#include <QMap>

// avlib
#include "avcalculationlib_export.h"
#include "avcoordellipsoid.h"
#include "avgausskruegersystemtrafo.h"
#include "avutmsystemtrafo.h"
#include "avsingleton.h"


// forward declarations
class AVCoordinateTransformBase;
class AVGeoRef;

///////////////////////////////////////////////////////////////////////////////
//! A factory for coordinate transformation classes.
/*! To get an instance of AVCoordinateTransformBase derived classes the trafo type, (name) and
    a parameter list has to be passed. The trafo factory caches all created trafo instances, so
    that calling trafoByName() a second time using the same parameters will return an already
    existent trafo object. This implies that the returned trafo object must not be deleted by the
    caller.
*/
class AVCALCULATIONLIB_EXPORT AVTrafoFactory
{
    //! call back function type
    typedef AVCoordinateTransformBase* (*TrafoCreator)(const QString&, const QStringList&);

    typedef QMap<QString, TrafoCreator> CallBackMap; //!< map holding call back functions

public:

    static const QString SYSTEM_TRAFO;    //!< used as type and default name for AVSystemTrafo
    static const QString GEO_REF_TRAFO;   //!< used as type and default name for AVGeoRefTrafo
    //! used as type and default name for AVMeteoSatTrafo
    static const QString METEO_SAT_TRAFO;
    //! used as type and default name for AVWeatherRadarTrafo
    static const QString WEATHER_RADAR_TRAFO;
    //!< used as type and default name for AVGaussKruegerTrafo
    static const QString GAUSS_KRUEGER_TRAFO;
    //!< used as type and default name for AVGaussKruegerSystemTrafo
    static const QString GAUSS_KRUEGER_SYSTEM_TRAFO;
    //!< used as type and default name for AVUtmTrafo
    static const QString UTM_TRAFO;
    //!< used as type and default name for AVUtmSystemTrafo
    static const QString UTM_SYSTEM_TRAFO;

public:

    //! Constructor
    AVTrafoFactory();
    //! Deconstructor
    ~AVTrafoFactory();

    //! Returns a trafo object of \c type with the given name and parameters
    /*! A list of all known trafo names can be found in
        \note The caller must not delete the returned pointer because the factory takes ownership
              of it.
        \sa createSystemParamList
        \sa createGeoRefParamList
        \sa createGaussKruegerParamList
        \sa createGaussKruegerSystemParamList
        \sa createUtmParamList
        \sa createUtmSystemParamList
    */
    AVCoordinateTransformBase* trafoByTypeAndName(const QString& type,
                                                  const QString& name,
                                                  const QStringList& params = QStringList());

    //! Returns a trafo object of \c type with the given parameters
    /*! The trafo name is set to the default trafo name which is
        the same as trafo type.

        \sa createSystemParamList
        \sa createGeoRefParamList
        \sa createGaussKruegerParamList
        \sa createGaussKruegerSystemParamList
        \sa createUtmParamList
        \sa createUtmSystemParamList
    */
    inline AVCoordinateTransformBase* trafoByType(const QString& type,
                                                  const QStringList& params = QStringList())
    {
        return trafoByTypeAndName(type, type, params);
    }

    //! Returns true if the given type is supported
    inline bool supports(const QString& type)
    {
        return m_trafoCreators->contains(type);
    }

public:

    //! Creates a parameter list for AVGeoRefTrafo
    /*! \param filename filename of the tie point file
        \sa AVGeoRef, AVGeoRefTrafo
    */
    inline static QStringList createGeoRefParamList(const QString& filename)
    {
        QStringList list;
        list += filename;
        return list;
    }

    //! Creates a system trafo parameter list
    /*! The origin is initialized using the given lat/long coordinates in degrees.
        \sa AVSystemTrafo
    */
    inline static QStringList createSystemParamList(const QString& refLatitude,
                                             const QString& refLongitude)
    {
        QStringList list;
        list += refLatitude;
        list += refLongitude;
        return list;
    }

    //! Creates a system trafo parameter list
    /*! The origin is initialized using the given lat/long coordinates in degrees.
        \sa AVSystemTrafo
    */
    inline static QStringList createSystemParamList(double refLatitude,
                                                    double refLongitude)
    {
        QStringList list;
        list += QString::number(refLatitude);
        list += QString::number(refLongitude);
        return list;
    }

    //! Creates a parameter list for AVGaussKruegerSystemTrafo
    /*! \sa AVGaussKruegerTrafo, AVCoordEllipsoid
        \note AVCoordEllipsoid::ET_CUSTOM is not supported by this method, use the overloaded
              method beneath.
        \return Returns an empty list if AVCoordEllipsoid::ET_CUSTOM was set. Otherwise the
                parameter list will be returned.
    */
    inline static QStringList
    createGaussKruegerParamList(double ref_mer, AVCoordEllipsoid::EllipsoidType refEllType)
    {
        if(refEllType == AVCoordEllipsoid::ET_CUSTOM)
        {
            return QStringList();
        }

        QStringList list;
        list += QString::number(ref_mer);
        list += QString::number(refEllType);
        return list;
    }

    //! Creates a parameter list for AVGaussKruegerTrafo using customized ellipsoid parameters
    /*! \sa AVGaussKruegerTrafo, AVCoordEllipsoid
    */
    inline static QStringList
    createGaussKruegerParamList(double ref_mer, double e_a, double e_f)
    {
        QStringList list;
        list += QString::number(ref_mer);
        list += QString::number(e_a);
        list += QString::number(e_f);
        return list;
    }

    //! Creates a parameter list for AVGaussKruegerSystemTrafo
    /*! \sa AVGaussKruegerSystemTrafo
    */
    inline static QStringList
    createGaussKruegerSystemParamList(AVGaussKruegerSystemTrafo::ReferenceMeridian refMer)
    {
        QStringList list;
        list += QString::number(refMer);
        return list;
    }

    //! Creates a parameter list for AVUtmSystemTrafo
    /*! \sa AVUtmTrafo, AVCoordEllipsoid
        \note AVCoordEllipsoid::ET_CUSTOM is not supported by this method, use the overloaded
              method beneath.
        \return Returns an empty list if AVCoordEllipsoid::ET_CUSTOM was set. Otherwise the
                parameter list will be returned.
    */
    inline static QStringList
    createUtmParamList(double ref_mer, AVCoordEllipsoid::EllipsoidType refEllType)
    {
        if(refEllType == AVCoordEllipsoid::ET_CUSTOM)
        {
            return QStringList();
        }

        QStringList list;
        list += QString::number(ref_mer);
        list += QString::number(refEllType);
        return list;
    }

    //! Creates a parameter list for AVUtmTrafo using customized ellipsoid parameters
    /*! \sa AVUtmTrafo, AVCoordEllipsoid
    */
    inline static QStringList
    createUtmParamList(double ref_mer, double e_a, double e_f)
    {
        QStringList list;
        list += QString::number(ref_mer);
        list += QString::number(e_a);
        list += QString::number(e_f);
        return list;
    }

    //! Creates a parameter list for AVUtmSystemTrafo
    /*! \sa AVUtmSystemTrafo
    */
    inline static QStringList
    createUtmSystemParamList(AVUtmSystemTrafo::ReferenceMeridian refMer)
    {
        QStringList list;
        list += QString::number(refMer);
        return list;
    }

protected:

    //! Creates a new AVGeoRefTrafo
    /*! \param name the name of the new trafo
        \param params parameter list used to create a new AVGeoRefTrafo instance.
        \sa createGeoRefParamList()
    */
    static AVCoordinateTransformBase* createGeoRefTrafo(const QString& name,
                                                        const QStringList& params);
    //! Creates a new AVMeteoSatTrafo
    /*! \param name the name of the new trafo
        \param params parameter list is unused
    */
    static AVCoordinateTransformBase* createMeteoSatTrafo(const QString& name,
                                                          const QStringList& params);

    //! Creates a new Weather AVWeatherRadarTrafo
    /*! \param name the name of the new trafo
        \param params parameter list is unused
    */
    static AVCoordinateTransformBase* createWeatherRadarTrafo(const QString& name,
                                                              const QStringList& params);
    //! Creates a new AVSystemTrafo
    /*! \param name the name of the new trafo
        \param params parameter list used to create a new AVGeoRefTrafo instance.
        \sa createSystemParamList()
    */
    static AVCoordinateTransformBase* createSystemTrafo(const QString& name,
                                                        const QStringList& params);

    //! Creates a new AVGaussKruegerSystemTrafo
    /*! \param name the name of the new trafo
        \param params parameter list used to create a new AVGaussKruegerSystemTrafo instance.
        \sa createGaussKruegerSystemTrafoParamList()
    */
    static AVCoordinateTransformBase* createGaussKruegerSystemTrafo(const QString& name,
                                                        const QStringList& params);

    //! Creates a new AVGaussKruegerTrafo
    /*! \param name the name of the new trafo
        \param params parameter list used to create a new AVGaussKruegerTrafo instance.
        \sa createGaussKruegerTrafoParamList()
    */
    static AVCoordinateTransformBase* createGaussKruegerTrafo(const QString& name,
                                                        const QStringList& params);

    //! Creates a new AVUtmSystemTrafo
    /*! \param name the name of the new trafo
        \param params parameter list used to create a new AVUtmSystemTrafo instance.
        \sa createUtmSystemTrafoParamList()
    */
    static AVCoordinateTransformBase* createUtmSystemTrafo(const QString& name,
                                                        const QStringList& params);

    //! Creates a new AVUtmTrafo
    /*! \param name the name of the new trafo
        \param params parameter list used to create a new AVUtmTrafo instance.
        \sa createUtmTrafoParamList()
    */
    static AVCoordinateTransformBase* createUtmTrafo(const QString& name,
                                                        const QStringList& params);

private:

    //! Initializes the factory. For instance registers all known creator functions
    void initialize();

private:

    CallBackMap* m_trafoCreators; //!< map holding all known trafo creator functions
    QHash<QString,AVCoordinateTransformBase*> m_trafoPool; //!< cache for trafo instances

private:
    //! Hidden copy constructor
    AVTrafoFactory(const AVTrafoFactory&);

    //! Hidden assignment operator
    AVTrafoFactory& operator=(const AVTrafoFactory&);
};

///////////////////////////////////////////////////////////////////////////////
class AVTrafoFactorySingleton : public AVTrafoFactory, public AVSingleton<AVTrafoFactorySingleton>
{
};
#define TRAFOFACTORY (*AVTrafoFactorySingleton::getInstance())
///////////////////////////////////////////////////////////////////////////////

#endif

// End of file
