#define PRINTF_(format, ...) {fprintf(stderr, format, ##__VA_ARGS__);}
#define PRINTF(format, ...) {struct timespec tspr; extern int clock_gettime(clockid_t clk_id, struct timespec *tp); clock_gettime(1, &tspr); fprintf(stderr, "[%ld.%03ld] " format, tspr.tv_sec, tspr.tv_nsec / 1000 / 1000, ##__VA_ARGS__);}

#define WIDTH 128
#define HEIGHT 128
#define DEPTH 3
#define VERSION "2025-01-01"
