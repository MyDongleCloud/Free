#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <dirent.h>
#include <signal.h>
#include <pthread.h>
#include <netdb.h>
#include <stdarg.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/mman.h>
#ifndef WEB
#include <arpa/inet.h>
#include <linux/wireless.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <openssl/md5.h>
#include <liboath/oath.h>
#endif
#include "macro.h"
#include "common.h"

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
	sz[16] = '\0';
}

void generateRandomHexString(char sz[33]) {
	sz[0] = '\0';
	char buffer[16];
	int fd = open("/dev/urandom", O_RDONLY);
	read(fd, buffer, 16);
	close(fd);
	char ss[3];
	int i;
	for (i = 0; i < 16; i++) {
		sprintf(ss, "%02x", buffer[i]);
		strcat(sz, ss);
	}
	sz[32] = '\0';
}

#ifndef WEB
int oathGenerate(char secret[33]) {
	int ret = 0;
	generateRandomHexString(secret);
	oath_init();
	char otp[8];
	char secretbin[17];
	size_t secretbinlen = 16;
	oath_hex2bin(secret, secretbin, &secretbinlen);
	oath_hotp_generate(secretbin, secretbinlen, 0, 6, 0, OATH_HOTP_DYNAMIC_TRUNCATION, otp);
	oath_done();
	sscanf(otp, "%d", &ret);
	return ret;
}

int oathValidate(char secret[33], int OTP) {
	if (OTP < 0 || OTP > 999999)
		return 0;
	char otp[8];
	sprintf(otp, "%d", OTP);
	oath_init();
	int ret = oath_hotp_validate(secret, strlen(secret), 0, 20, otp);
	oath_done();
	return ret;
}
#endif

void getSerialID() {
#ifdef DESKTOP
	strcpy(szSerial, "1234567890abcdef");
#else
	readString(PLATFORM_PATH, "serialNumber", szSerial, 16);
#endif
}

#ifndef WEB
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
	strcpy(sz, debug ? "/var/log/mydonglecloud-app.log" : "/tmp/app.log");
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
#endif

void logUninit() {
	doLog = 0;
}

void buzzer(int n) {
#ifndef DESKTOP
	writeValueKeyInt(PLATFORM_PATH, "buzzer", n);
#endif
}

void touchClick() {
	writeValueKey(PLATFORM_PATH, "buzzerClick", "1");
}

static void *jingle_t(void *arg) {
#define _100MS 100 * 1000
#define pwm(a, b) writeValueKeyInt(PLATFORM_PATH, "buzzerFreq", a);
#define Task_sleep usleep
	pwm(2000, 1); Task_sleep(2 * _100MS); pwm(0, 0); //200ms 2kHz
	Task_sleep(2 * _100MS);
	pwm(4000, 1); Task_sleep(1 * _100MS); pwm(0, 0); //100ms 4kHz
	Task_sleep(1 * _100MS);
	pwm(4000, 1); Task_sleep(2 * _100MS); pwm(0, 0); //200ms 4kHz
	Task_sleep(2 * _100MS);
	pwm(1650, 1); Task_sleep(1 * _100MS); pwm(0, 0); //100ms 1.65kHz
	Task_sleep(1 * _100MS);
	pwm(1850, 1); Task_sleep(2 * _100MS); pwm(0, 0); //200ms 1.85kHz
	Task_sleep(2 * _100MS);
	pwm(1550, 1); Task_sleep(2 * _100MS); pwm(0, 0); //200ms 1.55kHz
	Task_sleep(2 * _100MS);
	pwm(1650, 1); Task_sleep(2 * _100MS); pwm(0, 0); //200ms 1.65kHz
	Task_sleep(2 * _100MS);
	pwm(2000, 1); Task_sleep(2 * _100MS); pwm(0, 0); //200ms 2kHz
}

void jingle() {
#ifndef DESKTOP
	pthread_t pth;
	pthread_create(&pth, NULL, jingle_t, NULL);
#endif
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
		rv = readValue(PLATFORM_PATH, "hardwareVersion");
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
