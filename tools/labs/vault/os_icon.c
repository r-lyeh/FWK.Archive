// change window icon.
// - rlyeh, public domain.

void os_icon(const char *pathfile_ico);  // "" to restore (win32 only)

// ----------------------------------------------------------------------------

#ifdef ICON_C
#pragma once

#ifdef _WIN32
static HICON os_load_ico( const char *ico_pathfile ) {
    HICON hi = (HICON)LoadImageA( // returns a HANDLE so we have to cast to HICON
        NULL,             // hInstance must be NULL when loading from a file
        ico_pathfile,     // .ico file name
        IMAGE_ICON,       // specifies that the file is an icon
        0,                // width of the image (we'll specify default later on)
        0,                // height of the image
        LR_LOADFROMFILE|  // we want to load a file (as opposed to a resource)
        LR_DEFAULTSIZE|   // default metrics based on the type (IMAGE_ICON, 32x32)
        LR_SHARED         // let the system release the handle when it's no longer used
    );
    return hi;
}
void os_icon(const char *pathfile_ico) {
    // set app .ico after window is created
    HANDLE hIcon = os_load_ico(pathfile_ico);
    HWND hWndPseudo = (HWND)GetCurrentProcess();
    HWND hWndWindow = (HWND)GetActiveWindow();
    HWND hWndConsole = (HWND)GetConsoleWindow();
    HWND hWnds[] = { hWndWindow, hWndConsole, hWndPseudo };
    for( int i = 0; i < (sizeof(hWnds)/sizeof(0[hWnds])); ++i) {
        SendMessage( hWnds[i], (UINT)WM_SETICON, ICON_BIG,   (LPARAM)hIcon );
        SendMessage( hWnds[i], (UINT)WM_SETICON, ICON_SMALL, (LPARAM)hIcon );
    }
}
#else
void os_icon(const char *) {
    // @todo : Use https://wiki.libsdl.org/SDL_SetWindowIcon instead
}
#endif

#ifdef ICON_DEMO
int main() {
    os_icon("demo.ico");
    window_title("demo.ico");
    system("pause");
    os_icon("");
}
#define main main__
#endif // ICON_DEMO
#endif // ICON_C
