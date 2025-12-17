#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/inotify.h>
#include "macro.h"

//Private variables
static int fdNotify;
static int wd;

//Functions
void diskNotifyCB(int fdNotify, void (*cb)()) {
#define EVENT_SIZE (sizeof (struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))
	char buffer[EVENT_BUF_LEN];
	int length = read(fdNotify, buffer, EVENT_BUF_LEN);
	int i = 0;
	while (i < length) {
		struct inotify_event *event = (struct inotify_event *) &buffer[i];
		 if (event->mask)
			cb();
		i += EVENT_SIZE + event->len;
	}
}

int diskNotifyStart(char *sz) {
	int fdNotify = inotify_init1(IN_NONBLOCK);
	int wd = inotify_add_watch(fdNotify, sz, IN_CLOSE_WRITE);
	return fdNotify;
}

int diskNotifyStop() {
	inotify_rm_watch(fdNotify, wd);
	close(fdNotify);
}
