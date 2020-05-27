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
    \brief   Implementation of AVServerList -- a priorty sorted list of servers
*/



// system includes
#include <iostream>          // solaris 10 needs this for cxxxx includes to work
#include <cstdlib>

// Qt includes
#include <QStringList>

// AVLib includes
#include "avlog.h"
#include "avserverlist.h"

///////////////////////////////////////////////////////////////////////////////

AVServerList::AVServerList(const char* serverList, int defaultPort)
    : m_count(0), m_current(0), m_ports(0), m_priorities(0), m_servers(0)
{
    QStringList servers = QString(serverList).split("/");
    m_count = servers.count();

    if (m_count > 0)
    {
        m_servers = new QString[m_count];
        AVASSERT(m_servers != 0);
        m_ports = new int[m_count];
        AVASSERT(m_ports != 0);
        m_priorities = new int[m_count];
        AVASSERT(m_priorities != 0);
    }

    int i;
    m_count = 0;
    for (i = 0; i < servers.count(); ++i)
    {
        QString server = servers[i];
        int posOfSemi = server.indexOf(";");
        bool prioOk = true;
        int prio = 1;
        if (posOfSemi != -1)
        {
            QString prioString = server.mid(posOfSemi+1, server.length());
            prio = prioString.toInt(&prioOk);

            server = server.left(posOfSemi);
        }
        m_ports[m_current] = defaultPort;

        bool portOk = true;
        int posOfHash = server.indexOf("#");
        if (posOfHash != -1)
        {
            QString portString = server.mid(posOfHash+1, server.length());
            int port = portString.toInt(&portOk);
            if (portOk)
                m_ports[m_current] = port;

            server = server.left(posOfHash);
        }

        if (prioOk && portOk)
        {
            m_servers[m_current] = server;
            m_priorities[m_current] = prio;
            m_current++;
            m_count++;
        }
    }

    bool changed = true;
    while (changed)
    {
        changed = false;
        for (uint i = 1; i < m_count; ++i)
        {
            if (m_priorities[i] > m_priorities[i-1])
            {
                QString server = m_servers[i];
                int port = m_ports[i];
                int prio = m_priorities[i];

                m_servers[i] = m_servers[i-1];
                m_ports[i]   = m_ports[i-1];
                m_priorities[i] = m_priorities[i-1];

                m_servers[i-1] = server;
                m_ports[i-1]   = port;
                m_priorities[i-1] = prio;
                changed = true;
            }
        }
    }

    m_current = 0;
}

///////////////////////////////////////////////////////////////////////////////

AVServerList::~AVServerList()
{
    delete[] m_servers;
    delete[] m_ports;
    delete[] m_priorities;
}

///////////////////////////////////////////////////////////////////////////////

void AVServerList::firstAndRandom()
{
    m_current = randomNext(0);
}

///////////////////////////////////////////////////////////////////////////////

void AVServerList::nextAndRandom()
{
    m_current++;
    if (m_current > m_count)
        m_current = m_count;
    else
    {
        m_current = randomNext(m_current);
    }
}

uint AVServerList::randomNext(uint first)
{
    if (first >= m_count)
        return m_count;

    uint count = 0;
    for (uint i = first; i < m_count; ++i)
    {
        if (m_priorities[i] == m_priorities[first])
        {
            count++;
        }

    }

    if (count == 1)
        return first;

    return (first+ static_cast<int>((static_cast<double>(count)) * rand() / (RAND_MAX+1.0)));
}

///////////////////////////////////////////////////////////////////////////////

void AVServerList::copy(const AVServerList& other)
{
    if (m_count > 0)
    {
        delete[] m_ports;
        delete[] m_priorities;
        delete[] m_servers;
    }

    m_count = other.m_count;
    m_current = other.m_current;
    if (m_count > 0)
    {
        m_ports = new int[m_count];
        AVASSERT(m_ports != 0);
        m_priorities = new int[m_count];
        AVASSERT(m_priorities != 0);
        m_servers = new QString[m_count];
        AVASSERT(m_servers != 0);

        for (uint i = 0; i < m_count; ++i)
        {
            m_ports[i]      = other.m_ports[i];
            m_priorities[i] = other.m_priorities[i];
            m_servers[i]    = other.m_servers[i];
        }
    }
    else
    {
        m_ports      = 0;
        m_priorities = 0;
        m_servers    = 0;
    }
}

// end of file
