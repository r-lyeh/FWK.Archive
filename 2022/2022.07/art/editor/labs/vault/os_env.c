#ifndef ENV_H
#define ENV_H

// environment folders

char*    env_user(); // user name
char*    env_home(); // user home location
char*    env_curr(); // current directory
char*    env_game(); // binary location
char*    env_save(); // save data location
char*    env_temp(); // temp data location
uint64_t env_free(); // available free space

// environment vars

const char *env_get( const char *key );
const char *env_set( const char *key, const char *value );

#endif

// # env ######################################################################
// @ todo: paths must end with slash always

#ifdef ENV_C
#pragma once
#include <stdint.h>
#include <stdlib.h>
#ifndef _WIN32
#include <pwd.h>
#endif

#ifndef PATH_MAX
#ifdef MAX_PATH
#define PATH_MAX MAX_PATH
#elif defined _MAX_PATH
#define PATH_MAX _MAX_PATH
#else
#define PATH_MAX 260
#endif
#endif

static char *env_get_process_full_path_() {
    static char *t = 0;
    if(t) return t;

    char path[PATH_MAX+1] = {0};

#ifdef __APPLE__ // _IOS || _OSX
    uint32_t i = sizeof(path);
    if (_NSGetExecutablePath(path, &i) > -1) {
        return t = STRDUP(path);
    }
#elif defined __linux__ // _LIN
    //if (readlink(strf("/proc/%d/exe", getpid()), path, sizeof(path)) > -1) {
    if (readlink("/proc/self/exe", path, sizeof(path)) > -1) {
        return t = STRDUP(path);
    }
#elif defined _WIN32
    if (GetModuleFileNameA(0, path, sizeof(path))) {
        return t = STRDUP(path);
    }
#endif
    //t = STRDUP( __argv[0] );
    //return t = dirabs( &t );
    return "./";
}

// util that ensure paths end with '/' and are normalized as well.
static char *env_fix_(const char *pathfile ) { // must free() after use
    char *buf = (char*)MALLOC( PATH_MAX + 1 );
    strcpy(buf, pathfile);
    int len = strlen(buf);
    for( int i = 0; i < len; ++i ) if( buf[i] == '\\' ) buf[i] = '/';
    if( buf[ len ] != '/' ) {
        buf[ len ] = '/';
    }
    return buf;
}

char *env_user() {
    static char *t = 0;
    if(t) return t;
    t = t ? t : getenv("USER");
    t = t ? t : getenv("USERNAME");
    t = STRDUP( t ? t : "GUEST" );
    return t;
}
char *env_home() {
    static char *t = 0;
    if(t) return t;
    t = t ? t : getenv("USERPROFILE");
    t = t ? t : getenv("HOME");
    t = env_fix_( t ? t : "./" );
    return t;
}
char *env_save() { // envdata
    static char *t = 0;
    if(t) return t;
    t = t ? t : getenv("APPDATA");
    t = t ? t : getenv("XDG_DATA_HOME");
    t = env_fix_( t ? t : "./" );
    return t;
}
char *env_temp() {
    static char *t = 0;
    if(t) return t;
    t = t ? t : getenv("TMPDIR");
    t = t ? t : getenv("TMP");
    t = t ? t : getenv("TEMP");
#ifdef _WIN32
    // GetTempPathW(n, buf);
#else
    t = t ? t : "/tmp";
#endif
    t = env_fix_( t ? t : "./" );
    return t;
}
static __thread char cwd[PATH_MAX+1];
char *env_curr() { // envwork
#ifdef _WIN32
    _getcwd(cwd, sizeof(cwd));
#else
    getcwd(cwd, sizeof(cwd));
#endif
    return env_fix_( cwd );
}
uint64_t env_free() {
#ifdef _WIN32
    DWORD SectorsPerCluster, BytesPerSector, NumberOfFreeClusters, TotalNumberOfClusters;
    if( GetDiskFreeSpaceA( ".\\", &SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters ) ) {
        return ((uint64_t)NumberOfFreeClusters) * SectorsPerCluster * BytesPerSector;
    }
#endif
    return UINT64_C(-1); // ~0LLU;
}

char *env_game() { // envexec, envgame
    static char *t = 0;
    if( t ) return t;

    t = STRDUP( env_get_process_full_path_() );
    for( size_t i = strlen(t); i-- > 0; ) {
        if (t[i] == '\\' || t[i] == '/') {
            t[i] = '\0';
            break;
        }
    }
    return env_fix_(t);
}

// -----------------------------------------------------------------------------

const char *env_set( const char *key, const char *value ) { //$
#ifdef _WIN32
    char buf[1024];
    sprintf(buf,"%s=%s", key, value ? value : "");
    putenv( buf );
#else
    setenv( key, value ? value : "", 1 );
#endif
    return value;
}
const char *env_get( const char *key ) { //$
    return getenv(key);
}

#ifdef ENV_DEMO
#include <stdio.h>
int main() {
    printf("env_curr: %s\n", env_curr() );
    printf("env_home: %s\n", env_home() );
    printf("env_game: %s\n", env_game() );
    printf("env_save: %s\n", env_save() );
    printf("env_temp: %s\n", env_temp() );
    printf("env_user: %s\n", env_user() );

    double GiB = 1024 * 1024 * 1024;
    printf("env_free: %.3f GiB\n", env_free() / GiB );

    env_set("HELLO", "WORLD");
    assert( 0 == strcmp(env_get("HELLO"), "WORLD"));
    assert(~puts("Ok"));
}
#define main main__
#endif // ENV_DEMO
#endif // ENV_C
