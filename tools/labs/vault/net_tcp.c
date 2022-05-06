// - rlyeh, public domain

// client
int   tcp_open(const char *address, const char *port);

// server
int   tcp_bind(const char *interface_, const char *port, int queue);
int   tcp_peek(int, int(*callback)(int));

// common
int   tcp_send(int, const void* buf, int len);
int   tcp_recv(int, void* buf, int len);
char* tcp_host(int); // debug
char* tcp_port(int); // debug

int   tcp_close(int);

// 
int   tcp_debug(int);
//int   tcp_logger(int, const char *fmt, ...); // printf debugging data in remote
//int   tcp_crypt(int,uint64_t);               // set shared secret


#ifdef TCP_C
#pragma once

#define SWRAP_IMPLEMENTATION
#include "net_socket.h"
#pragma comment(lib, "ws2_32")

set(int) tcp_set;

void tcp_init(void) {
    static int once = 0; if( !once ) { 
        swrapInit(); /*atexit(swrapTerminate);*/
        set_init(tcp_set, sort_int, hash_int);
        once = 1; 
    }
}
int tcp_open(const char *address, const char *port) {
    tcp_init();
    int fd = swrapSocket(SWRAP_TCP, SWRAP_CONNECT, 0, address, port);
    return fd;
}
int tcp_bind(const char *interface_, const char *port, int backlog) {
    tcp_init();
    int fd = swrapSocket(SWRAP_TCP, SWRAP_BIND, 0/*|SWRAP_NOBLOCK*/, interface_, port);
    if( fd >= 0 ) swrapListen(fd, backlog);
    return fd;
}
int tcp_peek(int fd, int(*callback)(int)) {
    struct swrap_addr sa;
    int fd2 = swrapAccept(fd, &sa);
    if( fd2 >= 0 ) return callback(fd2);
    return 0;
}
int tcp_send(int fd, const void *buf, int len) {
    int rc = swrapSend(fd, (const char *)buf, len);
    if( set_find(tcp_set, fd) ) {
        printf("send -> %11d (status: %d) %s:%s\n", len, rc, tcp_host(fd), tcp_port(fd));
        if( rc > 0 ) hexdump_compact(buf, rc);
    }
    return rc;
}
int tcp_recv(int fd, void *buf, int len) {
    int rc = swrapReceive(fd, (char*)buf, len);
    if( rc != 0 && set_find(tcp_set, fd) ) {
        printf("recv <- %11d (status: %d) %s:%s\n", len, rc, tcp_host(fd), tcp_port(fd));
        if( rc > 0 ) hexdump_compact(buf, rc);
    }
    return rc;
}
char* tcp_host(int fd) {
    char buf[1024];
    struct swrap_addr sa;
    swrapAddress(fd, &sa);
    swrapAddressInfo(&sa, buf, 512, buf+512, 512);
    return va("%s", buf);
}
char* tcp_port(int fd) {
    char buf[1024];
    struct swrap_addr sa;
    swrapAddress(fd, &sa);
    swrapAddressInfo(&sa, buf, 512, buf+512, 512);
    return va("%s", buf+512);
}
int tcp_close(int fd) {
    swrapClose(fd);
    return 0;
}
int tcp_debug(int fd) {
    if( set_find(tcp_set, fd) ) {
        set_erase(tcp_set, fd);
        return 0;
    } else {
        set_insert(tcp_set, fd);
        return 1;
    }
}

#ifdef TCP_DEMO
int echo_server(int cli) {
    printf("srv: client connected from %s:%s\n", tcp_host(cli), tcp_port(cli));
    char buf[128];
    int len = tcp_recv(cli, buf, 128);
    if( len > 0 ) tcp_send(cli, buf, len); // echo
    tcp_close(cli);
    return 1;
}
int main(int argc, char **argv) {
    if( argc > 1 && 0 == strcmp(argv[1], "--server") ) {
        int srv = tcp_bind("0.0.0.0", "8080", 10);
        if( srv >= 0 ) {
            puts("ready at 8080");
            for(;;) {
                tcp_peek(srv, echo_server);
                Sleep(1);
            }
            tcp_close(srv);
        }
    } else {
        int cli = tcp_open("127.0.0.1", "8080");
        if( cli >= 0 ) {
            printf("cli: connected to server. my addr is %s:%s\n", tcp_host(cli), tcp_port(cli));
            tcp_send(cli, "Hi", 2);
            char buf[128];
            int rlen = tcp_recv(cli, buf, 128);
            if(rlen > 0) printf("cli: received '%.*s' from server\n", rlen, buf);
            tcp_close(cli);
        }
    }
}
#define main main__
#endif // TCP_DEMO
#endif // TCP_C
