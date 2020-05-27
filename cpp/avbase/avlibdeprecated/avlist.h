//////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
//////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Dr. Thomas Leitner, t.leitner@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   List helper classes for avmsgsbase.h

    ASTOS processes communicate by transmitting and receiving messages.
    Each message serves for a special purpose and can convey only certain
    variables. In the program, each message is just a class which can be
    used arbitrarily. The base class has methods to serialize and de-
    derialize the classes contents from/to a AVDataStream.

    A serialized message contains a header and a body. The message
    header contains the following variables which are defined in the
    base class:

    \code

    quint32  len;        // message length,     size = 4 bytes
    qint8    version;    // message version,    size = 1 byte
    qint8    type;       // message type,       size = 1 byte
    QDateTime dt;         // message time stamp, size = 8 bytes
    uchar[6]  src;        // message source addr,size = 6 bytes
    uchar[6]  dst;        // message dest addr,  size = 6 bytes
                                    Total header size = 26 Bytes
    \endcode
*/

#if !defined(AVLIST_H_INCLUDED)
#define AVLIST_H_INCLUDED

// local includes
#if !defined(AVMSGSBASE_H_INCLUDED)
#include "avmsgsbase.h"
#endif
#include "avlog.h"
#include "avmultimap.h"
#include "avptrlist.h"

///////////////////////////////////////////////////////////////////////////////
//                             LIST HELPER CLASS
///////////////////////////////////////////////////////////////////////////////

//! List template class to provide a deep copy method.
/*! The standard QT QPtrList class does not provide a copy constructor or copy
    method to create deeply copied lists from already existing lists. This
    template class is derived from QPtrList and provides these facilities.

    Additionally the class now supports different list entry versions.
    The reason why list entry versions are necessary is, that there are
    cases where it is necessary to add more variables to a list entry
    class. In order to facilitate old clients to read new messages and
    new clients to read old messages, message versions are necessary.

    Since list entries are written to the datastream in consecutive order,
    new list data elements are required to be written after all old list
    data elements were written to the datastream. The following figure
    depicts this situation:

    \code

    Notation:      H ......... header
                   A B C ..... data elements of the list entries.
                   N ......... a new data element in a list entry.

    OLD MESSAGE:   H A B C A B C A B C A B C

    NEW MESSAGE:   H A B C A B C A B C A B C N N N N

    \endcode

    The old message contains a header (defined in AVMsgBase) and four
    list entries consisting of the data elements A, B and C. Now a fourth
    data element called N is to be added to a list entry. The new
    message will have the new data elements appended at the end so that
    that old clients still are able to read the message.
*/
namespace AVListHelper {
    template <typename T> T* copyElement(T *other) {
        if (other == 0) {
            AVLogger->Write(LOG_FATAL, "AVListHelper<T>::copyElement: "
                            "Found NULL-Pointer in list.");
            AVASSERT(false);
        }

        T *ne = new T(*other);
        AVASSERT(ne != 0);
        return ne;
    }
}

