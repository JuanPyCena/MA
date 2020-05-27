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
    \author  Kevin Krammer, k.krammer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   List of server-port-priority tuples.

    AVServerList parses a string possibly containing a list of servers,
    asociated ports and priorities.
    Servers are stored sorted by priority, the default port is used for all
    servers which have no port specified.
    If no priority is specified, the server will get priority 1
*/

#if !defined(AVSERVERLIST_H_INCLUDED)
#define AVSERVERLIST_H_INCLUDED

// Qt includes
#include <QString>

#include "avlib_export.h"

class AVLIB_EXPORT AVServerList
{
public:
    //! Create a server list
    /*! Create a server list from the given string.
        The resulting list will only contain entries which were specified
        correctly, sorted by priority with maximum priority first.

        The input string format is defined as follows:
        <p>
        <code>
        inputstring ::= address | serverlist<br>
        serverlist ::= server/serverlist | nil<br>
        server ::= address[#port][;priority]<br>
        priority ::= unsigned integer<br>
        port ::= unsigned integer<br>
        address ::= string<br>
        </code>
        </p>
        examples:
        <pre>
            - testserver
                testserver, defaultPort
            - server1;9/server2;8/server3#7200;7
                server1, defaultPort, 9
                server2, defaultPort, 8
                server3, 7200,        7
        </pre>

        \param serverList a NULL terminated char array.
        \param defaultPort the port to use for servers for which no port
               is specified in the serverList parameter.
    */
    AVServerList(const char* serverList, int defaultPort);

    //! Copy constructor
    AVServerList(const AVServerList& other)
    : m_count(0), m_current(0), m_ports(0), m_priorities(0), m_servers(0) {
        if (&other == this) return;
        copy(other);
    }

    //! Assignment operator
    inline AVServerList& operator=(const AVServerList& other) {
        if (&other == this) return *this;
        copy(other);
        return *this;
    }

    //! Destructor
    ~AVServerList();

    //! Returns the number of servers in this list
    inline uint count() const { return m_count; }

    //! Returns the maximum priority of all servers or -1 if there is no server
    inline int maxPriority() const {
        if (m_count > 0)
            return m_priorities[0];
        else
            return -1;
    }

    //! Resets the list
    /*! After this the getXXX methods will return the data of the first
        server in the list.
    */
    inline void first() { m_current = 0; }

    //! Move to the next server
    inline void next() {
        m_current++;
        if (m_current > m_count)
            m_current = m_count;
    }

    //! Resets the list
    /*! After this the getXXX methods will return the data of a server
        with the highest priority.
        If there is more than one server with this priority, a server
        will be chosen randomly.
    */
    void firstAndRandom();

    //! Move to next server
    /*! The next server has less or equal priority to the current one.
        If there is more than one server with this priority, a server
        will be chosen randomly.
    */
    void nextAndRandom();

    //! Returns the current server's address
    /*! If next() or nextByPriority() was called while the list was already
        at or beyond its end, this methods will return QString::null.
    */
    inline QString getServer() const {
        return (m_current < m_count ? m_servers[m_current] : QString::null);
    }

    //! Returns the current server's port
    /*! If next() or nextByPriority() was called while the list was already
        at or beyond its end, this methods will return -1.
    */
    inline int getPort() {
        return (m_current < m_count ? m_ports[m_current] : -1);
    }

    //! Returns the current server's address
    /*! If next() or nextByPriority() was called while the list was already
        at or beyond its end, this methods will return -1.
    */
    inline int getPriority() {
        return (m_current < m_count ? m_priorities[m_current] : -1);
    }


protected:
    uint m_count;           //!< number of servers in list
    uint m_current;         //!< index of the current server or equal m_count

    int* m_ports;           //!< array of the port numbers
    int* m_priorities;      //!< array of the server priorities
    QString* m_servers;     //!< array of the server names or addresses

protected:
    /*! Among all servers that have the same priority as first, choose one
        randomly. first can be the smallest index returned.
    */
    uint randomNext(uint first);

    //! Copy method used by copy constructor and assignment operator
    /*! Copies the other instances member data to this instance, deleting
        any data already present.
    */
    void copy(const AVServerList& other);
};

#endif

// End of file
