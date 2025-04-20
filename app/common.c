#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <zlib.h>
#include <termios.h>
#include <dirent.h>
#include <signal.h>
#include <pthread.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <linux/wireless.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <openssl/md5.h>
#include "macro.h"

//Global variables
char szSerial[32];

//Private variables
static int termioUsed = 0;
static struct termio termioOriginal;
static pid_t pidLog = 0;
static int doLog = 1;
static int fillSD = 0;
static int fillInternal = 0;

//Functions
void readString(const char *path, const char *key, char *buf, int size) {
	memset(buf, 0, size);
	char fullpath[256];
	sprintf(fullpath, path, key);
	int fd = open(fullpath, O_RDONLY);
	if (fd >= 0) {
		read(fd, buf, size);
		close(fd);
	}
}

int readValue(const char *path, const char *key) {
	char fullpath[256];
	sprintf(fullpath, path, key);
	int fd = open(fullpath, O_RDONLY);
	int v = -1;
	char buf[16];
	char *p;
	if (fd >= 0) {
		int ret = read(fd, buf, 16);
		if (ret > 0) {
			buf[ret] = '\0';
			v = strtol(buf, &p, 10);
		}
		close(fd);
	}
	return v;
}

void readValues2(const char *path, const char *key, int *i, int *j) {
	char fullpath[256];
	sprintf(fullpath, path, key);
	int fd = open(fullpath, O_RDONLY);
	char buf[16];
	char *p;
	*i = -2;
	*j = -2;
	if (fd >= 0) {
		int ret = read(fd, buf, 16);
		if (ret > 0) {
			buf[ret] = '\0';
			sscanf(buf, "%d %d", i, j);
		}
		close(fd);
	}
}

void readValues4(const char *path, const char *key, int *i, int *j, int *k, int *l) {
	char fullpath[256];
	sprintf(fullpath, path, key);
	int fd = open(fullpath, O_RDONLY);
	char buf[32];
	char *p;
	*i = -1;
	*j = -1;
	*k = -1;
	*l = -1;
	if (fd >= 0) {
		int ret = read(fd, buf, 32);
		if (ret > 0) {
			buf[ret] = '\0';
			sscanf(buf, "%d %d %d %d", i, j, k, l);
		}
		close(fd);
	}
}

void writeString(const char *path, const char *key, char *buf, int size) {
	char fullpath[256];
	sprintf(fullpath, path, key);
	int fd = open(fullpath, O_WRONLY | O_CREAT, 0644);
	if (fd >= 0) {
		write(fd, buf, size);
		close(fd);
	}
}

void writeValue(const char *path, const char *v) {
	int fd = open(path, O_WRONLY | O_CREAT, 0644);
	if (fd >= 0) {
		write(fd, v, strlen(v));
		close(fd);
	}
}

void writeValueKey(const char *path, const char *key, const char *v) {
	char fullpath[256];
	sprintf(fullpath, path, key);
	writeValue(fullpath, v);
}

void writeValueInt(const char *path, int i) {
	char sz[8];
	sprintf(sz, "%d", i);
	writeValue(path, sz);
}

void writeValueInts(const char *path, int i, int j) {
	char sz[16];
	sprintf(sz, "%d %d", i, j);
	writeValue(path, sz);
}

void writeValueKeyInt(const char *path, const char *key, int i) {
	char fullpath[256];
	sprintf(fullpath, path, key);
	writeValueInt(fullpath, i);
}

void writeValueKeyInts(const char *path, const char *key, int i, int j) {
	char fullpath[256];
	sprintf(fullpath, path, key);
	writeValueInts(fullpath, i, j);
}

void writeValueKeyPrintf(const char *path, const char *key, const char *fmt, ...) {
	char fullpath[256];
	sprintf(fullpath, path, key);
    va_list args;
    char *sz = NULL;
    va_start(args, fmt);
    vasprintf(&sz, fmt, args);
    va_end (args);
	writeValue(fullpath, sz);
	if (sz)
		free(sz);
}

