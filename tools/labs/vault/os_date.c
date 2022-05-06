// base10 clock format
// -------------------
// Every base10 calendar is a 64-bit number, where:
//
// 18446744073709551615 <-- largest 64-bit number
// *YYYMMDDhhmmssuuuuuu

// - return current date & time in base10. date() = today_gmt() + time_us()
// - return format base10 as string, where "YYYY-MM-DD hh:mm:ss.uuuuuu W" YYY since 2k, u = microseconds, W = day-of-week
// - return adjusted system time (gmt)
// - return unadjusted system time (ust)

uint64_t date();
char *   date_format(uint64_t base10);

uint64_t today_gmt();
uint64_t today_ust();


#ifdef DATE_C
#pragma once
#ifndef _WIN32
#include <sys/time.h>
#endif
#include <time.h>

static uint64_t date_base10(int64_t unixstamp) {
    // Reference: Fliegel, H. F. and van Flandern, T. C. (1968).
    // Communications of the ACM, Vol. 11, No. 10 (October, 1968).
    enum { RTC_EPOCH_JULIAN_DAY = 2440588 }; // January 1st, 1970.
    int64_t yy, mm, dd, l, n;
    l = unixstamp / 86400 + 68569 + RTC_EPOCH_JULIAN_DAY;
    n = 4 * l / 146097;
    l = l - (146097 * n + 3) / 4;
    yy = 4000 * (l + 1) / 1461001;
    l = l - 1461 * yy / 4 + 31;
    mm = 80 * l / 2447;
    dd = l - 2447 * mm / 80;
    l = mm / 11;
    mm = mm + 2 - 12 * l;
    yy = 100 * (n - 49) + yy + l;
    yy -= 2000;

    unixstamp = unixstamp % (24 * 3600);
    int hh = (int)unixstamp / 3600;

    unixstamp = unixstamp % 3600;
    int mn = (int)unixstamp / 60;
    int ss = (int)unixstamp % 60;

#if 0
    int id = (time_us() % 1000000);
#elif 0
    static uint8_t id = 0xFF; ++id;
#else
    const uint8_t id = 0;
#endif

    return
    /// Every base10 calendar is a 64-bit number:
    /// 18446744073709551615
    /// *YYYMMDDhhmmssuuuuuu uuuuuu = microseconds
    yy *   10000000000000000ULL +
    mm *     100000000000000ULL +
    dd *       1000000000000ULL +
    hh *         10000000000ULL +
    mn *           100000000ULL +
    ss *             1000000ULL +
    //                000000ULL;
                             id;
}

uint64_t today_ust() {
    return date_base10( time(0) );
}

uint64_t today_gmt() {
    time_t t = time(0);
    struct tm *gtm = gmtime(&t);
    int hh1 = gtm->tm_hour, mm1 = gtm->tm_min;
    struct tm *ltm = localtime(&t);
    int hh2 = ltm->tm_hour, mm2 = ltm->tm_min;
    int hdiff = hh2-hh1;
    int mdiff = mm2-mm1;
    return date_base10( time(0) + (1) * (hdiff * 3600 + mdiff * 60) );
}

uint64_t date() {
    return today_gmt() + time_us() % 1000000;
}

char *date_format( uint64_t base10 ) {
    int y, m, d, dow;
    int us = base10 % 1000000; base10 /= 1000000;
    int ss = base10 %     100; base10 /= 100;
    int mn = base10 %     100; base10 /= 100;
    int hh = base10 %     100; base10 /= 100; d = base10 % 100;
    int dd =                 d; base10 /= 100; m = base10 % 100;
    int mm =                 m; base10 /= 100; y = base10 % 100;
    int yy =                 y; base10 /= 100;
    if( 1 ) {
        /* Tomohiko Sakamoto's Algorithm */
        /* Get day of the week [0=Sun..6=Sat]*/
        int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
        dow = (y -= m < 3, (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7);
    }
    // return va("%04d-%02d-%02d %02d:%02d:%02d.%06d %d", 2000 + yy, mm, dd, hh, mn, ss, us, dow);
    static __thread char buf[320];
    static __thread int slot = 0; slot = (slot + 1) % 10;
    snprintf(buf + slot*32, 32, "%04d-%02d-%02d %02d:%02d:%02d.%06d %d", 2000 + yy, mm, dd, hh, mn, ss, us, dow);
    return buf + slot*32;
}

#ifdef DATE_DEMO
#include <stdio.h>
int main() {
    uint64_t gmt = today_gmt(); printf("gmt %019llu -> %s\n", gmt, date_format(gmt) );
    uint64_t ust = today_ust(); printf("ust %019llu -> %s\n", ust, date_format(ust) );
    // printf("%+lld adjust time\n", (gmt - ust) / 100000000);
    while(1) {
        printf("\rnow %019llu -> %s", date(), date_format(date()) );
    }
}
#define main main__
#endif // DATE_DEMO
#endif // DATE_C
