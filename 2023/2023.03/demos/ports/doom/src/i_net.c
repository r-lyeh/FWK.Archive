// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------


//#define I_NET_ENABLED

#if defined(I_NET_ENABLED)
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#include "doom_config.h"

#if defined(I_NET_ENABLED)
#if defined(DOOM_WIN32)
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <winsock.h>
#define IPPORT_USERRESERVED 5000
#pragma comment(lib, "ws2_32.lib")
#else
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <sys/ioctl.h>
#define SOCKET int
#endif
#else
#define IPPORT_USERRESERVED 5000
#endif

#include "i_system.h"
#include "d_event.h"
#include "d_net.h"
#include "m_argv.h"
#include "doomstat.h"
#include "i_net.h"


// For some odd reason...
#if !defined(DOOM_APPLE) // It doesn't complain on Win32? O_o
#define ntohl(x) \
        ((unsigned long int)((((unsigned long int)(x) & 0x000000ffU) << 24) | \
                             (((unsigned long int)(x) & 0x0000ff00U) <<  8) | \
                             (((unsigned long int)(x) & 0x00ff0000U) >>  8) | \
                             (((unsigned long int)(x) & 0xff000000U) >> 24)))

#define ntohs(x) \
        ((unsigned short int)((((unsigned short int)(x) & 0x00ff) << 8) | \
                              (((unsigned short int)(x) & 0xff00) >> 8))) \

#define htonl(x) ntohl(x)
#define htons(x) ntohs(x)
#endif


//
// NETWORKING
//

int DOOMPORT = (IPPORT_USERRESERVED + 0x1d);
int DOOMPORT_SEND = (IPPORT_USERRESERVED + 0x1e);

#if defined(I_NET_ENABLED)
SOCKET sendsocket;
SOCKET insocket;

struct sockaddr_in sendaddress[MAXNETNODES];
#endif

void (*netget) (void);
void (*netsend) (void);


void NetSend(void);
doom_boolean NetListen(void);


//
// UDPsocket
//
#if defined(I_NET_ENABLED)
SOCKET UDPsocket(void)
{
    SOCKET s;

    // allocate a socket
    s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0)
    {
        //I_Error("Error: can't create socket: %s", strerror(errno));
        
        doom_strcpy(error_buf, "Error: can't create socket: ");
        doom_concat(error_buf, strerror(errno));
        I_Error(error_buf);
    }

    return s;
}
#endif


//
// BindToLocalPort
//
#if defined(I_NET_ENABLED)
void BindToLocalPort(SOCKET s, int port)
{
    int v;
    struct sockaddr_in address;

    doom_memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = port;

    v = bind(s, (void*)&address, sizeof(address));
    if (v == -1)
    {
        //I_Error("Error: BindToPort: bind: %s", strerror(errno));
        
        doom_strcpy(error_buf, "Error: BindToPort: bind: ");
        doom_concat(error_buf, strerror(errno));
        I_Error(error_buf);
    }
}
#endif


//
// PacketSend
//
void PacketSend(void)
{
#if defined(I_NET_ENABLED)
    int c;
    doomdata_t sw;

    // byte swap
    sw.checksum = htonl(netbuffer->checksum);
    sw.player = netbuffer->player;
    sw.retransmitfrom = netbuffer->retransmitfrom;
    sw.starttic = netbuffer->starttic;
    sw.numtics = netbuffer->numtics;
    for (c = 0; c < netbuffer->numtics; c++)
    {
        sw.cmds[c].forwardmove = netbuffer->cmds[c].forwardmove;
        sw.cmds[c].sidemove = netbuffer->cmds[c].sidemove;
        sw.cmds[c].angleturn = htons(netbuffer->cmds[c].angleturn);
        sw.cmds[c].consistancy = htons(netbuffer->cmds[c].consistancy);
        sw.cmds[c].chatchar = netbuffer->cmds[c].chatchar;
        sw.cmds[c].buttons = netbuffer->cmds[c].buttons;
    }

    //doom_print ("sending %i\n",gametic);                
    c = sendto(sendsocket, (const char*)&sw, doomcom->datalength
               , 0, (void*)&sendaddress[doomcom->remotenode]
               , sizeof(sendaddress[doomcom->remotenode]));
#endif
}