int readTemperature() {
#ifdef DESKTOP
	return 65000;
#else
	return readValue(TEMPERATURE_PATH, "temp");
#endif
}

void enterInputMode() {
	termioUsed = 1;
	int fd = fileno(stdin);
	struct termio zap;
	ioctl(fd, TCGETA, &termioOriginal);
	zap = termioOriginal;
	zap.c_cc[VMIN] = 0;
	zap.c_cc[VTIME] = 0;
	zap.c_lflag = 0;
	ioctl(fd, TCSETA, &zap);
}

void leaveInputMode() {
	if (termioUsed) {
		int fd = fileno(stdin);
		ioctl(fd, TCSETA, &termioOriginal);
		termioUsed = 0;
	}
}

void copyFile(char *from, char *to, void (*progresscallback)(int add)) {
	FILE *pf = fopen(from, "rb");
	if (pf) {
		FILE *pt = fopen(to, "wb");
		if (pt) {
			unsigned char buffer[2048];
			size_t ret;
			while ((ret = fread(buffer, 1, 2048, pf)) != 0) {
				fwrite(buffer, 1, ret, pt);
				if (progresscallback)
					progresscallback(ret);
			}
			fflush(pt);
			fclose(pt);
		}
		fclose(pf);
	}
}

static unsigned int rand_() {
	struct timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);
	unsigned int r = spec.tv_nsec;
	int i;
	for (i = 0; i < 100; i++)
		r = ((r * 7621) + 1) % 32768;
	if (r > 32768)
		r = 1;
	return r;
}

void generateUniqueId(char sz[17]) {
	time_t now = time(0);
	sz[0] = '\0';
	char ss[3];
	int i;
	for (i = 0; i < 8; i++) {
		sprintf(ss, "%02x", rand_() % 256);
		strcat(sz, ss);
	}
}

void getSerialID() {
#ifdef DESKTOP
	strcpy(szSerial, "12345678");
#else
#ifdef DIRECT
#define ADDRESS_ID 0xC0067000
	int fdMem = open("/dev/mem", O_RDWR | O_SYNC);
	void *mapdieid, *virtdieid;
	mapdieid = mmap(0, MAP4096_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fdMem, ADDRESS_ID & ~MAP4096_MASK);
	virtdieid = mapdieid + (ADDRESS_ID & MAP4096_MASK);
	sprintf(szSerial, "%08x", REG32(virtdieid + 0x04));
	PRINTF("ECID: %08x%08x%08x%08x\n", REG32(virtdieid + 0x00), REG32(virtdieid + 0x04), REG32(virtdieid + 0x08), REG32(virtdieid + 0x0C));
	PRINTF("GUID: %08x%08x%08x%08x\n", REG32(virtdieid + 0x44), REG32(virtdieid + 0x48), REG32(virtdieid + 0x4C), REG32(virtdieid + 0x50));
	PRINTF("Name: %08x\n", REG32(virtdieid + 0x10));
	munmap(mapdieid, MAP4096_SIZE);
	if (fdMem)
		close(fdMem);
#else
	readString(MDC_PATH, "serialnumber", szSerial, 16);
#endif
#endif
}

int killOtherPids(char *sz) {
	int ret = 0;
	char line_[1024];
	char *line = line_;
	sprintf(line, "pidof -o %d %s", getpid(), sz);
	FILE *cmd = popen(line, "r");
	strcpy(line, "");
	fgets(line, 1024, cmd);
	pclose(cmd);
	while (line) {
		pid_t pid = strtoul(line, NULL, 10);
		if (pid > 0) {
			kill(pid, SIGINT);
			ret = 1;
		}
		line = strchr(line, ' ');
		if (line)
			line++;
	}
	return ret;
}

int fileExists(char *st) {
	struct stat statTest;
	return (stat(st, &statTest) == 0);
}

