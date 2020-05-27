/////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
// QT-Version: QT4
//
// Module:    AVLIB - Avibit Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Michael Gebetsroither, m.gebetsroither@avibit.com
    \author  QT4-PORT: Gerhard Scheikl, g.scheikl@avibit.com
    \brief
*/

#ifndef __AVCrc__
#define __AVCrc__

// Qt includes
//#include <QObject>
#include <QString>
#include <QTextStream>

// avcommon library
//#include <avlog.h>

// local includes

// forward declarations

template<typename T, typename DH, T TruncPoly>
struct AVCrcTablePrivate
{
    static const T WIDTH = 8 * sizeof(T);
    static const T TOP_BIT = static_cast<T>(1 << (WIDTH - 1));

    static const unsigned SIZE = 0x100;
    typedef DH table_type[SIZE];
    static table_type table;

    static void initTable()
    {
        static bool is_initialised = false;
        if(is_initialised)
            return;

        for (unsigned i = 0; i < SIZE; ++i)
        {
            T reg = i << (WIDTH - 8);
            for (int j = 0; j < 8; ++j)
            {
                bool topBit = (reg & TOP_BIT) != 0;
                reg <<= 1;
                if (topBit)
                    reg ^= TruncPoly;
            }
            table[i] = reg;
        }
        is_initialised = true;
    }
};

template<typename T, typename DH, T TruncPoly>
typename AVCrcTablePrivate<T, DH, TruncPoly>::table_type
AVCrcTablePrivate<T, DH, TruncPoly>::table = { 0 };

//! CRC processing engine
/*! \param  T                   Type of crc (eg. quint16 for 16bit or Q_UINY32 for 32bit)
 *  \param  DH                  Type of data holder (MUST _NOT_ be smaller than T, but should be the
 *                                  fastest type on the system), usually unsigned long
 *  \param  TruncPoly           Truncated polynom for crc
 *  \param  InitialRemainder    Initial Remainder
 *  \param  FinalXor            Value for the final XOR step
 *
 *  Notes:
 *      Predefined type for ccitt crc.
 *          \code
 *          typedef AVCrc<quint16, unsigned long, 0x1021, 0xFFFF, 0> AVCrcCcitt;
 *          \endcode
 *      Predefined type for crc used in arinc 622 protocoll.
 *          \code
 *          typedef AVCrc<quint16, unsigned long, 0x1021, 0xFFFF, 0xFFFF> AVCrcAcars;
 *          \endcode
 *
 *  Usage:
 *  \code
 *      AVCrcCcitt crc;
 *      char data[] = "123456789";
 *      crc.processBytes(data, sizeof(data) - 1);
 *      quint16 crc_sum = crc();
 *      or
 *      quint16 crc_sum = crc.checksum();
 *  \endcode
 *
 *  Implementation Notes:
 *      For additional features such as reflection of input, don't use a
 *      reflection table.
 *      Simply reflect the crc table itself (both the index and the value).
 *      As a result CrcTable needs another template argument (e.g. bool reflect).
 *      For remainder reflection it's a good idea to use a reflection function
 *      (because it's rarly used therefor a reflection table would only pollute
 *      the caches).
 */
template<typename T, typename DH, T TruncPoly, T InitialRemainder, T FinalXor>
class AVCrc
{
    // to give access to the crc table
    friend class AVCrcTest;

public:
    //! Constructs an empty class of AVCrc
    AVCrc() : m_rem(InitialRemainder)
    {
        CrcTable::initTable();
    }

    //! Copy constructor
    AVCrc(const AVCrc &src) { m_rem = src.m_rem; }

    //! Destructor
    ~AVCrc() {}

    //! Assignment operator
    AVCrc& operator=(const AVCrc &src)
    {
        if(this == &src)
            return *this;

        m_rem = src.m_rem;
        return *this;
    }

    bool operator==(const AVCrc &src) const { return m_rem != src.m_rem ? false : true; }
    bool operator!=(const AVCrc &src) const { return !operator==(src); }

    //! Calculate CRC from one byte
    /*! \param byte Byte to calculate crc from
     */
    inline void processByte (unsigned char byte)
    {
        processBytes_p(&byte, 1);
    }

    //! Calculate CRC from byte_count many bytes pointed to by data
    /*! \param  data        Pointer to data to calculate crc from
     *  \param  byte_count  Number of bytes to calculate crc from
     */
    void processBytes(void const *data, unsigned long byte_count)
    {
        if(!data || !byte_count)
            return;
        processBytes_p(data, byte_count);
    }

    //! Returns the CRC result (XORed with the FinalXor value given as template parameter)
    inline T checksum() const { return m_rem ^ FinalXor; }

    //! Returns the remainder without xoring it
    T getInterimRemainder() const { return m_rem; }

    //! Reset the crc object
    /*! \param new_remainder    Set this as the new remainder, per default the remainder given as
     *                              template parameter
     *
     * NOTE:
     *  This function together with getInterimRemainder could be used to save and restore the
     *  state of an CRC object.
     *  \sa getInterimRemainder
     */
    void reset(T new_remainder = InitialRemainder) { m_rem = new_remainder; }

    //! Prints information about the crc object through qDebug
    void printInfo()
    {
        QString tmp;
        QTextStream stream(&tmp);
        getInfo(stream);
        qDebug("%s", tmp.toLocal8Bit().constData());
    }

    //! Prints the same information as printInfo but to the given stream
    void getInfo(QTextStream &stream)
    {
        stream << "CRC info:\n";
        stream << "WIDTH   = " << CrcTable::WIDTH << "\n";
        stream << "TOP_BIT = 0x" << hex << CrcTable::TOP_BIT << "\n";
        stream.setPadChar('0');
        bool begin = false;
        unsigned j = 0;
        for(unsigned i = 0; i<256; ++i) {
            if(begin)
                stream << ", ";
            stream << "0x" << hex;
            stream.setFieldWidth(CrcTable::WIDTH/4);
            stream << CrcTable::table[i];
            if(++j == 8) {
                j = 0;
                if(i != 255)
                    stream << ",\n";
                begin = false;
            } else {
                begin = true;
            }
        }
    }

protected: // members
    typedef AVCrcTablePrivate<T, DH, TruncPoly> CrcTable;

    DH m_rem;       //!< Remainder of the previous crc operations

protected: // methods
    inline void processBytes_p(void const *data, unsigned long byte_count)
    {
        unsigned char const* in = static_cast<unsigned char const*>(data);
        for(unsigned long i = 0; i<byte_count; ++i) {
            unsigned char top = in[i] ^ (m_rem >> (CrcTable::WIDTH - 8));
            m_rem = (m_rem << 8) ^ CrcTable::table[top];
        }
    }
};

typedef AVCrc<quint16, unsigned long, 0x1021, 0xFFFF, 0> AVCrcCcitt;
typedef AVCrc<quint16, unsigned long, 0x1021, 0xFFFF, 0xFFFF> AVCrcAcars;

#endif

// vim: foldmethod=marker
// End of file
