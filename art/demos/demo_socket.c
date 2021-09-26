// network demo
// - rlyeh, public domain.

#include "fwk.h"

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
        int srv = tcp_bind("0.0.0.0", "8080", 10); // port "0" for auto
        if( srv >= 0 ) {
            puts("ready at 8080");
            for(;;) {
                tcp_peek(srv, echo_server);
                sleep_ms(100);
            }
            tcp_close(srv);
        }
    }
    else if( argc > 1 && 0 == strcmp(argv[1], "--client") ) {
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
    else {
        printf("%s --server\n", argv[0]);
        printf("%s --client\n", argv[0]);
    }
}
