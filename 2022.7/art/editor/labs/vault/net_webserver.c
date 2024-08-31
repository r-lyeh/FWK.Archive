// webserver, forked from nweb23.c by Nigel Griffiths (public domain).
// - rlyeh, public domain.

#ifndef WEBSERVER_H
#define WEBSERVER_H

int webserver(int port, const char *folder);

#endif

// -----------------------------------------------------------------------------

#ifdef WEBSERVER_C
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>

const char index_html[] =
    // markdeep begin
    "<meta charset='utf-8' emacsmode='-*- markdown -*-'>" \
    "<link rel='stylesheet' href='https://casual-effects.com/markdeep/latest/apidoc.css?'>" \
    "<style>.backtick, .tilde {overflow-x: auto;} .longTOC {overflow-x: hidden;}</style>"
    // content
    "# hello\n"
    " - webserver.c\n"
    // markdeep end
    "<script>markdeepOptions={tocStyle:'long'};</script>" \
    "<!-- Markdeep: --><script src='https://casual-effects.com/markdeep/latest/markdeep.min.js?'></script>";


#define WEB_VERSION    23
#define WEB_BUFSIZE  8096
#define WEB_ERROR      42
#define WEB_LOG        44
#define WEB_FORBIDDEN 403
#define WEB_NOTFOUND  404

#define WEB_GENERIC_MIME "application/octet-stream"

struct {
    char *ext;
    char *filetype;
} extensions [] = {
    {"gif", "image/gif" },
    {"jpg", "image/jpg" },
    {"jpeg", "image/jpeg"},
    {"png", "image/png" },
    {"ico", "image/ico" },
    {"zip", "image/zip" },
    {"gz",  "image/gz"  },
    {"tar", "image/tar" },
    {"htm", "text/html" },
    {"html", "text/html" },
    {0, 0}
};

#define webpanic(...) do { LOG(WEB|SERVER, __VA_ARGS__); exit(-1); } while(0)

#define weblogger(...) do { if(!weblogger(__VA_ARGS__)) goto clean_up; } while(0)
bool (weblogger)(int type, const char *s1, const char *s2, int socket_fd) {
    int fd;
    char logbuffer[WEB_BUFSIZE * 2];
    const char *notfound = "HTTP/1.1 404 Not Found\nContent-Length: 138\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>Not Found</h1>\nThe requested URL was not found on this server.\n</body></html>\n";
    const char *forbidden = "HTTP/1.1 403 Forbidden\nContent-Length: 185\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>403 Forbidden</title>\n</head><body>\n<h1>Forbidden</h1>\nThe requested URL, file type or operation is not allowed on this simple static file webserver.\n</body></html>\n";
    /**/ if( type == WEB_LOG) { sprintf(logbuffer, " INFO: %s:%s:%d", s1, s2, socket_fd); }
    else if( type == WEB_ERROR) { sprintf(logbuffer, "WEB_ERROR: %s:%s Errno=%d exiting pid=%d", s1, s2, errno, getpid()); }
    else if( type == WEB_NOTFOUND) { tcp_send(socket_fd, notfound, 224); sprintf(logbuffer, "NOT FOUND: %s:%s", s1, s2); }
    else if( type == WEB_FORBIDDEN) { tcp_send(socket_fd, forbidden, 271); sprintf(logbuffer, "WEB_FORBIDDEN: %s:%s", s1, s2); }
    /* No checks here, nothing can be done with a failure anyway */
    LOG(WEB|HOST, logbuffer);
    return (type == WEB_ERROR || type == WEB_NOTFOUND || type == WEB_FORBIDDEN) ? false : true;
}

