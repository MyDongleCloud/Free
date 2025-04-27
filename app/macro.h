#ifndef MACRO_H
#define MACRO_H

#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, 5,4,3,2,1)
#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5,N,...) N
#define macro_dispatcher(func, ...) macro_dispatcher_(func, VA_NUM_ARGS(__VA_ARGS__))
#define macro_dispatcher_(func, nargs) macro_dispatcher__(func, nargs)
#define macro_dispatcher__(func, nargs) func ## nargs
#undef MAX
#define MAX(...) macro_dispatcher(MAX,__VA_ARGS__)(__VA_ARGS__)
#define MAX1(a) a
#define MAX2(a,b) ((a)>(b)?(a):(b))
#define MAX3(a,b,c) MAX2(MAX2(a,b),c)
#define MAX4(a,b,c,d) MAX2(MAX3(a,b,c),d)
#define MAX5(a,b,c,d,e) MAX2(MAX4(a,b,c,d),e)
#define MIN2(a,b) ((a)>(b)?(b):(a))
#define RANGE(a,b,c) MIN2(MAX2(a,b),c)

#ifdef DESKTOP
#define MAIN_PATH "./"
#else
#define MAIN_PATH "/home/mdc/app/"
#endif

#define FIRMWARE_PATH "/tmp/firmware.zip"

#define PLATFORM_PATH "/dev/mydonglecloud_platform/%s"
#define SCREEN_PATH "/dev/mydonglecloud_screen/%s"
#define SCREEN_FILE "/dev/mydonglecloud_screen_f"
#define TEMPERATURE_PATH "/sys/class/thermal/thermal_zone0/%s"
#define OATH_PATH "/etc/users.oath"
#define SPACESNAME_PATH "/etc/mydonglecloud-spaces.txt"

#define COLOR_BACKGROUND 0x0054e9
#define COLOR_TEXT 0xffffff

#define WIDTH 128
#define HEIGHT 128
#define DEPTH 3
#define VERSION "2025-01-01"

#define UUID_VERSION 0xfff1

#if 1
#define PRINTF_(format, ...) {fprintf(stderr, format, ##__VA_ARGS__);}
#define PRINTF(format, ...) {struct timespec tspr; extern int clock_gettime(clockid_t clk_id, struct timespec *tp); clock_gettime(1, &tspr); fprintf(stderr, "[%ld.%03ld] " format, tspr.tv_sec, tspr.tv_nsec / 1000 / 1000, ##__VA_ARGS__);}
#else
#define PRINTF(format, ...) {fprintf(stderr, format, ##__VA_ARGS__);}
#endif

static inline float deltaTime(struct timespec a, struct timespec b) {
	return (a.tv_nsec - b.tv_nsec) / 1000.0 / 1000.0 + ((a.tv_sec - b.tv_sec) * 1000.0);
}

#endif
