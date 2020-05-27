///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Thomas Pock, t.pock@avibit.com
    \author  QT4-PORT: Alexander Randeu, a.randeu@avibit.com
    \brief   Class to check an existing license file
*/



// qt includes
#include "qdatetime.h"
// avcommon includes
#include "avsystemid.h"
#include "avmd5.h"
#include "avlog.h"

// local includes
#include "avlicense.h"

const QString AVLicense::m_licenseFileName = "license.cfg";

///////////////////////////////////////////////////////////////////////////////

AVLicense::AVLicense() : AVConfigBase("", m_licenseFileName)
{
    AVSystemId sid;
    m_sid = sid.getSID();
    readConfig();
}

///////////////////////////////////////////////////////////////////////////////

bool AVLicense::checkLicense(const QString& feature)
{
    AVConfigEntryList* entryList = geteList();

    bool ret = true;
    bool found = false;

    unsigned int i;
    unsigned int n = entryList->count();

    for (i = 0; i < n; i++)
    {
        AVConfigEntry *e = entryList->at(i);

    if ((feature.isEmpty()) || (e->key == feature))
    {
        found  = true;
        // This feature has to be checked !
        AVCVariant &var = e->getVar();
        if (var.type() == AVCVariant::String)
        {
        QString license = var.asString();
        if (license.length() != 40)
        {
            AVLogger->Write(LOG_ERROR,
                    "The license for %s"
                    " is not valid !",
                    qPrintable(e->key));
            return false;
        }
        QString expiry = license.right(8);
        QString compare = makeLicense(e->key, expiry, m_sid);
        if (compare == license)
        {
            int year = expiry.mid(0,4).toInt();
            int month = expiry.mid(4,2).toInt();
            int day = expiry.mid(6,2).toInt();
            QDate date(year, month, day);
            if (QDate::currentDate() > date)
            {
            AVLogger->Write(LOG_ERROR,
                    "The expiry date of the license for %s"
                    " has exceeded !",
                    qPrintable(e->key));
            ret = false;
            }
            else
            {
            AVLogger->Write(LOG_INFO,
                    "License for %s is OK !",
                    qPrintable(e->key));
            }
        }
        else
        {
            AVLogger->Write(LOG_ERROR,
                    "The license for %s"
                    " is not valid !",
                    qPrintable(e->key));
           ret = false;
        }
        }
        else
        {
        AVLogger->Write(LOG_ERROR,
                "Wrong License entry in %s",
                qPrintable(m_licenseFileName));
        return false;
        }
    }
     }

    if (!found)
    {
    ret = false;
    AVLogger->Write(LOG_ERROR,
            "Couldn't find feature %s in file %s",
            qPrintable(feature),
            qPrintable(m_licenseFileName));
    }
     return ret;
}

///////////////////////////////////////////////////////////////////////////////

QString AVLicense::makeLicense(const QString& feature, const QString& expiry,
                               const QString& sid)
{
    // room for terminating \0 required for QString conversion
    char md5[33] = {};
    QString SumString = sid + "-" + feature + "-" + expiry;

    AVMD5EncodeHex(SumString.toLatin1().constData(), md5);

    QString license = QString::fromLatin1(md5) + expiry;
    return license.toUpper();
}

///////////////////////////////////////////////////////////////////////////////

void AVLicense::createLicenseEntry(const QString& feature,
                                   const QString& expiry, const QString& sid)
{
    QString *license = new QString();
    AVASSERT(license != 0);

    ADDCE(feature.toLatin1().constData(),
      license,
      "",
      feature.toLatin1().constData());

    *license = makeLicense(feature, expiry, sid);
}

// End of File