/* this is a child web server process, so we can exit on errors */
static __thread int hit = 0;
static
int nweb23(int fd) {
    int j, buflen;
    long i, ret, len;
    char *fstr;
    static char buffer[WEB_BUFSIZE + 1]; /* static so zero filled */

    ret = tcp_recv(fd, buffer, WEB_BUFSIZE); /* read Web request in one go */
    if(ret == 0 || ret == -1)    /* read failure stop now */ {
        weblogger(WEB_FORBIDDEN, "failed to read browser request", "", fd);
    }
    if(ret > 0 && ret < WEB_BUFSIZE)  /* return code is valid chars */
        buffer[ret] = 0;  /* terminate the buffer */
    else buffer[0] = 0;
    for(i = 0; i < ret; i++) /* remove CF and LF characters */
        if(buffer[i] == '\r' || buffer[i] == '\n')
            buffer[i] = '*';
    weblogger(WEB_LOG, "request", buffer, hit);
    if( strncmp(buffer, "GET ", 4) && strncmp(buffer, "get ", 4) ) {
        weblogger(WEB_FORBIDDEN, "Only simple GET operation supported", buffer, fd);
    }
    for(i = 4; i < WEB_BUFSIZE; i++) /* null terminate after the second space to ignore extra stuff */ {
        if(buffer[i] == ' ')   /* string is "GET URL " +lots of other stuff */ {
            buffer[i] = 0;
            break;
        }
    }
    for(j = 0; j < i - 1; j++) /* check for illegal parent directory use .. */
        if(buffer[j] == '.' && buffer[j + 1] == '.') {
            weblogger(WEB_FORBIDDEN, "Parent directory (..) path names not supported", buffer, fd);
        }
    if( !strncmp(&buffer[0], "GET /\0", 6) || !strncmp(&buffer[0], "get /\0", 6) ) /* convert no filename to index file */
        strcpy(buffer, "GET /index.html");

    /* work out the file type and check we support it */
    buflen = strlen(buffer);
    fstr = (char *)0;
    for(i = 0; extensions[i].ext != 0; i++) {
        len = strlen(extensions[i].ext);
        if( !strncmp(&buffer[buflen - len], extensions[i].ext, len)) {
            fstr = extensions[i].filetype;
            break;
        }
    }
    if(fstr == 0) {
        //weblogger(WEB_FORBIDDEN, "file extension type not supported", buffer, fd);
        fstr = WEB_GENERIC_MIME;
    }

    const char *fname = &buffer[5];

    if( !strcmp(fname, "index.html") ) {
        len = strlen(index_html);

        weblogger(WEB_LOG, "Header", buffer, hit);
        sprintf(buffer, "HTTP/1.1 200 OK\nServer: net.webserver/%d.0\nContent-Length: %ld\nConnection: close\nContent-Type: %s\n\n", WEB_VERSION, len, fstr); /* Header + a blank line */
        tcp_send(fd, buffer, strlen(buffer));

        /* send file in 8KB block - last block may be smaller */
        const char *ptr = index_html, *end = ptr + len;
        while( ptr < end ) {
            tcp_send(fd, ptr, (ptr + WEB_BUFSIZE) >= end ? end - ptr : WEB_BUFSIZE);
            ptr += WEB_BUFSIZE;
        }
        goto clean_up;
    }

    // open the file for reading
    FILE *file_fd = fopen(fname, "rb");
    if( file_fd ) {
        fseek(file_fd, (off_t)0, SEEK_END); /* lseek to the file end to find the length */
        len = (long)ftell(file_fd);
        fseek(file_fd, (off_t)0, SEEK_SET); /* lseek back to the file start ready for reading */

        weblogger(WEB_LOG, "Header", buffer, hit);
        sprintf(buffer, "HTTP/1.1 200 OK\nServer: net.webserver/%d.0\nContent-Length: %ld\nConnection: close\nContent-Type: %s\n\n", WEB_VERSION, len, fstr); /* Header + a blank line */
        tcp_send(fd, buffer, strlen(buffer));

        /* send file in 8KB block - last block may be smaller */
        weblogger(WEB_LOG, "SEND", fname, hit);
        while (  (ret = fread(buffer, 1, WEB_BUFSIZE, file_fd)) > 0 ) {
            tcp_send(fd, buffer, ret);
        }
        fclose(file_fd);
    } else {
        weblogger(WEB_NOTFOUND, "failed to open file", fname, fd);
    }

    clean_up:;

    /* allow socket to drain before signalling the socket is closed */
    sleep_ss(1);
    tcp_close(fd);

    return 0;
}

#undef weblogger


int webserver(int port, const char *path) {
    /* input checks */
    if( port < 1024 || port > 60000) {
        webpanic("!Invalid port number (try 1024<port<60000)");
    }
    if( !strncmp(path, "/"   , 2 ) || !strncmp(path, "/etc", 5 ) ||
        !strncmp(path, "/bin", 5 ) || !strncmp(path, "/lib", 5 ) ||
        !strncmp(path, "/tmp", 5 ) || !strncmp(path, "/usr", 5 ) ||
        !strncmp(path, "/dev", 5 ) || !strncmp(path, "/sbin", 6) ) {
        webpanic("!Blacklisted top directory (use another folder)");
    }
    if( chdir(path) < 0 ) {
        webpanic("!Cannot change to directory");
    }

    weblogger(WEB_LOG, "host started", "", getpid());

    /* setup the network socket */
    tcp_init();
    int listenfd = tcp_bind("0.0.0.0", va("%d",port), 64), socketfd;
    if( listenfd < 0 ) {
        webpanic("!Cannot create listening socket");
    }
    for(hit = 1; ; hit++) {
        if(tcp_peek(listenfd,nweb23)) {}
        sleep_ms(500);
    }

    clean_up:;
    return 0;
}

#ifdef WEBSERVER_DEMO
int main() {
    puts("listening at http://localhost:8000");
    webserver(8000, "./");
}
#endif // WEBSERVER_DEMO
#endif // WEBSERVER_C
