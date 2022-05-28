// - rlyeh, public domain

bool os_singleton(const char *guid);

#ifdef SINGLETON_C
#pragma once
#include <stdbool.h>

#ifdef _WIN32
    // from microsoft website

    //#include <winsock2.h>
    #include <stdlib.h>
    #include <stdio.h>

    static HANDLE app_mutex  = 0;

    void os_singleton__quit(void) {
        if( app_mutex ) {
            ReleaseMutex( app_mutex );
            app_mutex  = NULL;
        }
    }

    bool os_singleton(const char *guid) {
        //Make sure that you use a name that is unique for this application otherwise
        //two apps may think they are the same and it wont work.
        char buffer[128];
        sprintf(buffer, "Global\\{%s}", guid);
        app_mutex  = CreateMutexA(NULL, FALSE, buffer);
        if(ERROR_ALREADY_EXISTS == GetLastError()) {
            return false;
        }
        atexit(os_singleton__quit); // needed?
        return true;
    }

#else

    bool os_singleton(const char *guid) {
        return true;
    }

#endif

#ifdef SINGLETON_DEMO
#include <stdio.h>
int main() {
    bool ok = os_singleton("619967F0-CDD6-48b5-9C61-DE91175C3112");
    if( ok ) {
        puts("Ok. os_singleton() initialized...");
    } else {
        puts("Error: App already initialized...");
    }
    system("pause");
}
#define main main__
#endif // SINGLETON_DEMO
#endif // SINGLETON_C
