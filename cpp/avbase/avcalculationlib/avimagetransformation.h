///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVCALCULATIONLIB - Avibit Calculation Library
//
///////////////////////////////////////////////////////////////////////////////

/*!
  \file
  \author  Kevin Krammer, k.krammer@avibit.com
  \author  QT4-PORT: Karlheinz Wohlmuth, k.wohlmuth@avibit.com
  \brief   Declaration of classes applying transformation to images
*/

#if !defined(AVIMAGETRANSFORMATION_H_INCLUDED)
#define AVIMAGETRANSFORMATION_H_INCLUDED

//qt includes
#include <qimage.h>
#include <qobject.h>
#include "avcalculationlib_export.h"

// forward declarations
class AVCoordinateTransformBase;
class AVProjectionTransformationCalculator;
class AVThread;
class AVTransformationCalculator;
class ImageTransformationMT;
class QRect;
class QSize;

class AVCALCULATIONLIB_EXPORT AVImageTransformation
{
public:

    //! Caching policy
    //! None: No caching is used
    //! UseOrCreate: If a cache file does exist, transformation values will be loaded from file.
    //!              Otherwise values will be calculated and written to a cache file.
    enum CachingPolicy
    {
        None,
        UseOrCreate,
        Mandatory
    };

    //! Constructor
    //! \param cache_file_must_exist if true a cache file corresponding to the unique name of a set
    //!                              calculator must exist. For instance a call to method
    //!                              calculateTransformationValues() will fail if the cache file
    //!                              does not exist.
    //! \sa AVTransformationCalculator::uniqueName()
    explicit AVImageTransformation(CachingPolicy opts = None);

    //! Destructor
    virtual ~AVImageTransformation();

    inline void setCalculator(AVTransformationCalculator* calculator)
    {
        m_calculator = calculator;
    }

    bool transformImage(const QImage& input, QImage& output);

    bool transformImageSection(const QImage& input, QImage& output, const QRect& outputSection);

    bool calculateTransformationValues(const QSize& maxOutputSize)
    {
        return calculateTransformationValues(QSize(32767, 32767), maxOutputSize);
    }

    bool calculateTransformationValues(const QSize& maxInputSize, const QSize& maxOutputSize);

    //! Returns the progress of calculating the transformation values.
    /*! The returned value is in the range of 0 and numCols + numRows (manhattanLength).
        Calling this method does only make sense if calculation will be done
        asynchronous.
        \sa calculateTransformationValues(), ImageProjectionProcessor
    */
    int progressOfCalculation() const;

    //! Resets the progress information.
    /*! Call this method to get a valid progress value when progressOfCalculation()
        will be invoked before calculateTransformationValues() has been started.

        \sa progressOfCalculation()
    */
    void resetProgressInfo(){ m_progress.reset(); }

    QSize maxOutputSize() const { return QSize(m_numCols, m_numRows); }

protected:

    //! Transforms an 8 bit input image (with a color look up table) to a 32 bit output image
    bool transformInputImageWithColorLUT(const QImage& input, QImage& output);

    //! Transforms an 8 bit input image (with a color look up table) to a 32 bit output image
    bool transformInputImageSectionWithColorLUT(const QImage& input,
                                                QImage& output, const QRect& outputSection);

    AVTransformationCalculator* m_calculator;

    struct Point
    {
        int x;
        int y;

        inline bool isValid() const { return x >= 0 && y >= 0; }
    };

    Point** m_transformationValues;
    int m_numRows;
    int m_numCols;
    int m_capacityRows;
    int m_capacityCols;
    int m_maxInputRows;
    int m_maxInputCols;

    struct Progress
    {
        enum State { Start, CheckCacheFile, Calculate, CreateCacheFile, Finished };

        Progress(){ reset(); }

        void reset()
        {
            calculating = 0;
            loadedFromCacheFile = false;
            fileWritten         = false;
            fileCompressed      = false;
            state = Start;
        }

