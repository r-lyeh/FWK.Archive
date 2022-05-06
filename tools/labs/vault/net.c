// - rlyeh, public domain

#ifdef NET_C
//#pragma once
#define ECDH_C
#define TCP_C
#define FRAGMENT_C
#define TUNNEL_C
#define WEBSERVER_C
#endif

#include "os.c"
#include "ds.c"
#include "net_tcp.c"
#include "net_ecdh.h"
#include "net_fragment.c"
#include "net_tunnel.c"
#include "net_webserver.c"