//
// PacketGet
//
void PacketGet(void)
{
#if defined(I_NET_ENABLED)
    int i;
    int c;
    struct sockaddr_in fromaddress;
#if defined(__APPLE__)
    socklen_t fromlen;
#else
    int fromlen;
#endif
    doomdata_t sw;

    fromlen = sizeof(fromaddress);
    c = recvfrom(insocket, (char*)&sw, sizeof(sw), 0
                 , (struct sockaddr*)&fromaddress, &fromlen);
    if (c == -1)
    {
#if defined(DOOM_WIN32)
        int r = WSAGetLastError();
        if (r != WSAEWOULDBLOCK)
        {
            //I_Error("Error: GetPacket: %i", r);
            
            doom_strcpy(error_buf, "Error: GetPacket: ");
            doom_concat(error_buf, doom_itoa(r, 10));
            I_Error(error_buf);
        }
#else
        if (errno != EWOULDBLOCK)
        {
            //I_Error("Error: GetPacket: %s", strerror(errno));
            
            doom_strcpy(error_buf, "Error: GetPacket: ");
            doom_concat(error_buf, strerror(errno));
            I_Error(error_buf);
        }
#endif
        doomcom->remotenode = -1;                // no packet
        return;
    }

    {
        static int first = 1;
        if (first)
        {
            //doom_print("len=%d:p=[0x%x 0x%x] \n", c, *(int*)&sw, *((int*)&sw + 1));
            doom_print("len=");
            doom_print(doom_itoa(c, 10));
            doom_print(":p=[0x");
            doom_print(doom_itoa(*(int*)&sw, 16));
            doom_print(" 0x");
            doom_print(doom_itoa(*((int*)&sw + 1), 16));
            doom_print("] \n");
        }
        first = 0;
    }

    // find remote node number
    for (i = 0; i < doomcom->numnodes; i++)
        if (fromaddress.sin_addr.s_addr == sendaddress[i].sin_addr.s_addr)
            break;

    if (i == doomcom->numnodes)
    {
        // packet is not from one of the players (new game broadcast)
        doomcom->remotenode = -1; // no packet
        return;
    }

    doomcom->remotenode = i; // good packet from a game player
    doomcom->datalength = c;

    // byte swap
    netbuffer->checksum = ntohl(sw.checksum);
    netbuffer->player = sw.player;
    netbuffer->retransmitfrom = sw.retransmitfrom;
    netbuffer->starttic = sw.starttic;
    netbuffer->numtics = sw.numtics;

    for (c = 0; c < netbuffer->numtics; c++)
    {
        netbuffer->cmds[c].forwardmove = sw.cmds[c].forwardmove;
        netbuffer->cmds[c].sidemove = sw.cmds[c].sidemove;
        netbuffer->cmds[c].angleturn = ntohs(sw.cmds[c].angleturn);
        netbuffer->cmds[c].consistancy = ntohs(sw.cmds[c].consistancy);
        netbuffer->cmds[c].chatchar = sw.cmds[c].chatchar;
        netbuffer->cmds[c].buttons = sw.cmds[c].buttons;
    }
#endif
}


int GetLocalAddress(void)
{
#if defined(I_NET_ENABLED)
    char hostname[1024];
    struct hostent* hostentry; // host information entry
    int v;

    // get local address
    v = gethostname(hostname, sizeof(hostname));
    if (v == -1)
    {
        //I_Error("Error: GetLocalAddress : gethostname: errno %d", errno);
        
        doom_strcpy(error_buf, "Error: GetLocalAddress : gethostname: errno ");
        doom_concat(error_buf, strerror(errno));
        I_Error(error_buf);
    }

    hostentry = gethostbyname(hostname);
    if (!hostentry)
    {
        I_Error("Error: GetLocalAddress : gethostbyname: couldn't get local host");
    }

    return *(int*)hostentry->h_addr_list[0];
#else
    return 0;
#endif
}


