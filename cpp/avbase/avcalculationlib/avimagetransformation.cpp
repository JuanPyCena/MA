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
  \brief  Implementation of classes applying transformation to images
*/


// Qt includes
#include <qdir.h>
#include <qfileinfo.h>
#include <qimage.h>
#include <qsize.h>

// AVLib includes
#include "avconfig.h"
#include "avcoordinatetransformbase.h"
#include "avimagetransformation.h"
#include "avlog.h"
#include "avthread.h"
#include "avtransformationcalculator.h"


///////////////////////////////////////////////////////////////////////////////

AVImageTransformation::AVImageTransformation( CachingPolicy opts )
    : m_calculator(0),
      m_transformationValues(0),
      m_numRows(0),
      m_numCols(0),
      m_capacityRows(0),
      m_capacityCols(0),
      m_maxInputRows(0),
      m_maxInputCols(0),
      m_cachingOpts(opts),
      m_stopCalculating(false)
{}

///////////////////////////////////////////////////////////////////////////////

AVImageTransformation::~AVImageTransformation()
{
    deleteValueArrays();
}

///////////////////////////////////////////////////////////////////////////////

bool AVImageTransformation::transformImage(const QImage& input, QImage& output)
{
    if (m_calculator == 0)
    {
        if (input.size() == output.size())
        {
            output = input;
            return true;
        }

        if (output.width() < input.width() || output.height() < input.height())
        {
            output = input;
            return true;
        }

        return copyImage(input, 0, 0, input.width(), input.height(), output);
    }

    AVASSERT(m_transformationValues != 0);

    if (input.width() > m_maxInputCols || input.height() > m_maxInputRows)
    {
        LOGGER_AVCOMMON_AVLIB.Write(LOG_ERROR, "AVImageTransformation::transformImage: "
                "Input image larger than calculated transformation value matrix contents");

        return false;
    }

    if (output.width() > m_numCols || output.height() > m_numRows)
    {
        LOGGER_AVCOMMON_AVLIB.Write(LOG_ERROR, "AVImageTransformation::transformImage: "
                "Output image larger than calculated transformation value matrix");

        return false;
    }

    if(input.depth() == 8)
    {
       return transformInputImageWithColorLUT(input, output);
    }

    for (int row = 0; row < output.height(); ++row)
    {
        QRgb*  imageLine = reinterpret_cast<QRgb*>(output.scanLine(row));
        Point* valueLine = m_transformationValues[row];

        for (int col = 0; col < output.width(); ++col)
        {
            Point p = valueLine[col];

            if (p.isValid() && p.x < input.width() && p.y < input.height())
            {
                imageLine[col] = (reinterpret_cast<const QRgb*>(input.scanLine(p.y)))[p.x];
            }
            else
            {
                imageLine[col] = qRgba(0, 0, 0, 0);
            }
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVImageTransformation::transformImageSection(const QImage& input, QImage& output,
                                                  const QRect& outputSection)
{
    AVASSERT(m_transformationValues != 0);

    if (!outputSection.isValid()) return transformImage(input, output);

    if (outputSection.width() > output.width() || outputSection.height() > output.height())
    {
        LOGGER_AVCOMMON_AVLIB.Write(LOG_ERROR, "AVImageTransformation::transformImageSection: "
                "Output section size (%d, %d) does not fit into output image size (%d, %d)",
                outputSection.width(), outputSection.height(),
                output.width(), output.height());

        return false;
    }

    if (input.width() > m_maxInputCols || input.height() > m_maxInputRows)
    {
        LOGGER_AVCOMMON_AVLIB.Write(LOG_ERROR, "AVImageTransformation::transformImageSection: "
                "Input image larger than calculated transformation value matrix contents");

        return false;
    }

    if ((outputSection.x() + outputSection.width()) > m_numCols ||
        (outputSection.y() + outputSection.height()) > m_numRows)
    {
        LOGGER_AVCOMMON_AVLIB.Write(LOG_ERROR, "AVImageTransformation::transformImageSection: "
                "Output section (%d, %d, %d, %d) "
                "outside calculated transformation value matrix (%d, %d, %d, %d)",
                outputSection.x(), outputSection.y(),
                outputSection.width(), outputSection.height(),
                0, 0, m_numCols, m_numRows);

        return false;
    }

    if (input.depth() == 8)
    {
        return transformInputImageSectionWithColorLUT(input, output, outputSection);
    }

    for (int row = 0; row < outputSection.height(); ++row)
    {
        QRgb*  imageLine = reinterpret_cast<QRgb*>(output.scanLine(row));
        Point* valueLine = m_transformationValues[row + outputSection.y()];

        for (int col = 0; col < outputSection.width(); ++col)
        {
            Point p = valueLine[col + outputSection.x()];

            if (p.isValid() && p.x < input.width() && p.y < input.height())
            {
                imageLine[col] = (reinterpret_cast<const QRgb*>(input.scanLine(p.y)))[p.x];
            }
            else
            {
                imageLine[col] = qRgba(0, 0, 0, 0);
            }
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVImageTransformation::transformInputImageWithColorLUT(const QImage& input, QImage& output)
{
    if(output.depth() != 32)
    {
        LOGGER_AVCOMMON_AVLIB.Write(LOG_ERROR,
            "AVImageTransformation::transformInputImageWithColorLUT: output image must "
            " have a color depht of 32 bit");
        return false;
    }

    int numColors = input.colorCount();
    QVector<QRgb> colorTable = input.colorTable();

    for (int row = 0; row < output.height(); ++row)
    {
        QRgb*  imageLine = reinterpret_cast<QRgb*>(output.scanLine(row));
        Point* valueLine = m_transformationValues[row];

        for (int col = 0; col < output.width(); ++col)
        {
            Point p = valueLine[col];

            if (p.isValid() && p.x < input.width() && p.y < input.height())
            {
                quint8 index = input.scanLine(p.y)[p.x];
                imageLine[col] = index < numColors ?
                                    qRgba(qRed(colorTable[index]),
                                        qGreen(colorTable[index]),
                                        qBlue(colorTable[index]),
                                        qAlpha(colorTable[index]))
                                    : qRgba(0, 0, 0, 0);
            }
            else
            {
                imageLine[col] = qRgba(0, 0, 0, 0);
            }
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVImageTransformation::transformInputImageSectionWithColorLUT(const QImage& input,
        QImage& output, const QRect& outputSection)
{
    if(output.depth() != 32)
    {
        LOGGER_AVCOMMON_AVLIB.Write(LOG_ERROR,
            "AVImageTransformation::transformInputImageSectionWithColorLUT: output image must "
            " have a color depth of 32 bit");
        return false;
    }

    int numColors = input.colorCount();
    QVector<QRgb> colorTable = input.colorTable();

    for (int row = 0; row < outputSection.height(); ++row)
    {
        QRgb*  imageLine = reinterpret_cast<QRgb*>(output.scanLine(row));
        Point* valueLine = m_transformationValues[row + outputSection.y()];

        for (int col = 0; col < outputSection.width(); ++col)
        {
            Point p = valueLine[col + outputSection.x()];

            if (p.isValid() && p.x < input.width() && p.y < input.height())
            {
                quint8 index = input.scanLine(p.y)[p.x];
                imageLine[col] = index < numColors ?
                                    qRgba(qRed(colorTable[index]),
                                        qGreen(colorTable[index]),
                                        qBlue(colorTable[index]),
                                        qAlpha(colorTable[index]))
                                    : qRgba(0, 0, 0, 0);
            }
            else
            {
                imageLine[col] = qRgba(0, 0, 0, 0);
            }
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVImageTransformation::calculateTransformationValues(const QSize& maxInputSize,
                                                          const QSize& maxOutputSize)
{
    if (!maxInputSize.isValid() || !maxOutputSize.isValid()) return false;

    m_progress.reset();

    if (m_calculator != 0)
    {
        m_progress.state = Progress::CheckCacheFile;

        if( m_cachingOpts == UseOrCreate || m_cachingOpts == Mandatory )
        {
            bool rc = loadPrecalculatedValueArrays(maxInputSize, maxOutputSize);

            if( !rc && m_cachingOpts == Mandatory )
            {
                LOGGER_ROOT.Write(LOG_ERROR,
                    "AVImageTransformation::calculateTransformationValues: "
                        "Loading trafo cache file failed.");
                return false;
            }

            if(rc)
            {
                 m_progress.state = Progress::Finished;
                 return true;
            }
        }

        m_progress.state = Progress::Calculate;

        createValueArrays(maxOutputSize.height(), maxOutputSize.width());

        m_maxInputRows = maxInputSize.height();
        m_maxInputCols = maxInputSize.width();

        m_stopCalculating = false;

        for (int row = 0; row < m_numRows && !m_stopCalculating; ++row)
        {
            Point* scanLine = m_transformationValues[row];

            for (int col = 0; col < m_numCols && !m_stopCalculating; ++col)
            {
                double x = 0.0;
                double y = 0.0;

                if (m_calculator->untransform(static_cast<double>(col), static_cast<double>(row), x, y))
                {
                    scanLine[col].x = x < 0.0 ? -1 : static_cast<int>(x + 0.5);

                    if (scanLine[col].x >= maxInputSize.width()) scanLine[col].x = -1;

                    scanLine[col].y = y < 0.0 ? -1 : static_cast<int>(y + 0.5);

                    if (scanLine[col].y >= maxInputSize.height()) scanLine[col].y = -1;
                }
                else
                {
                    scanLine[col].x = -1;
                    scanLine[col].y = -1;
                }
            }

            m_progress.calculating++;
        }

        if(m_stopCalculating) return false;

        if(m_cachingOpts == UseOrCreate)
        {
            m_progress.state = Progress::CreateCacheFile;
            createCacheFile();
        }
    }

    m_progress.state = Progress::Finished;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVImageTransformation::
loadPrecalculatedValueArrays(const QSize& inputSize, const QSize& outputSize)
{
    QString baseDir = AVEnvironment::getApplicationCache() + "/";
    QFileInfo cacheFileInfo(baseDir + m_calculator->name() + ".trafocache");

    if ( !cacheFileInfo.exists() || !cacheFileInfo.isReadable() )
    {
        if( m_cachingOpts == Mandatory )
        {
            LOGGER_ROOT.Write( LOG_ERROR, "AVImageTransformation::loadPrecalculatedValueArrays: "
                "either file '%s' does not exist or is not readable",
                    qPrintable(cacheFileInfo.absoluteFilePath()) );
        }

        return false;
    }

    QFile cache_file( cacheFileInfo.absoluteFilePath() );

    if( !cache_file.open( QIODevice::ReadOnly ) )
    {
        LOGGER_ROOT.Write( LOG_ERROR, "AVImageTransformation::loadPrecalculatedValueArrays: "
            "opening file '%s' failed", qPrintable(cacheFileInfo.absoluteFilePath()) );
        return false;
    }

    // uncompress data
    QByteArray cached_values = cache_file.readAll();
    cached_values = qUncompress(cached_values);

    if ( cached_values.isEmpty()  )
    {
        LOGGER_ROOT.Write( LOG_ERROR, "AVImageTransformation::loadPrecalculatedValueArrays: "
            "file '%s' is empty or corrupted", qPrintable(cacheFileInfo.absoluteFilePath()) );
        return false;
    }

    QDataStream stream( &cached_values, QIODevice::ReadOnly );
    stream.setVersion(QDataStream::Qt_3_3);

    qint32 version = 0;
    QSize maxInputSize, calcOutputSize;

    stream >> version;
    stream >> maxInputSize;
    stream >> calcOutputSize;

    // check version
    if(version != CACHE_FILE_VERSION)
    {
        LOGGER_ROOT.Write( LOG_ERROR, "AVImageTransformation::loadPrecalculatedValueArrays: "
                "mismatching version '%d' of file '%s'. The currently supported version is '%d'",
                      version, qPrintable(cacheFileInfo.absoluteFilePath()), CACHE_FILE_VERSION);
        return false;
    }

    // check if precalulated value array is large enough
    if(    maxInputSize.width()    < inputSize.width()
        || maxInputSize.height()   < inputSize.height()
        || calcOutputSize.width()  < outputSize.width()
        || calcOutputSize.height() < outputSize.height())
    {
        LOGGER_ROOT.Write( LOG_DEBUG, "AVImageTransformation::loadPrecalculatedValueArrays: "
            "The existing cache file, calculated for a max input size of [%d, %d] and a "
            "calculated output size of [%d, %d], isn't large enough for handling an input size "
            "of [%d x %d] and an ouput size of [%d x %d].",
            maxInputSize.width(), maxInputSize.height(),
            calcOutputSize.width(), calcOutputSize.height(),
            inputSize.width(), inputSize.height(), outputSize.width(), outputSize.height());
        return false;
    }

    int rows = calcOutputSize.height();
    int cols = calcOutputSize.width();

    // check valid file size
    int calculatedCacheFileSize = 2 * sizeof(QSize) + sizeof(qint32) +
        sizeof(Point) * rows * cols;

    if(cached_values.count() != calculatedCacheFileSize)
    {
        LOGGER_ROOT.Write(LOG_ERROR, "AVImageTransformation::loadPrecalculatedValueArrays: "
            "file '%s' is corrupted: calculated size (%d) != real size (%d)",
                qPrintable(cacheFileInfo.absoluteFilePath()), calculatedCacheFileSize,
                    cached_values.count());
        return false;
    }

    createValueArrays(rows, cols);

    for (int row = 0; row < rows; ++row)
    {
        Point* scanLine = m_transformationValues[row];

        if(stream.atEnd())
        {
            cache_file.close();
            return false;
        }

        stream.readRawData(reinterpret_cast<char*>(scanLine), sizeof(Point) * cols);
    }

    cache_file.close();

    m_maxInputRows = maxInputSize.height();
    m_maxInputCols = maxInputSize.width();

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void AVImageTransformation::createCacheFile()
{
    AVASSERT( m_calculator );

    QString baseDir = AVEnvironment::getApplicationCache() + "/";
    QString cacheFileName    = m_calculator->name() + ".trafocache";
    QString tmpCacheFileName = cacheFileName + ".tmp";
    QFileInfo cacheFileInfo( baseDir + tmpCacheFileName );

    QFile cache_file( cacheFileInfo.absoluteFilePath() );

    if( !cache_file.open( QIODevice::WriteOnly ) )
    {
        LOGGER_ROOT.Write( LOG_ERROR,
                           "AVImageTransformation::createCacheFile: opening file '%s' failed",
                           qPrintable(m_calculator->name()) );
        return;
    }

    QByteArray cached_values;
    QDataStream stream( &cached_values, QIODevice::WriteOnly | QIODevice::Unbuffered);
    stream.setVersion(QDataStream::Qt_3_3);

    stream << CACHE_FILE_VERSION;
    stream << QSize(m_maxInputCols, m_maxInputRows);
    stream << QSize(m_numCols, m_numRows);

    for (int row = 0; row < m_numRows; ++row)
    {
        Point* scanLine = m_transformationValues[row];
        stream.writeRawData(reinterpret_cast<char*>(scanLine), sizeof(Point) * m_numCols );
    }

    //compress data
    cached_values = qCompress(cached_values);

    m_progress.fileCompressed = true;

    if( cache_file.write( cached_values ) == -1 )
    {
        LOGGER_ROOT.Write( LOG_ERROR,
                           "AVImageTransformation::createCacheFile: writing precalculated"
                           "values to file '%s' failed",
                           qPrintable(m_calculator->name()) );
    }

    cache_file.close();

    QDir dir(baseDir);
    if(!dir.rename(tmpCacheFileName, cacheFileName))
    {
        LOGGER_ROOT.Write( LOG_ERROR, "AVImageTransformation::createCacheFile: "
            "Renaming temporary cache file '%s', got an IO error",
                    qPrintable(cacheFileInfo.absoluteFilePath()));
        return;
    }

    qDebug("AVImageTransformation::createCacheFile: output size [%d x %d], input size [%d x %d]",
        m_numCols, m_numRows, m_maxInputRows, m_maxInputCols);

    m_progress.fileWritten = true;
}

///////////////////////////////////////////////////////////////////////////////

int AVImageTransformation::progressOfCalculation() const
{
    int manhattanLength = m_numRows + m_numCols;

    switch(m_progress.state)
    {
        case Progress::Start: return 0;

        case Progress::CheckCacheFile: return 0;

        case Progress::Calculate:
        {
            double val = 0.0;

            if(m_cachingOpts == UseOrCreate)
            {
                val = static_cast<double>(m_progress.calculating * manhattanLength * 0.8)
                    / m_numRows;
            }
            else
            {
                val = static_cast<double>(m_progress.calculating * manhattanLength) / m_numRows;
            }
            return static_cast<int>(val < 0.0 ? val - 0.5 : val + 0.5);
        }

        case Progress::CreateCacheFile:
        {

            double val = m_progress.fileCompressed && m_progress.fileWritten ? manhattanLength :
                manhattanLength * 0.8;

            return static_cast<int>(val < 0.0 ? val - 0.5 : val + 0.5);
        }
        case Progress::Finished:
            return manhattanLength;

        default: return 0;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVImageTransformation::createValueArrays(int rows, int cols)
{
    AVASSERT(rows > 0);
    AVASSERT(cols > 0);

    // check if current arrays are large enough, if not delete them
    if (m_transformationValues != 0)
    {
        if (rows > m_capacityRows)
            deleteValueArrays();
        else if (cols > m_capacityCols)
            deleteValueArrays();
    }

    // if there are currently no arrays, create them
    if (m_transformationValues == 0)
    {
        m_transformationValues = new Point*[rows];
        AVASSERT(m_transformationValues != 0);

        for (int row = 0; row < rows; ++row)
        {
            m_transformationValues[row] = new Point[cols];
            AVASSERT(m_transformationValues[row] != 0);
        }

        m_capacityRows = rows;
        m_capacityCols = cols;
    }

    m_numRows = rows;
    m_numCols = cols;
}

///////////////////////////////////////////////////////////////////////////////

void AVImageTransformation::deleteValueArrays()
{
    if (m_transformationValues == 0) return;

    for (int row = 0; row < m_capacityRows; ++row)
    {
        delete[] m_transformationValues[row];
    }

    delete[] m_transformationValues;

    m_transformationValues = 0;
}

///////////////////////////////////////////////////////////////////////////////

bool AVImageTransformation::copyImage(const QImage& source, int sx, int sy, int w, int h,
                                      QImage& destination)
{
    // sanity checks
    if (source.isNull()) {
        LOGGER_AVCOMMON_AVLIB.Write(LOG_WARNING, "AVImageTransformation::copyImage: "
                                    "Source image is NULL!");
        return false;
    }

    if (w <= 0 || h <= 0 || sx < 0 || sy < 0 ||
        sx > source.width() || sy > source.height()) {
        LOGGER_AVCOMMON_AVLIB.Write(LOG_WARNING, "AVImageTransformation::copyImage: "
                        "Source image area out of range (w=%d, h=%d, sx=%d, sy=%d, "
                        "src.w=%d,src.h=%d)!", w, h, sx, sy, source.width(), source.height());
        return false;
    }

    // check and adjust width + height
    int w_copy = w;
    int h_copy = h;
    if (sx + w > source.width())  w_copy = source.width()  - sx;
    if (sy + h > source.height()) h_copy = source.height() - sy;

    if (w <= 0 || h <= 0) {
        LOGGER_AVCOMMON_AVLIB.Write(LOG_WARNING, "AVImageTransformation::copyImage: "
                        "Source image area out of range (w=%d, h=%d, sx=%d, sy=%d, "
                        "src.w=%d,src.h=%d)!", w, h, sx, sy, source.width(), source.height());
        return false;
    }

    if (destination.width()    == w && destination.height() == h &&
        destination.depth()    == source.depth() &&
        destination.byteCount() >= w*h)
    {
        if (source.colorCount() > 0)
        {
            destination.setColorCount(source.colorCount());
            for (int i = 0; i < destination.colorCount(); ++i)
            {
                destination.setColor(i, source.color(i));
            }
        }

        // now copy the single lines from the source to the target image
        int fill_bytes = destination.bytesPerLine() - w_copy;
        const unsigned char *psrc;
        unsigned char *pdst;
        int iy;
        for (iy = 0; iy < h_copy; ++iy)
        {
            pdst = destination.scanLine(iy);
            psrc = source.scanLine(iy + sy);
            memcpy(pdst, psrc + sx, w_copy);
            memset(pdst + w_copy, 0, fill_bytes);
        }
        for (iy = h_copy; iy < h; ++iy)
        {
            pdst = destination.scanLine(iy);
            memset(pdst, 0, w);
        }
    }
    else
    {
        destination = source.copy(sx, sy, w, h);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//! This subclass of AVImageTransformation provides an extended interface for
/*! calculating transformation values in a separate thread.
*/
class ImageTransformationMT : public AVImageTransformation
{

public:

    //! Constructor
    explicit ImageTransformationMT(CachingPolicy opts = None)
        : AVImageTransformation(opts)
    {}

public:

    //! Stops the calculation task soon as possible
    inline void stopCalculating(){ m_stopCalculating = true; }
};

///////////////////////////////////////////////////////////////////////////////

//! Calculates the image transformation matrix of a given AVImageTransformation object
//! and notifies that calculation has finished.
class ImageTransformationCalcThread : public AVThread
{

public:

    //! Result code delivered when calculation has finished
    //! \sa sendNotification()
    enum ResCode
    {
        CalcError,
        CalcOk,
        CalcStopped
    };

    explicit ImageTransformationCalcThread(ImageTransformationMT* imageTransformation,
        const QSize& maxInputSize, const QSize& maxOutputSize)
          : AVThread("ImageTransformationCalcThread"),
            m_imageTransformation(imageTransformation),
            m_maxInputSize(maxInputSize),
            m_maxOutputSize(maxOutputSize)
    {}

    //! Destructor
    ~ImageTransformationCalcThread() override {}

    //! runs the thread and calculates the transformation values
    void runThread() override
    {
        LOGGER_ROOT.Write( LOG_DEBUG, "ImageTransformationCalcThread::runThread: thread running");

        QTime time;
        time.start();
        bool res = m_imageTransformation->
            calculateTransformationValues(m_maxInputSize, m_maxOutputSize);

        bool stopped = checkForStop();

        if(res && !stopped)
        {
            LOGGER_ROOT.Write( LOG_DEBUG, "ImageTransformationCalcThread::runThread: "
                "calculating image transformation table in %d ms",
                    time.elapsed() );

            sendNotification(CalcOk);
            return;
        }

        sendNotification(stopped ? CalcStopped : CalcError);
    }

    //! stops the thread
    void stopThread() override { m_imageTransformation->stopCalculating(); }

protected:

    ImageTransformationMT* m_imageTransformation;
    QSize m_maxInputSize, m_maxOutputSize;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AVImageProjectionProcessor::
AVImageProjectionProcessor(AVImageTransformation::CachingPolicy caching_opts,
                         QObject* parent, const char* name)
    : QObject( parent ),
    m_imageTransformation( 0 ),
    m_calculator( 0 ),
    m_calcThread( 0 ),
    m_sourceAndTargetAreEqual( false ),
    m_calculationFinished( true ),
    m_totalCalcSteps(0)
{
    setObjectName(name);
    m_imageTransformation = new ImageTransformationMT( caching_opts );
    AVASSERT(m_imageTransformation != 0);

    m_imageTransformation->setCalculator( 0 );
}

///////////////////////////////////////////////////////////////////////////////

AVImageProjectionProcessor::~AVImageProjectionProcessor()
{
    if(m_calcThread->running())
    {
        if(!m_calcThread->stop())
        {
            LOGGER_ROOT.Write(LOG_ERROR,
                              "AVImageProjectionProcessor::~AVImageProjectionProcessor: "
                              "could not stop calculation.");
            return;
        }
    }

    delete m_calcThread;
    delete m_imageTransformation;
    delete m_calculator;
}

///////////////////////////////////////////////////////////////////////////////

void AVImageProjectionProcessor::setupProjection(AVCoordinateTransformBase* source_trafo,
                                                 AVCoordinateTransformBase* target_trafo )
{
    AVASSERT(source_trafo);
    AVASSERT(target_trafo);

    if( !m_calculationFinished )
    {
        AVLogError <<  "AVImageProjectionProcessor::setupProjection: "
                    "Can not setup projection, there is a pending calculation task";
        return;
    }

    LOGGER_ROOT.Write( LOG_DEBUG, "AVImageProjectionProcessor::setupProjection(%s): "
        "Source trafo '%s' : Target trafo '%s' ", qPrintable(objectName()), qPrintable(source_trafo->uniqueName()),
            qPrintable(target_trafo->uniqueName()) );

    m_imageTransformation->setCalculator( 0 );
    delete m_calculator;
    m_calculator = 0;

    //setup calculator iff source and target trafo are different
    if( source_trafo->uniqueName() != target_trafo->uniqueName() )
    {
        m_calculator = new AVProjectionTransformationCalculator( source_trafo, target_trafo );
        AVASSERT(m_calculator != 0);

        m_imageTransformation->setCalculator( m_calculator );
        return;
    }

    LOGGER_ROOT.Write( LOG_DEBUG, "AVImageProjectionProcessor::setupProjection: "
        "Source and target trafo are equal, trafo name '%s'",
            qPrintable(source_trafo->uniqueName()) );
}

///////////////////////////////////////////////////////////////////////////////

void AVImageProjectionProcessor::calculateProjection(const QSize& minInputSize,
                                                     const QSize& maxOutputSize )
{
    if( !m_calculationFinished )
    {
        AVLogError <<  "AVImageProjectionProcessor::calculateProjection: "
                    "Can not calculate projection, there is a pending calculation task";
        return;
    }

    m_calculationFinished = false;
    m_imageTransformation->resetProgressInfo();

    m_totalCalcSteps = maxOutputSize.width() + maxOutputSize.height();

    delete m_calcThread;
    m_calcThread = new ImageTransformationCalcThread( m_imageTransformation, minInputSize,
        maxOutputSize);
    AVASSERT(m_calcThread != 0);
    m_calcThread->connectNotification( this, SLOT(slotCalculationFinished(int)) );
    m_calcThread->start(QThread::LowPriority);
    return;
}

///////////////////////////////////////////////////////////////////////////////

bool AVImageProjectionProcessor::stopProcessing()
{
    if(m_calculationFinished)
        return true;

    if(!m_calcThread->stop())
    {
        return false;
    }

    m_calculationFinished = true;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

QImage AVImageProjectionProcessor::
applyProjection(const QImage& image, const QSize& outputSize) const
{
    if( !m_calculationFinished )
    {
        AVLogError <<  "AVImageProjectionProcessor::applyProjection: "
            "Can not apply projection, calculation task hasn't finished";
        return QImage();
    }

    // at the moment only images with a 32bit color depth are supported
    QImage targetImage( outputSize, QImage::Format_ARGB32 );

    if ( !m_imageTransformation->transformImage( image, targetImage ) )
    {
        AVLogError <<  "AVImageProjectionProcessor::applyProjection: "
                    "applying projection transformation failed";

                    AVASSERT(0);
    }

    return targetImage;
}

///////////////////////////////////////////////////////////////////////////////

void AVImageProjectionProcessor::slotCalculationFinished(int res)
{
     if( res == ImageTransformationCalcThread::CalcOk )
     {
        m_maxOutputSize = m_imageTransformation->maxOutputSize();
        m_calculationFinished = true;
        emit signalCalculationFinished();
        m_totalCalcSteps = 0;
        return;
     }

     AVLogError <<  "AVImageProjectionProcessor::slotApplyTransformation: "
                "calculating image projection failed";
}

///////////////////////////////////////////////////////////////////////////////

int AVImageProjectionProcessor::progressOfCalculation() const
{
    return m_imageTransformation->progressOfCalculation();
}

///////////////////////////////////////////////////////////////////////////////

int AVImageProjectionProcessor::totalCalculationSteps() const
{
    return m_totalCalcSteps;
}

///////////////////////////////////////////////////////////////////////////////

// End of File
