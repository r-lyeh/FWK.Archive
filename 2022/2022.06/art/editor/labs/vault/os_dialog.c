// # dialog ###################################################################

int os_dialog( int buttons, const char *title, const char *message );

#ifdef DIALOG_C
#pragma once

#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef _WIN32
#include <winsock2.h>
#ifdef __TINYC__
    #define CP_UTF8 65001
    int WINAPI MultiByteToWideChar();
    int WINAPI WideCharToMultiByte();
#else
    #include <shlobj.h>
#endif
wchar_t *dialog_widen(wchar_t *buf, int sz, const char *utf8) { // wide strings (windows only)
    int needed = 2 * MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    buf[0] = 0;
    if( needed < sz ) {
        MultiByteToWideChar(CP_UTF8, 0, utf8, -1, (void*)buf, needed);
    }
    return (wchar_t *)buf;
}
#endif

int os_dialog( int buttons, const char *title, const char *message ) {
    title += title[0] == '!';     // advance text if starts with '!' (with callstack option)
    message += message[0] == '!'; // advance text if starts with '!' (with callstack option)

    char lower[256] = {0};
    for( int i = 0; title[i]; ++i) lower[i] = title[i] | 32;

    int is_fail = !!strstr(lower, "fail") || !!strstr(lower, "error") || !!strstr(lower, "crash");
    int is_help = !!strstr(lower, "help") || !!strstr(lower, "about") || !!strstr(lower, "info");
    int is_warn = !!strstr(lower, "warning");

#if 0
    if( is_fail || is_warn ) {
        if(window) glfwSetClipboardString(window, message);
    }
#endif

#ifdef _WIN32
    #pragma comment(lib, "user32")
    wchar_t buf1[256], buf2[2048];
    wchar_t *title16 = dialog_widen(buf1, 256, title), *message16 = dialog_widen(buf2, 2048, message);
    int rc = MessageBoxW(0, message16, title16, MB_SETFOREGROUND |
        (buttons >= 3 ? MB_YESNOCANCEL : buttons >= 2 ? MB_YESNO : MB_OK) |
        (is_fail ? MB_ICONERROR :
        (is_warn ? MB_ICONWARNING :
        (is_help ? MB_ICONQUESTION : 0)))
    );
    /**/ if( rc == IDYES || rc == IDOK ) return 1;
    else if( rc == IDNO ) return 0;
    return 2;
#else
    return 0;
#endif
}

#ifdef DIALOG_DEMO
#include <assert.h>
#include <stdio.h>
int main() {
    int unicode = os_dialog(0, "unicode test info dialog", "私 は ガ");

    int pressed1 = os_dialog(3, "3-buttons general dialog", "Bla.\nContinue?");
    int pressed2 = os_dialog(2, "2-buttons warning dialog", "Bla.\nContinue?");
    int pressed3 = os_dialog(1, "1-button error dialog", "Bla.\nBla.");
    int pressed4 = os_dialog(1, "1-button about dialog", "Bla.\nBla.");

    const char *buttons[] = { "no", "yes/ok", "cancel/error" };
    printf("User pressed '%s'\n", buttons[ pressed1 ] );
    printf("User pressed '%s'\n", buttons[ pressed2 ] );
    printf("User pressed '%s'\n", buttons[ pressed3 ] );
    printf("User pressed '%s'\n", buttons[ pressed4 ] );

    assert( os_dialog(2, "Warning!", "This is a test.\nIs this dialog visible?" ) );
    assert(~puts("Ok") );
}
#define main main__
#endif // DIALOG_DEMO
#endif // DIALOG_C
