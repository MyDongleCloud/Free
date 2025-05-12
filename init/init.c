#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/mount.h>
#include <linux/loop.h>

#define PRINTF(format, ...) {printf(format, ##__VA_ARGS__);}

//Functions
int main(int argc, char **argv) {
	PRINTF("Start\n");

/*
/fs
	img
	img.new
	lower
	overlay
	upper
	work
/sbin/init
/persistent
*/
	int ret, ffd, dfd;
	int reset = 0;
	struct stat buf;
	struct loop_info64 loopinfo;

	ret = stat("/fs/img.new", &buf);
	if (ret == 0) {
		reset = 1;
		rename("/fs/img.new", "/fs/img");
		PRINTF("Mount Warning: Updating img\n");
	}

	/* 0. Do we have reset? */
	ret = stat("/fs/upper/reset", &buf);
	if (ret == 0)
		reset = 1;
	if (reset) {
		char sz[128];
		unsigned int i;
		PRINTF("Mount Warning: Doing Reset\n");
		rename("/fs/upper", "/fs/trash");
		sprintf(sz, "/fs/upper/trash.upper.%u", rand());
		rename("/fs/trash", sz);
		sprintf(sz, "/fs/upper/trash.work.%u", rand());
		rename("/fs/work", sz);
	}


	/* Open the squashfs */
	ffd = open("/fs/img", O_RDONLY, 0);
	if (ffd < 0) {
		PRINTF("Mount Error: failed to open img %d\n", ffd);
		//return -1;
	}

	/* Bind the squashfs */
	dfd = open("/dev/loop0", O_RDWR, 0);
	if (dfd < 0) {
		PRINTF("Mount Error: failed to open loop %d\n", dfd);
		//return -2;
	}
	ret = ioctl(dfd, LOOP_SET_FD, ffd);
	if (ret) {
		PRINTF("Mount Error: failed ioctl1 %d\n", ret);
		//return -3;
	}

	/* Bind "/fs/img" to the loop */
	memset(&loopinfo, 0, sizeof(loopinfo));
	loopinfo.lo_offset = 0;
	ret = ioctl(dfd, LOOP_SET_STATUS64, (long unsigned int)&loopinfo);
	if (ret) {
		PRINTF("Mount Error: failed ioctl2 %d\n", ret);
		//return -4;
	}

	/* Close descriptors */
	close(dfd);
	close(ffd);

	/* Mount the squashfs */
	ret = mount("/dev/loop0", "/fs/lower", "squashfs", MS_RDONLY, NULL);
	if(ret) {
		PRINTF("Mount Error: failed to mount lower %d\n", ret);
		//return -5;
	}

	/* Final mount */
	ret = mount("none", "/fs/overlay", "overlay", 0, "lowerdir=/fs/lower,upperdir=/fs/upper,workdir=/fs/work");
	if(ret) {
		PRINTF("Mount Error: failed to mount overlayfs %d\n", ret);
		//return -6;
	}

	PRINTF("Mount: Success\n");
	ret = chdir("/fs/overlay");
	if(ret) {
		PRINTF("Mount Error: failed to chdir %d\n", ret);
		//return -7;
	}

	/* Pivot */
	ret = syscall(SYS_pivot_root, "/", "/fs/overlay");
	if(ret) {
		PRINTF("Mount Error: failed to pivot_root %d\n", ret);
		return -8;
	}

	/* Fork */
	if (fork() == 0) {
		status = system("/sbin/init");
		exit(0);
	}

	PRINTF("End\n");
	return 0;
}
