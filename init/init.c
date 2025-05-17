#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/mount.h>
#include <linux/loop.h>

/*
/fs
	upper
	mdc.img
	mdc.new.img
/fs_
	lower
	overlay
	work
*/

//Defines
#define PRINTF(format, ...) {printf(format, ##__VA_ARGS__);}
#define printk PRINTF
#define statfs stat
#define sys_chdir chdir
#define sys_close close
#define sys_ioctl ioctl
#define sys_mkdir mkdir
#define sys_mount mount
#define sys_open open
#define sys_rename rename
#define sys_statfs stat
#define sys_keyctl(a, b, c, d, e) syscall(SYS_keyctl, a, b, c, d, e)
#define sys_add_key(a, b, c, d, e) syscall(SYS_add_key, a, b, c, d, e)

static void get_random_bytes(void *buf, int nbytes) {
	for (int i = 0; i < nbytes; i++)
		*((unsigned char *)buf + i) = rand();
}

//Defines special
#define ROOTALL  PREFIX ""
#define LOWER PREFIX "/fs_/lower"
#define OVERLAY PREFIX "/fs_/overlay"
#define LOOP "/dev/loop0"
#include "../kernel/do_mount_mydonglecloud.h"

//Functions
int main(int argc, char **argv) {
	int ret = do_mount_mydonglecloud(1);
	PRINTF("Ret: %d\n", ret);
	return 0;
}
