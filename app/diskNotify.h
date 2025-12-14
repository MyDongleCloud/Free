#ifndef DISKNOTIFY_H
#define DISKNOTIFY_H

//Global functions
void diskNotifyCB(int fdNotify, void (*cb)(char *path));
int diskNotifyStart(char *sz);
int diskNotifyStop();

#endif