void logInit(int daemon, int debug) {
	char sz[256];
	strcpy(sz, debug ? MAIN_PATH "mdc.log" : MAIN_PATH "tmp/mdc.log");
	int pipe_fd[2];
	pipe(pipe_fd);
	pidLog = fork();
	if (!pidLog) {
		close(pipe_fd[1]);
		FILE* logFile = fopen(sz, "a");
		char *buf = "===========================\n";
		fwrite(buf, strlen(buf), 1, logFile);
		char ch;
		while (doLog && read(pipe_fd[0], &ch, 1) > 0) {
			if (!daemon)
				putchar(ch);
			if (logFile)
				fputc(ch,logFile);
			if ('\n' == ch) {
				if (!daemon)
					fflush(stdout);
				if(logFile)
					fflush(logFile);
			}
		}
		close(pipe_fd[0]);
		if (logFile)
			fclose(logFile);
		exit(0);
	} else {
		close(pipe_fd[0]);
		dup2(pipe_fd[1], STDOUT_FILENO);
		dup2(pipe_fd[1], STDERR_FILENO);
		close(pipe_fd[1]);
	}
}

void logUninit() {
	doLog = 0;
}

void buzzer() {
#ifndef DESKTOP
	writeValueKey(MDC_PATH, "buzzer", "1");
#endif
}

void touchClick() {
	writeValueKey(MDC_PATH, "buzzerclick", "1");
}

void touch(char *szPath) {
	FILE *pf = fopen(szPath, "w+");
	if (pf)
		fclose(pf);
}

int hardwareVersion() {
	static int rv = -1;
#ifdef DESKTOP
	rv = 30;
#else
	if (rv == -1)
		rv = readValue(MDC_PATH, "hardwareVersion");
#endif
	return rv;
}

int downloadURLFile(char *szURL, char *szFile, int (*progresscallback)(void *, double,  double,  double,  double)) {
	CURLcode ret = -1;
	unlink(szFile);
	FILE *pf = fopen(szFile, "wb");
	if (pf) {
		CURL *curl = curl_easy_init();
		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, szURL);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, pf);
			if (progresscallback) {
				curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
				curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progresscallback);
			}
			ret = curl_easy_perform(curl);
			PRINTF("Download (ret:%d) %s to %s\n", ret, szURL, szFile);
			curl_easy_cleanup(curl);
		}
		fclose(pf);
	}
	return ret;
}

int uploadURLFile(char *szURL, char *szName0, char *szdata0, char *szName1, char *szFile1, char *szType1, char *szName2, char *szFile2, char *szType2, int (*progresscallback)(void *, double,  double,  double,  double)) {
	CURLcode ret = -1;
	CURL *curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, szURL);
		struct curl_httppost *post = NULL;
		struct curl_httppost *last = NULL;
		curl_formadd(&post, &last, CURLFORM_COPYNAME, szName0, CURLFORM_COPYCONTENTS, szdata0, CURLFORM_END);
		curl_formadd(&post, &last, CURLFORM_COPYNAME, szName1, CURLFORM_FILE, szFile1, CURLFORM_CONTENTTYPE, szType1, CURLFORM_END);
		curl_formadd(&post, &last, CURLFORM_COPYNAME, szName2, CURLFORM_FILE, szFile2, CURLFORM_CONTENTTYPE, szType2, CURLFORM_END);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
		if (progresscallback) {
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progresscallback);
		}
		ret = curl_easy_perform(curl);
		PRINTF("Upload (ret:%d) %s\n", ret, szURL);
		curl_easy_cleanup(curl);
	}
	return ret;
}

static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
	memcpy(userdata, ptr, MIN2(nmemb, 256));
	char *p = (char *)userdata;
	p[MIN2(nmemb, 255)] = '\0';
	return nmemb;
}

int downloadURLBuffer(char *szURL, char *buf) {
	CURLcode ret = -1;
	CURL *curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, szURL);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, buf);
		ret = curl_easy_perform(curl);
		PRINTF("Download (ret:%d) %s #%s#\n", ret, szURL, buf);
		curl_easy_cleanup(curl);
	}
	return ret;
}

