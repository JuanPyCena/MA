///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    AVUNITTESTHELPERLIB - AVCOMMON Unit Test helper library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Gerhard Scheikl, g.scheikl@avibit.com
    \brief   AVUnittestHelpers implementation
 */

#include <iostream>

// Qt includes
#include <QProcess>
#include <QTemporaryFile>
#include <QTest>

// avlib includes
#include "avenvironment.h"

// local includes
#include "avunittesthelpers.h"

//--------------------------------------------------------------------------------------------------

std::unique_ptr<QTemporaryDir> AVUnittestHelpers::m_temporary_home_folder;

//--------------------------------------------------------------------------------------------------

QString AVUnittestHelpers::setTemporaryHome(const QString &application_name)
{
    const QByteArray func_info = AV_FUNC_INFO;

    for (int i=0; i<10; ++i)
    {
        // work around strange isValid constraints
        m_temporary_home_folder = std::make_unique<QTemporaryDir>();
        if (m_temporary_home_folder->isValid()) break;
        std::cerr << "** ERROR: " << func_info.data() << ": "
                  << qPrintable(m_temporary_home_folder->errorString()) << std::endl;
    }

    if (!m_temporary_home_folder->isValid())
    {
        std::cerr << "** ERROR: " << func_info.data() << ": " << "giving up";
        return ".";
    }

    // in case of success, logging will be done later in the unit test main function (when a logger is available)
    AVEnvironment::setEnv(application_name + "_HOME", m_temporary_home_folder->path());

    AVASSERTNOLOGGER(QDir::setCurrent(m_temporary_home_folder->path()));

    if (AVEnvironment::getEnv("AV_KEEP_TEMPORARY_UNIT_TEST_HOME", false, AVEnvironment::NoPrefix) == "1")
    {
        m_temporary_home_folder->setAutoRemove(false);
    }

    return m_temporary_home_folder->path();
}

//--------------------------------------------------------------------------------------------------

void AVUnittestHelpers::kdiff3(const QByteArray &actual, const QByteArray &expected, bool blocking)
{
    QTemporaryFile expecte_filed;
    expecte_filed.open();
    expecte_filed.write(expected);
    expecte_filed.close();

    QTemporaryFile actual_file;
    actual_file .open();
    actual_file.write(actual);
    actual_file.close();

    if(blocking)
    {
        if(QProcess::execute("diff", QStringList() << "-q" << expecte_filed.fileName() << actual_file.fileName()))
        {
            QProcess::execute("kdiff3", QStringList() << expecte_filed.fileName() << actual_file.fileName()
                              << "--L1" << "expected output" << "--L2" << "actual output");
        }
    }
    else
    {
        QProcess::startDetached("kdiff3", QStringList() << expecte_filed.fileName() << actual_file.fileName());
    }
}

//--------------------------------------------------------------------------------------------------

void AVUnittestHelpers::kdiff3(const QString &actual, const QString &expected, bool blocking)

{
    kdiff3(expected.toLocal8Bit(), actual.toLocal8Bit(), blocking);
}

//--------------------------------------------------------------------------------------------------

// End of file
