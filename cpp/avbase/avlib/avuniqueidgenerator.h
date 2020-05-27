///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Dietmar Gösseringer, d.goesseringer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief
*/

#ifndef __AVUNIQUEIDGENERATOR_H__
#define __AVUNIQUEIDGENERATOR_H__

// Global includes
#include <limits.h>             // for knowing INT_MAX

// QT includes
#include <QtGlobal>             // for class variables of type quint32, ...
#include <QDateTime>            // for class variables of type QTime, QDateTime, ...
#include <QStringList>        // for class variables of type QValueList
#include <QString>              // for class variables of type QString

// AV* includes
#include "avlib_export.h"
#include "avdatetime.h"
#include "avlog.h"             // for knowing AVLogger
#include "avmisc.h"            // for knowing AVsprintf()
#include "avsingleton.h"       // singleton for track number generator
#include "avtimereference.h"   // for accessing AVTimeReference
#include "avuniquenumbergenerator.h" // to generate unique track numbers

//////////////////////////////////////////////////////////////////////////////

//! unique id generator
/*!

    *** DEPRECATED, don't use in new code ***
    Refer to QUuid instead.


    Creates unique ID strings composed of 5 sub-items separated by a separator (default: ":")
    <PREFIX>:<INFO>:<YYYYMMDDhhmmss>:<ID-PER-SECOND-COUNTER>:<UNIQUE-NUMBER>

    The <PREFIX> item can be set with the constructor parameter or the method setPrefix(...)
    The <INFO> item can be set individually for each id with the acquiteId(..) method parameters
    The <YYYYMMDDhhmmss> is set automatically.
    The <ID-PER-SECOND-COUNTER> is set to 0 at each second increment and incremented at each id
    acquisition.
    The <UNIQUE-NUMBER> is generated automatically using the class AVUniqueNumberGenerator.

    Note: Make sure to call setter methods set*() for non-default id creation!
 */
class AVLIB_EXPORT AVUniqueIdGenerator
{

// ------  CONSTRUCTORS, INITIALIZERS, OPERATORS

public:

    //! Standard Constructor
    explicit AVUniqueIdGenerator(
        const QString& prefix = "DEF",
        const QString& separator = ":",
        uint max_unique_number = 65535,
        uint unique_number_postfix_width = 5 ) :
        m_prefix(prefix),
        m_seperator(separator),
        m_counter(0),
        m_unique_number_postfix_width(unique_number_postfix_width)
    {
        m_unique_number_generator.setMinMax(0, max_unique_number);
    }

    //! Destructor
    virtual ~AVUniqueIdGenerator() {};

private:

    //! Hidden copy-constructor
    AVUniqueIdGenerator(const AVUniqueIdGenerator&);

    //! Hidden assignment operator
    const AVUniqueIdGenerator& operator = (const AVUniqueIdGenerator&);

// ------  METHODS, SIGNALS & SLOTS

public:

    // In-between-item SEPARATOR

    //! Set the seperator symbol that separates the ID's items.
    inline void setSeperator(const QString& sep) { m_seperator = sep; }

    //! Returns the seperator symbol that separates the ID's items.
    inline QString getSeperator() { return m_seperator; }

    // Item <PREFIX>

    //! Set the prefix that will be prepended to all internally created IDs.
    inline void setPrefix(const QString& pre) { m_prefix = pre; }

    //! Returns the prefix that will be prepended to all internally created IDs.
    inline QString getPrefix() { return m_prefix; }

    // Item <UNIQUE-NUMBER>

    //! Sets the minimum and maximum numbers to use for the <UNIQUE-NUMBER> item
    inline void setUniqueNumberMinMax(uint minimum, uint maximum)
    { m_unique_number_generator.setMinMax(minimum, maximum); }

    //! Returns the configured minimum track number
    inline uint getUniqueNumberMin() const
    { return m_unique_number_generator.getMin(); }

    //! Returns the configured maximum track number
    inline uint getUniqueNumberMax() const
    { return m_unique_number_generator.getMax(); }

    //! Sets the width of the last trackId item : the unique track number.
    inline void setUniqueNumberPostfixWidth(uint width) { m_unique_number_postfix_width = width; }

    //! Returns the width of the last trackId item : the unique track number.
    inline uint getUniqueNumberPostfixWidth() { return m_unique_number_postfix_width; }

    //! Return the number of free trackIds
    inline uint getNFreeIds() {
        return (m_unique_number_generator.getMax()-m_unique_number_generator.getMin()-
                m_id_list.count());
    }

    //! Return the number of acquired/taken trackIds
    inline uint getNAcquiredIds() { return m_id_list.count(); }

    // Unique Number

    inline long getUniqueNumberFromUniqueId(const QString& id, bool* convok = 0)
    { return id.right(m_unique_number_postfix_width).toLong(convok); }

    // Unique ID

    //! Creates and returns a new ID.
    /*!
        Returns an empty string "" if not successful.
     */
    QString acquireId(const QString& info, bool create_datetime_id = true);

    //! Add externally created IDs (e.g. to avoid creation of such IDs).
    /*!
        Passed string will be taken as such, no prefix will be added.
        Returns TRUE if the ID could be added.
        Returns FALSE if the ID could not be added, e.g. it already exists.
     */
    inline bool addId(const QString& id)
    {
        if (id.isEmpty() || m_id_list.contains(id))
        {
            return false;
        }
        m_id_list.insert(id);
        return true;
    }

    //! Removes the given number from the set of issued numbers.
    /*!
        Returns true if the given number was issued.
     */
    inline bool freeId(const QString& id)
    {
        if (m_id_list.remove(id))
        {
            if (! m_unique_number_generator.freeNumber(getUniqueNumberFromUniqueId(id)) )
            {
                AVLogger->Write(LOG_WARNING,
                    "AVUniqueIdGenerator(%p):freeId: "
                    "Successfully free'd Id '%s' while holding %d Id's "
                    "BUT could not free number %ld while holding %d numbers - AMBIGUOUS!?",
                    this, qPrintable(id), getNAcquiredIds(), getUniqueNumberFromUniqueId(id),
                    m_unique_number_generator.getNAcquiredNumbers());
            }
            return true;
        }
        else
        {
            AVLogger->Write(LOG_WARNING,
                "AVUniqueIdGenerator(%p):freeId: "
                "Could not free Id '%s' while holding %d Id's - not found/registered",
                this, qPrintable(id), getNAcquiredIds());
            return false;
        }
    }

    //! Checks if an ID is already existing.
    /*! Returns TRUE if the ID exists.
        Returns FALSE if the ID does not exist.
     */
    inline bool containsId(const QString& id) { return (m_id_list.contains(id)); }

private:

    //! Helper method: Truncate ms from QDateTime
    QDateTime quantize(const QDateTime& t_in)
    {
        AVDateTime t_out(t_in);
        t_out.setTime(t_out.time().addMSecs(-t_out.time().msec()));
        return t_out;
    }

// ------  MEMBER VARIABLES

protected:

    AVUniqueNumberGenerator m_unique_number_generator;
    QSet<QString>       m_id_list;
    QString             m_prefix;
    QString             m_seperator;
    AVDateTime          m_last_id_creation;
    uint                m_counter;
    uint                m_unique_number_postfix_width;
};

#endif /* __AVUNIQUEIDGENERATOR_H__ */

// End of file