void deleteDirectory(char *szFolder) {
	DIR *d = opendir(szFolder);
	struct dirent *dir;
	while ((dir = readdir(d)) != NULL) {
		if (dir->d_type == DT_REG) {
			char szPath[256];
			strcpy(szPath, szFolder);
			strcat(szPath, "/");
			strcat(szPath, dir->d_name);
			unlink(szPath);
		}  else if (dir->d_type == DT_DIR && strlen(dir->d_name) > 2) {
			char szFolder2[256];
			strcpy(szFolder2, szFolder);
			strcat(szFolder2, "/");
			strcat(szFolder2, dir->d_name);
			DIR *d2 = opendir(szFolder2);
			struct dirent *dir2;
			while ((dir2 = readdir(d2)) != NULL) {
				if (dir2->d_type == DT_REG) {
					char szPath[256];
					strcpy(szPath, szFolder2);
					strcat(szPath, "/");
					strcat(szPath, dir2->d_name);
					unlink(szPath);
				}
			}
			closedir(d2);
			rmdir(szFolder2);
		}
	}
	rmdir(szFolder);
	closedir(d);
}

void getMd5sum(char *szPath, char *szMd5sum) {
	int n;
	MD5_CTX c;
	char buf[512];
	ssize_t bytes;
	unsigned char out[MD5_DIGEST_LENGTH];

	MD5_Init(&c);
	FILE *f = fopen(szPath, "rb");
	while ((bytes = fread(buf, 1, 512, f)) > 0)
		MD5_Update(&c, buf, bytes);
	MD5_Final(out, &c);

	strcpy(szMd5sum, "");
	char sz[8];
	for(n = 0; n < MD5_DIGEST_LENGTH; n++) {
		sprintf(sz, "%02x", out[n]);
		strcat(szMd5sum, sz);
	}
}

int getLocalIP(char *szIPCurrent) {
	int ret = 0;
	int sockInet;
	if ((sockInet = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		PRINTF("Error opening socket");
		return -1;
	}

#ifdef DESKTOP
	struct ifaddrs *ifaddr;
	if (getifaddrs(&ifaddr) != -1) {
		struct ifaddrs *ifa;
		int family, s;
		for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)  {
			if (ifa->ifa_addr == NULL)
				continue;
			int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), szIPCurrent, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			if ((strcmp(ifa->ifa_name, "lo") != 0) && (ifa->ifa_addr->sa_family == AF_INET) && s == 0)
				break;
		}
		freeifaddrs(ifaddr);
	} else {
		ret = -1;
		PRINTF("ERROR: Can't get IP address\n");
	}
#else
	struct ifreq ifr;
	ifr.ifr_addr.sa_family = AF_INET;
	int getIPTries = 10;
	while(getIPTries > 0) {
		strcpy(ifr.ifr_name, "wlan0");
		if (ioctl(sockInet, SIOCGIFADDR, &ifr) == 0)
			break;
		PRINTF("Can't get IP address (retry #%d)\n", getIPTries);
		getIPTries--;
		usleep(1000 * 1000);
	}
	if (getIPTries == 0) {
		ret = -1;
		PRINTF("ERROR: Can't get IP address\n");
	}
	if (ret != -1)
		sprintf(szIPCurrent, "%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
#endif
	PRINTF("Current IP address is %s\n", szIPCurrent);
	close(sockInet);
	return ret;
}

void fillZeroFile(FILE *pf, int ssize) {
	fseek(pf, 0, SEEK_SET);
	int count = 0;
	char zeros[128];
	memset(zeros, 0, 128);
	while (count < ssize) {
		int a = MIN2(ssize - count, 128);
		fwrite(zeros, a, 1, pf);
		count += a;
	}
}

static int syncInProgress = 0;
static void *sync_t(void *arg) {
	if (syncInProgress)
		return 0;
	syncInProgress = 1;
	if ((long)arg == 1)
		usleep(1000 * 1000);
	sync();
	syncInProgress = 0;
}

void syncForce(int delay) {
	pthread_t pth;
	pthread_create(&pth, NULL, sync_t, delay ? (void *)1 : (void *)0);
	pthread_setname_np(pth, "sync");
}
