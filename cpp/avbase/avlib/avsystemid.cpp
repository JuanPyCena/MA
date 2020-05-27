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
    \brief   Class to find out the system id of a computer
*/



// system includes
#include <cstdio>

// local includes
#include "avlog.h"
#include "avmisc.h"
#include "avsystemid.h"

///////////////////////////////////////////////////////////////////////////////
AVSystemId::AVSystemId()
{
}

///////////////////////////////////////////////////////////////////////////////
AVSystemId::~AVSystemId()
{
}

///////////////////////////////////////////////////////////////////////////////
//                    L I N U X  I m p l e m e n t a t i o n 
///////////////////////////////////////////////////////////////////////////////

#if defined(Q_OS_LINUX)
#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/route.h>
#include <net/if.h>

QString AVSystemId::getSID()
{
    char sid[20];
    
    int i, s;
    struct ifreq *ifrq, ifreqs[32];
    struct ifconf ifc;
    struct sockaddr *sa;

    // clear struct 

    memset(ifreqs, 0, sizeof(ifreqs));

    // get socket and the list of interfaces 

    s = socket(AF_INET, SOCK_STREAM, 0);
    ifc.ifc_req = ifreqs;
    ifc.ifc_len = sizeof(ifreqs);
    ioctl(s, SIOCGIFCONF, &ifc);

    // loop through list 

    for (ifrq = ifreqs; ifc.ifc_len > 0; ifrq++, ifc.ifc_len-=sizeof(*ifrq)) {

        // get interface flags and HW addr for this interface 

        ioctl(s, SIOCGIFFLAGS, ifrq);
        ioctl(s, SIOCGIFHWADDR, ifrq);

        // if its a non loopback interface && hw addr != 0 -> break 

        sa = &ifrq->ifr_hwaddr;
        if (!(ifrq->ifr_flags & IFF_LOOPBACK) &&
            (static_cast<unsigned char>(sa->sa_data[0]) != 0 ||
             static_cast<unsigned char>(sa->sa_data[1]) != 0 ||
             static_cast<unsigned char>(sa->sa_data[2]) != 0 ||
             static_cast<unsigned char>(sa->sa_data[3]) != 0 ||
             static_cast<unsigned char>(sa->sa_data[4]) != 0 ||
             static_cast<unsigned char>(sa->sa_data[5]) != 0)) break;
    }
    if (ifc.ifc_len <= 0) return "";
       
    sa = &ifrq->ifr_hwaddr;

    int pos = 0;
    for (i = 0; i < 6; i++) {
        sprintf(&sid[pos], "%02x", static_cast<unsigned char>(sa->sa_data[i]));
        pos += 2;
    }
    sid[pos] = '\0';    
    
    QString str(sid);
    return str.toUpper();
}

///////////////////////////////////////////////////////////////////////////////
//                   W i n d o w s   I m p l e m e n t a t i o n 
///////////////////////////////////////////////////////////////////////////////

#elif defined(Q_OS_WIN32)

#include <windows.h>
#include <winsock.h>
#include <wincon.h>
 
typedef struct _ASTAT_ {
   ADAPTER_STATUS adapt;
   NAME_BUFFER    NameBuff [30];
} ASTAT;
 
QString AVSystemId::getSID()
{
    QString sid("UNKNOWN");
    int lnum = 0;
    ASTAT Adapter;
    NCB Ncb;
    UCHAR uRetCode;
 
    // loop until a valid lana num is found, usually this is the first one
    do {
        memset( &Ncb, 0, sizeof(Ncb) );
        Ncb.ncb_command  = NCBRESET;
        Ncb.ncb_lana_num = lnum;
        uRetCode = Netbios( &Ncb );
        if (uRetCode == NRC_BRIDGE) lnum++;
    } while (uRetCode == NRC_BRIDGE && lnum < 200);
    if (uRetCode != 0) {
        AVLogger->Write(LOG_ERROR, "NCBRESET return code is: 0x%x", uRetCode);
        return sid;
    }

    // found & reset a valid LANA, -> get stats
    memset( &Ncb, 0, sizeof (Ncb) );
    Ncb.ncb_command = NCBASTAT;
    Ncb.ncb_lana_num = lnum;
    strcpy( (char*) Ncb.ncb_callname,  "*               " );
    Ncb.ncb_buffer = (unsigned char *) &Adapter;
    Ncb.ncb_length = sizeof(Adapter);
    uRetCode = Netbios( &Ncb );
    if (uRetCode != 0) {
        AVLogger->Write(LOG_ERROR, "NCBASTAT return code is: 0x%x", uRetCode);
        return sid;
    }

    // construct SID string
    AVsprintf(sid, "%02x%02x%02x%02x%02x%02x",
              Adapter.adapt.adapter_address[0],
              Adapter.adapt.adapter_address[1],
              Adapter.adapt.adapter_address[2],
              Adapter.adapt.adapter_address[3],
              Adapter.adapt.adapter_address[4],
              Adapter.adapt.adapter_address[5]);
    return sid;
}

///////////////////////////////////////////////////////////////////////////////

#else

QString AVSystemId::getSID()
{    
    return "UNKNOWN";
}
#endif

// End of File
