///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2010
//
// Module:     AVCALCULATIONLIB - AviBit Calculation Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author    Alexander Randeu, a.randeu@avibit.com
    \author    QT4-PORT: Alexander Randeu, a.randeu@avibit.com
    \brief     Defines scientific constants and transformation formulas
*/


//system includes
#include <iostream>
#include <cmath>

// Qt includes
#include <QStringList>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

// local includes
#include "avsciconst.h"
#include "avmisc.h"

using namespace std;


///////////////////////////////////////////////////////////////////////////////

const double AVSciConst::pi = AV_PI;
const double AVSciConst::T0ISA = 288.15;
const double AVSciConst::Rho0ISA = 1.225;
const double AVSciConst::P0ISA = 101325;
const double AVSciConst::Ttrop = 216.65;
const double AVSciConst::Rair = 287.04;
const double AVSciConst::g = 9.81;
const double AVSciConst::kT = -0.0065;
const double AVSciConst::atrop = 295.07;
const double AVSciConst::a0ISA = 340.29;
const double AVSciConst::gamma = 1.4;
const double AVSciConst::HtropISA = 11000.;

const double AVSciConst::ftTom  = 0.3048;
const double AVSciConst::mToft  = 1./AVSciConst::ftTom;
const double AVSciConst::ktTomps = 0.5144444;
const double AVSciConst::mpsTokt = 1./AVSciConst::ktTomps;
const double AVSciConst::nmTokm  = 1.852;
const double AVSciConst::kmTonm  = 1./AVSciConst::nmTokm;

const double AVSciConst::Re = 6378000.0;
const double AVSciConst::c  = 299792458;

///////////////////////////////////////////////////////////////////////////////

void AVSciConst::convdegTodms(double deg, bool& signbit, int &d, int &m, double &s)
{
    int sign;
    double absdeg;
    double md;

    if(deg < 0) {
        sign = -1;
        absdeg = -deg;
    }
    else {
        sign = 1;
        absdeg = deg;
    }

    d = static_cast<int>(absdeg);
    md = (absdeg - d)*60.0;
    m = static_cast<int>(md);
    s = (md - m)*60.0;
    signbit = sign == -1;
}

///////////////////////////////////////////////////////////////////////////////

void AVSciConst::convdmsTodeg(double &deg, bool signbit, int d, int m, double s)
{
    double sign = signbit ? -1.0 : 1.0;

    if (d < 0 || m < 0 || s < 0)
    {
        AVLogWarning << "AVSciConst::convdmsTodeg: at least one component of the DMS value "
                        "{" << d << " " << m << " " << s << "} is negative - using absolute values instead";
    }

    double absd = qAbs(d);

    deg = absd + static_cast<double>(qAbs(m)/60.0) + static_cast<double>(fabs(s)/3600.0);
    deg *= sign;
}

///////////////////////////////////////////////////////////////////////////////

