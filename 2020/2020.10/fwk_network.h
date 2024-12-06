// network framework
// - rlyeh, public domain

int download(const char *url, FILE *out);

#ifdef NETWORK_C
#pragma once

#if defined(_WIN32)
#   include <winsock2.h>
#   include <wininet.h>
#   pragma comment(lib,"wininet.lib")
#endif

#ifndef _WIN32

int download(const char *url, FILE *out) {
    return 0;
}

#else

int download(const char *url, FILE *out) {
    int ok = 0;
    DWORD lBytesRead = 0;

    HINTERNET request = 0, session = InternetOpenA("request 1", PRE_CONFIG_INTERNET_ACCESS, NULL, INTERNET_INVALID_PORT_NUMBER, 0);
    if( session ) {
        request = InternetOpenUrlA(session, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if( request ) {
            enum { buflen = 4096 };
            char buffer[ buflen ];
            for(; (ok = (InternetReadFile(request, buffer, buflen, &lBytesRead) != FALSE) && lBytesRead > 0); ) {
                ok = 1 == fwrite(buffer, lBytesRead, 1, out);
                if( !ok ) break;
            }
        }
    }

    if( request ) InternetCloseHandle(request);
    if( session ) InternetCloseHandle(session);
    return ok && !lBytesRead;
}

#endif // _WIN32
#endif // DOWNLOAD_C