template<class T> class AVList : public AVPtrList<T>
{
public:
    //! create an empty list.
    AVList() : AVPtrList<T>() {}

    //! create a copy of the specified list. DEEP copy!
    //! For this reason, you should never use foreach() on an AVList! (slow for const, wrong for non-const)
    AVList(const AVList<T> &s) : AVPtrList<T>() {
        if (&s == this) return;
        copyList(s);
    }

    //! assign one list to the other: this does a DEEP copy!
    AVList<T> &operator=(const AVList<T> &s) {
        if (&s == this) return *this;
        copyList(s);
        return *this;
    }

    //! assign a AVPtrList to our AVList: this does a SHALLOW copy!
    AVList<T> &operator=(const AVPtrList<T> &s) {
        if (&s == this) return *this;
        this->clear();
        AVPtrList<T>::operator=(s);
        return *this;
    }

    //! clear the current list and copy the other. DEEP copy;
    void copyList(const AVList<T> &s) {
        this->clear();

        for (T *e : s)
            this->append(AVListHelper::copyElement(e));
    }

    //! write the list to the specified datastream.
    /*! \param msg the message to write the list for
        \param s the data data stream to write the list on
        \param vers the message version to write
        \param allv true to write all versions, false to write only the specified version
        \param write_num when true, the number of listentries is written to
        the stream (needed for FP version HACK!)
    */
    bool writeList(AVMsgBase *msg, AVDataStream &s, int vers,
                   bool allv = true, bool write_num = true)
    {
        if (msg == 0) return false;
        if (write_num) s << static_cast<quint32>(this->count());
        if (!AVMsgBase::checkIOStatus(s)) return false;

        AVVersionedDataStream vs(s);
        for (int v = 0; v <= vers; v++) {
            if (!allv) v = vers;
            vs.setVersion(v);
            for (uint i = 0; i < this->count(); i++) {
                T *e = this->at(i);

                if (e != 0)
                    vs << *e;

                if (!AVMsgBase::checkIOStatus(vs)) {
                    s.setStatus(vs.status());
                    return false;
                }
            }
        }

        return true;
    }

    //! read the list from the specified datastream.
    /*! \param msg the message to read the list for
        \param s the data data stream to read the list on
        \param vers the message version to read
        \param allv true to read all versions, false to read only the specified version
        \param read_num when true, the number of listentries is read from
        the stream (needed for FP version HACK!)
    */
    bool readList(AVMsgBase *msg, AVDataStream &s, int vers,
                  bool allv = true, bool read_num = true)
    {
        if (msg == 0) return false;
        qint32 n;

        if (read_num) s >> n;
        else n = this->count();

        if (!AVMsgBase::checkIOStatus(s)) return false;

        AVVersionedDataStream vs(s);
        for (int v = 0; v <= vers; v++) {

            if (!allv) v = vers;
            if (v == 0) this->clear();

            T *e;
            vs.setVersion(v);

            for (uint i = 0; i < static_cast<uint>(n); i++)
            {
                if (v == 0)
                {
                    e = new T;
                    AVASSERT(e != 0);
                    this->append(e);
                }
                else
                {
                    e = this->at(i);
                }

                if (e != 0)
                    vs >> *e;

                if (!AVMsgBase::checkIOStatus(vs)) {
                    s.setStatus(vs.status());
                    return false;
                }
            }
        }

        AVASSERT(n == this->count());

        return true;
    }

    //! Returns pointer pairs to all duplicates found in the list.
    /*! The item type's equality operator== is used for identity checking.

        Returns the number of duplicate pairs found in the list.
      */
#ifdef KDAB_EXCLUDED
//    uint getDuplicates(AVMultiMap<T*,T*> &duplicatesMap) {
//        uint duplicates = 0;
//        duplicatesMap.clear();

//        QListIterator<T*> it1 = this->javaIterator();
//        T *item1;
//        QListIterator<T*> it2 = this->javaIterator();
//        T *item2;

//        while ( it1.hasNext() ) {
//            item1 = it1.next();
//            it2.toBack();
//            while ( it2.hasPrevious() && it2.peekPrevious() != item1 ) {
//                item2 = it2.previous();
//                if ( *item1 == *item2 ) {
//                    duplicatesMap.insert(item1, item2);
//                    duplicates++;
//                }
//            }
//        }

//        return duplicates;
//    }
#endif

    //! write the list to the specified datastream.
    bool writeList(AVDataStream &s, int vers) {
        s << static_cast<quint32>(this->count());
        if (!AVMsgBase::checkIOStatus(s)) return false;

        AVVersionedDataStream vs(s);
        for (int v = 0; v <= vers; v++)
        {
            vs.setVersion(v);
            for (uint i = 0; i < this->count(); i++) {
                T *e = this->at(i);

                if (e != 0)
                    vs << *e;

                if (!AVMsgBase::checkIOStatus(vs)) {
                    s.setStatus(vs.status());
                    return false;
                }
            }
        }

        return true;
    }

    //! read the list from the specified datastream.
    bool readList(AVDataStream &s, int vers) {
        quint32 n;
        s >> n;
        this->clear();
        if (!AVMsgBase::checkIOStatus(s)) return false;

        AVVersionedDataStream vs(s);
        for (int v = 0; v <= vers; v++)
        {
            T *e;
            vs.setVersion(v);
            for (uint i = 0; i < n; i++) {
                if (v == 0)
                {
                    e = new T;
                    AVASSERT(e != 0);
                }
                else
                    e = this->at(i);

                if (e != 0)
                    vs >> *e;

                if (!AVMsgBase::checkIOStatus(vs)) {
                    s.setStatus(vs.status());
                    delete e;
                    return false;
                }

                if (v == 0)
                    this->append(e);
            }
        }

        return true;
    }

    //! Equality operator
    /*! Two AVLists are considered equal, if they have the same size
        and the values store in them are either the same (same pointer)
        or equal according to their operator==.

        \param other the other list

        \return true if the lists are equal, false if not
    */
    bool operator==(const AVList<T>& other) const {
        if (&other == this) return true;
        if (this->count() != other.count()) return false;

        bool result = true;

        typename AVPtrList<T>::const_iterator iter1(this->begin());
        typename AVPtrList<T>::const_iterator iter2(other.begin());

        for (uint i = 0; result && i < this->count(); ++i, ++iter1, ++iter2) {
            T* a = (*iter1);
            T* b = (*iter2);
            if (a == b)
                result = true;
            else if (a != 0 && b != 0)
                result = (*a == *b);
            else
                result = false;
        }

        return result;
    }
};

//! operator to write the specified list to the specified datastream.
template <class T> AVDataStream& operator<<(AVDataStream &s, const AVList<T> &l)
{
    s << static_cast<quint32>(l.count());
    for (uint i = 0; i < l.count(); i++) {
        T *e = l.at(i);
        if (e != 0) s << *e;
    }
    return s;
};

//! operator to read the specified list from the specified datastream.
template <class T> AVDataStream& operator>>(AVDataStream &s, const AVList<T> &l)
{
    quint32 n;
    s >> n;
    l.clear();
    for (uint i = 0; i < n; i++) {
        T *e = new T;
        AVASSERT(e != 0);
        s >> *e;
    }
    return s;
};

#endif
// End of file
