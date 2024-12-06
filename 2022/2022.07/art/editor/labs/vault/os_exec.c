// cmd system invokation
// - rlyeh, public domain.

char *os_exec( const char *cmd );

#ifdef EXEC_C
#pragma once
#ifdef _WIN32
#define popen  _popen
#define pclose _pclose
#endif
char *os_exec( const char *cmd ) {
    static __thread char buf[4096]; buf[0] = 0;
    FILE *fp = popen( cmd, "rt" );
    if( fp ) {
        while( fgets(buf, 4096 - 1, fp) ) {
            char *r = strrchr(buf, '\r');
            char *n = strrchr(buf, '\n');
            if( r ) *r = 0;
            if( n ) *n = 0;
            //puts(buf);
        }
        pclose(fp);
    }
    return buf;
}
#endif