        int  calculating;
        bool loadedFromCacheFile;
        bool fileWritten;
        bool fileCompressed;
        State state;
    };

    Progress m_progress;
    int m_cachingOpts;

    volatile bool m_stopCalculating;

    static const qint32 CACHE_FILE_VERSION = 1;

protected:

    void createValueArrays(int rows, int cols);
    void createCacheFile();
    bool loadPrecalculatedValueArrays(const QSize& inputSize, const QSize& outputSize);
    void deleteValueArrays();

    static bool copyImage(const QImage& source, int sx, int sy, int w, int h, QImage& destination);

private:
    //! Hidden copy constructor
    AVImageTransformation(const AVImageTransformation&);

    //! Hidden assignment operator
    AVImageTransformation& operator=(const AVImageTransformation&);
};

///////////////////////////////////////////////////////////////////////////////
//! This class projects a given input image from a source to a target coordinate system
/*! Usage of this class is demonstrated in the weather layer test application

    \note This class works asynchron which means that the time consuming calculation
          of the projection is done by a separate thread.

     Already used in: \ref weather_layer_test_app
*/
class AVCALCULATIONLIB_EXPORT AVImageProjectionProcessor : public QObject
{
Q_OBJECT

public:

    //! Constructor
    //! \param caching_opts sets the caching mode of AVImageTransformation
    //! \sa AVImageTransformation
    explicit AVImageProjectionProcessor(
        AVImageTransformation::CachingPolicy caching_opts = AVImageTransformation::UseOrCreate,
        QObject* parent = 0, const char* name = 0 );

    //! Destructor
    ~AVImageProjectionProcessor() override;

    //! sets up the projection
    void setupProjection( AVCoordinateTransformBase* source_trafo,
                          AVCoordinateTransformBase* target_trafo );

    //! calculate the projection matrix
    //! \sa AVImageTransformation::calculateTransformationValues
    void calculateProjection(const QSize& maxOutputSize)
    {
        calculateProjection(QSize(32767, 32767), maxOutputSize);
    }

    //! calculate the projection matrix
    //! \sa AVImageTransformation::calculateTransformationValues
    void calculateProjection(const QSize& minInputSize,
                             const QSize& maxOutputSize);

    //! Applies the projection to an output image of the given size
    //! \return projection of the given input image image
    QImage applyProjection(const QImage& image, const QSize& outputSize) const;

    //! Returns the max output size
    QSize maxOutputSize() const { return m_maxOutputSize; }

    //! Returns true if the projection processor is currently busy. For instance
    //! if it is calculating the transformation matrix at the moment.
    bool isBusy(){ return !m_calculationFinished; }

    //! Stops calculating the transformation matrix
    //! \return true on success. Stopping the processor can fail if the internally
    //!         used thread couldn't be stopped.
    bool stopProcessing();

    //! Returns how many lines (rows) are already calculated.
    int progressOfCalculation() const;

    //! Returns the total number of steps needed for calculating the projection
    int totalCalculationSteps() const;

signals:
    //! Signals that calculation has finished
    void signalCalculationFinished();

protected slots:
    //! Internally called when the transformation matrix has been successfully created/loaded.
    //! \sa calculateProjection
    void slotCalculationFinished(int res);

protected:

    ImageTransformationMT* m_imageTransformation;      //!< image transformer
    AVProjectionTransformationCalculator* m_calculator; //!< projection calculator
    AVThread* m_calcThread;  //!< thread for calculating the tranformation matrix

    QSize m_maxOutputSize;

    bool m_sourceAndTargetAreEqual; //!< true if source and target projection are equal
    bool m_calculationFinished;    //!< true if calculation has finished
    int  m_totalCalcSteps; //!< total number of calculation steps
private:

    //! Hidden copy constructor
    AVImageProjectionProcessor( const AVImageProjectionProcessor& src );

    //! Hidden assignment operator
    AVImageProjectionProcessor& operator=( const AVImageProjectionProcessor& other );
};

///////////////////////////////////////////////////////////////////////////////

#endif

// End of file
