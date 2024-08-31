#include "fwk.h"
#include "oscedit.h"

// demo

int main() {
    do_once map_init(client_vars, less_str, hash_str);
    do_once map_init(server_vars, less_str, hash_str);

    int server_socket = osc_listen("0.0.0.0", OSC_EDIT_PORT);
    if( server_socket >= 0 ) {        
        if( !strcmp("--launch", argv(1)) ) system("oscgame"); // launch game

        window_create(0.80, WINDOW_PORTRAIT );
        window_title("EDITOR");
        while( window_swap() ) {
            osc_edit_sync(server_socket, -1, 4);
        }
    }
}