void AVSciConst::rounddms(bool& signbit, int &d, int &m, double &s, int pr)
{
    if (d < 0 || m < 0 || s < 0)
    {
        AVLogWarning << "AVSciConst::convdmsTodeg: at least one component of the DMS value "
                        "{" << d << " " << m << " " << s << "} is negative - using absolute values instead";
    }

    int absd = qAbs(d);
    int absm = qAbs(m);
    double f = pow(10.0,pr);
    double ss = fabs(s)*f;
    //round the seconds to the given precision
    int ssint = static_cast<int>(ss + 0.5);
    s = static_cast<double>(ssint)/f;

    if(s >= 60.0) {
        s -= 60.0;
        ++absm;
        if(absm >= 60) {
            absm -= 60;
            ++absd;
            if(absd >= 360)
            {
                // Handle leap from -359 59 59.X to +0 0 0
                if (signbit)
                {
                    signbit = false;
                }
                absd -= 360;
            }

            m = absm;
            d = absd;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVSciConst::convdegTodmsString(double deg, int pr)
{
    bool signbit = false;
    int d = 0;
    int m = 0;
    double s = 0.0;
    QString result;

    convdegTodms(deg,signbit,d,m,s);
    rounddms(signbit,d,m,s,pr);

    result = QString("%1%2 %3 %4")
            .arg(signbit ? "-" : "")
            .arg(d)
            .arg(m, 2, 10, QChar('0'))
            .arg(s, pr < 1 ? 2 : pr+3, 'f', pr, QChar('0'));

    return result;
}

///////////////////////////////////////////////////////////////////////////////

double AVSciConst::convdmsStringTodeg(const QString &dms)
{
    double result = 0.0;
    bool ok;
    int d = 0;
    int m = 0;
    double s = 0.0;
    QStringList l;
    l = dms.split(" ");
    if (l.count() != 3) return 0.0;

    bool signbit = false;

    // The angle is negative if the the string representation of the degree component contains a minus sign.
    signbit = l[0].contains(QChar('-'));

    d = qAbs(l[0].toInt(&ok));
    if(ok) {
        m = l[1].toInt(&ok);
        if(ok) {
            s = l[2].toDouble(&ok);
            if(ok) {
                convdmsTodeg(result, signbit, d, m, s);
            }
        }
    }

    if(ok) return result;
    else return 0.0;
}

///////////////////////////////////////////////////////////////////////////////

double AVSciConst::convdmsAIXMStringTodeg(const QString &aixmStr)
{
    static QRegularExpression latDirRegExp(".*([NS])$");
    static QRegularExpression lonDirRegExp(".*([WE])$");
    static QRegularExpression dirNegRegExp(".*([SW])$");

    static QRegularExpression latRegExp(
        "^(([0-8][0-9])([0-5][0-9])([0-5][0-9])(\\.\\d{1,4})?(N|S))$|"
        "^(([0-8][0-9])([0-5][0-9])(\\.\\d{1,8})?(N|S))$|"
        "^(([0-8][0-9])(\\.\\d{1,8})?(N|S))$|"
        "^(900000(\\.0{1,4})?(N|S))$|"
        "^(9000(\\.0{1,8})?(N|S))$|"
        "^(90(\\.0{1,8})?(N|S))$");
    static QRegularExpression lonRegExp(
        "^((((0[0-9])|(1[0-7]))[0-9])([0-5][0-9])([0-5][0-9])(\\.\\d{1,4})?(E|W))$|"
        "^((((0[0-9])|(1[0-7]))[0-9])([0-5][0-9])(\\.\\d{1,8})?(E|W))$|"
        "^((((0[0-9])|(1[0-7]))[0-9])(\\.\\d{1,8})?(E|W))$|"
        "^(1800000(\\.0{1,4})?(E|W))$|"
        "^(18000(\\.0{1,8})?(E|W))$|"
        "^(180(\\.0{1,8})?(E|W))$");

    // if the complete RegEx matches we can use a simpler one for the captured texts
    // stringlist for better performance
    static QRegularExpression latRegExpSimple("(\\d\\d)"    // match index 1 = DEG
                                              "(\\d\\d)?"   // match index 2 = MIN
                                              "(\\d\\d)?"   // match index 3 = SEC
                                              "(\\.\\d+)?"  // match index 4 = decimal part
                                              "([NS])");    // match index 5 = N or S
    static QRegularExpression lonRegExpSimple("(\\d\\d\\d)" // match index 1 = DEG
                                              "(\\d\\d)?"   // match index 2 = MIN
                                              "(\\d\\d)?"   // match index 3 = SEC
                                              "(\\.\\d+)?"  // match index 4 = decimal part
                                              "([WE])");    // match index 5 = E or W


    double ret = 0.0;

    QRegularExpressionMatch matchLatDir = latDirRegExp.match(aixmStr);
    QRegularExpressionMatch matchLonDir = lonDirRegExp.match(aixmStr);
    QRegularExpressionMatch matchDirNeg = dirNegRegExp.match(aixmStr);
    QRegularExpressionMatch matchFull;
    QRegularExpressionMatch matchSimple;

    if (matchLatDir.hasMatch())
    {
        matchFull   = latRegExp.match(aixmStr);
        matchSimple = latRegExpSimple.match(aixmStr);
    }
    else if (matchLonDir.hasMatch())
    {
        matchFull   = lonRegExp.match(aixmStr);
        matchSimple = lonRegExpSimple.match(aixmStr);
    }
    else  // none of NSWE
        return 0.0;

    if (! matchFull.hasMatch()) // neither complete LAT nor LON RegEx matched
        return 0.0;

    // if full RegEx matches we can be sure the simple one matches as well
    AVASSERT(matchSimple.hasMatch());

    QString degreeStr    = matchSimple.capturedRef(1).toString();
    QString minutesStr   = matchSimple.capturedRef(2).toString();
    QString secondsStr   = matchSimple.capturedRef(3).toString();
    QString decimalStr   = matchSimple.capturedRef(4).toString();

    ret = degreeStr.toDouble(); // degrees always given
    if (! minutesStr.isNull()) // minutes given
        ret += (minutesStr.toDouble() / 60.0);
    if (! secondsStr.isNull()) // seconds given
        ret += (secondsStr.toDouble() / 3600.0);
    if (! decimalStr.isNull()) // decimal part given
    {
        if (! secondsStr.isNull()) // belongs to seconds part
            ret += (decimalStr.toDouble() / 3600.0);
        else if (! minutesStr.isNull()) // belongs to minutes part
            ret += (decimalStr.toDouble() / 60.0);
        else // belongs to degrees part
            ret += decimalStr.toDouble();
    }
    if (matchDirNeg.hasMatch()) // negative direction
        ret *= -1.0;

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

double AVSciConst::normaliseDeg(double angle)
{
    double mod = fmod(angle, 360.0);
    if (mod < 0.0) mod += 360.0;
    return mod;
}

// end of file
