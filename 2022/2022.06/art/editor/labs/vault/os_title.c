// - rlyeh, public domain

void os_title(const char *name);

#ifdef TITLE_C
#pragma once

void os_title(const char *name) {
#ifdef _WIN32
    HWND hWndPseudo = (HWND)GetCurrentProcess();
    HWND hWndWindow = (HWND)GetActiveWindow();
    HWND hWndConsole = (HWND)GetConsoleWindow();
    HWND hWnds[] = { hWndWindow, hWndConsole, hWndPseudo };
    for( int i = 0; i < sizeof(hWnds)/sizeof(hWnds[0]); ++i) {
        SetWindowTextA(hWnds[i], name);
    }
#endif
}

#endif