//
// I_InitNetwork
//
void I_InitNetwork(void)
{
#if defined(I_NET_ENABLED)
#if defined(DOOM_WIN32)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    u_long trueval = 1;
#else
    doom_boolean trueval = true;
#endif
#endif
    int i;
    int p;
    struct hostent* hostentry;        // host information entry

    doomcom = doom_malloc(sizeof(*doomcom));
    doom_memset(doomcom, 0, sizeof(*doomcom));

    // set up for network
    i = M_CheckParm("-dup");
    if (i && i < myargc - 1)
    {
        doomcom->ticdup = myargv[i + 1][0] - '0';
        if (doomcom->ticdup < 1)
            doomcom->ticdup = 1;
        if (doomcom->ticdup > 9)
            doomcom->ticdup = 9;
    }
    else
        doomcom->ticdup = 1;

    if (M_CheckParm("-extratic"))
        doomcom->extratics = 1;
    else
        doomcom->extratics = 0;

    p = M_CheckParm("-port");
    if (p && p < myargc - 1)
    {
        DOOMPORT = doom_atoi(myargv[p + 1]);
        //doom_print("using alternate port %i\n", DOOMPORT);
        doom_print("using alternate port ");
        doom_print(doom_itoa(DOOMPORT, 10));
        doom_print("\n");
    }

    p = M_CheckParm("-sendport");
    if (p && p < myargc - 1)
    {
        DOOMPORT_SEND = doom_atoi(myargv[p + 1]);
        //doom_print("using alternate send port %i\n", DOOMPORT_SEND);
        doom_print("using alternate send port ");
        doom_print(doom_itoa(DOOMPORT_SEND, 10));
        doom_print("\n");
    }

    // parse network game options,
    //  -net <consoleplayer> <host> <host> ...
    i = M_CheckParm("-net");
    if (!i)
    {
        // single player game
        netgame = false;
        doomcom->id = DOOMCOM_ID;
        doomcom->numplayers = doomcom->numnodes = 1;
        doomcom->deathmatch = false;
        doomcom->consoleplayer = 0;
        return;
    }

#if defined(I_NET_ENABLED)
    netsend = PacketSend;
    netget = PacketGet;
    netgame = true;

    // parse player number and host list
    doomcom->consoleplayer = myargv[i + 1][0] - '1';

    doomcom->numnodes = 1;        // this node for sure

    i++;
    while (++i < myargc && myargv[i][0] != '-')
    {
        sendaddress[doomcom->numnodes].sin_family = AF_INET;
        sendaddress[doomcom->numnodes].sin_port = htons(DOOMPORT);
        if (myargv[i][0] == '.')
        {
            sendaddress[doomcom->numnodes].sin_addr.s_addr
                = inet_addr(myargv[i] + 1);
        }
        else
        {
            hostentry = gethostbyname(myargv[i]);
            if (!hostentry)
            {
                //I_Error("Error: gethostbyname: couldn't find %s", myargv[i]);
                
                doom_strcpy(error_buf, "Error: gethostbyname: couldn't find ");
                doom_concat(error_buf, myargv[i]);
                I_Error(error_buf);
            }
            sendaddress[doomcom->numnodes].sin_addr.s_addr
                = *(int*)hostentry->h_addr_list[0];
        }
        doomcom->numnodes++;
    }

    doomcom->id = DOOMCOM_ID;
    doomcom->numplayers = doomcom->numnodes;

    // build message to receive
    insocket = UDPsocket();
    BindToLocalPort(insocket, htons(DOOMPORT));
#if defined(DOOM_WIN32)
    ioctlsocket(insocket, FIONBIO, &trueval);
#else
    ioctl(insocket, FIONBIO, &trueval);
#endif

    sendsocket = UDPsocket();
#endif
}


void I_NetCmd(void)
{
#if defined(I_NET_ENABLED)
    if (doomcom->command == CMD_SEND)
    {
        netsend();
    }
    else if (doomcom->command == CMD_GET)
    {
        netget();
    }
    else
    {
        //I_Error("Error: Bad net cmd: %i\n", doomcom->command);
        
        doom_strcpy(error_buf, "Error: Bad net cmd: ");
        doom_concat(error_buf, doom_itoa(doomcom->command, 10));
        I_Error(error_buf);
    }
#endif
}
