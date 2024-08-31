// [x] logger: info, warn, fail, quit. also todo() and fixme()
// - rlyeh, public domain

// -----------------------------------------------------------------------------

#ifndef LOGGER_H
#define LOGGER_H

#ifndef LOGLEVEL
#   define LOGLEVEL 4 // 0(off).. 4(max)
#endif

#ifdef SHIPPING
#   undef  LOGLEVEL
#   define LOGLEVEL 0
#endif

#if LOGLEVEL <= 0
#   define LOG(tags, ...) (void)0
#else
#   ifdef _MSC_VER
#   define SHORT_FILE strrchr("\\" __FILE__, '\\') // todo: test with +1
#   else
#   define SHORT_FILE __FILE__
#   endif
#   define LOG(tags, ...) (fprintf(stderr,__VA_ARGS__),fprintf(stderr," (%s:%d) %s\n",SHORT_FILE,__LINE__,#tags))
#endif

#if LOGLEVEL >= 1
#   define LOGQUIT(tags, ...)  do { LOG(tags, __VA_ARGS__); exit(-__LINE__); } while(0)
#else
#   define LOGQUIT(tags, ...)  exit(-__LINE__)
#endif

#if LOGLEVEL >= 2
#   define LOGFAIL LOG
#else
#   define LOGFAIL(tags, ...)  (void)0
#endif

#if LOGLEVEL >= 3
#   define LOGWARN LOG
#else
#   define LOGWARN(tags, ...)  (void)0
#endif

#if LOGLEVEL >= 4
#   define LOGINFO LOG
#else
#   define LOGINFO(tags, ...)  (void)0
#endif

// additional log utils
#define TODO(...)  do { static int todo  = 0; if(!todo ) { ++todo ; LOGINFO(TODO, __VA_ARGS__); } } while(0)
#define FIXME(...) do { static int fixme = 0; if(!fixme) { ++fixme; LOGINFO(FIXME, __VA_ARGS__); } } while(0)

#endif // LOGGER_H
