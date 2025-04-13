#define PRINTF_(format, ...) {fprintf(stderr, format, ##__VA_ARGS__);}
#define PRINTF(format, ...) {struct timespec tspr; extern int clock_gettime(clockid_t clk_id, struct timespec *tp); clock_gettime(1, &tspr); fprintf(stderr, "[%ld.%03ld] " format, tspr.tv_sec, tspr.tv_nsec / 1000 / 1000, ##__VA_ARGS__);}

#define VERSIONLINE "2025-01-01"
#define BACKGROUNDCOLOR 0x89A3
#define BACKGROUNDCOLOR2 0x6100
