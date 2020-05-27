///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Andrei Malashkin, a.malashkin@avibit.com
    \brief   Tests of the host resolution
*/


// Qt includes
#include <QString>
#include <QTest>
#include <avunittestmain.h>
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QHostInfo>

// AVLib includes
#include "avmisc.h"
#include "avlog.h"

////////////////////////////////////////////////////////////////////////////////

class AvResolveHostTest : public QObject
{

    Q_OBJECT
public:
    AvResolveHostTest()
    {}

private slots:

////////////////////////////////////////////////////////////////////////////////

    /*!
     * \brief testHostNameResolve_data test reads a host file and try to resolve all pairs of host-ip with AVResolveHost function.
     * This test can also be used as a manual test, if a tester overwrites hosts file manually.
     *
     * Examples of hosts file:
     * [1]
     * 127.0.0.1   localhost.localdomain localhost4 localhost4.localdomain4 localhost
     * ::1         localhost.localdomain localhost6 localhost6.localdomain6 localhost
     *
     * [2]
     * ::1         localhost localhost.localdomain localhost6 localhost6.localdomain6
     * 127.0.0.1   localhost localhost.localdomain localhost4 localhost4.localdomain4
     */
    void testHostNameResolve_data()
    {
        QTest::addColumn<QString>("host_name");
        QTest::addColumn<QString>("ip_address");
        QTest::newRow("placeholder") << "" << "";

        QString hosts_path =
        #if defined(Q_OS_UNIX)
                "/etc/hosts";
        #elif defined(Q_OS_WIN32)
                "C:\\Windows\\System32\\drivers\\etc\\hosts";
        #endif

        QFile hosts_file(hosts_path);
        if (!hosts_file.open(QIODevice::ReadOnly | QIODevice::Text))
                AVASSERTMSG(false, "No hosts file");

        QStringList hosts_lines;
        {
            AVLOG_ENTER("hosts file content:");
            while (!hosts_file.atEnd())
            {
                QByteArray raw_line = hosts_file.readLine();
                QString line(raw_line);
                line.remove(QRegularExpression("#.*"));
                hosts_lines << line;

                AVLogInfo << line;
            }
        }

        QHash<QString, QString> hosts_to_ip_mapping;
        for(const QString& line : hosts_lines)
        {
            QStringList words = line.split(QRegularExpression("\\s"), QString::SkipEmptyParts);

            if(words.size() < 2)
                return;
            QString ip_address = words.takeFirst();
            bool is_ip_v4 = ip_address.contains(".");
            for(const QString& host : words)
            {
                if(is_ip_v4 || !hosts_to_ip_mapping.contains(host))
                {
                    hosts_to_ip_mapping[host] = ip_address;
                }
            }
        }

        QHashIterator<QString, QString> iter(hosts_to_ip_mapping);
        while (iter.hasNext())
        {
            iter.next();
            AVLogWarning << "test data: " << iter.key() << " " << iter.value();
            QTest::newRow("hosts_test") << iter.key() << iter.value();
        }
    }

////////////////////////////////////////////////////////////////////////////////

    void testHostNameResolve()
    {
        QFETCH(QString, host_name);
        QFETCH(QString, ip_address);
        ip_address = QHostAddress(ip_address).toString();

        QHostAddress host_address;
        AVResolveHost(host_name, host_address);

        AVLogInfo << "expected ip: " << host_address.toString() << " got: " << ip_address;

        QCOMPARE(host_address.toString(), ip_address);
    }

////////////////////////////////////////////////////////////////////////////////
};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AvResolveHostTest,"avlib/unittests/config")
#include "avresolvehosttest.moc"


// End of file
