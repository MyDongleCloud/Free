#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "common.h"
#include "macro.h"
#include "cJSON.h"
#include "apache2.h"

//Functions
static void jsonDump(cJSON *el) {
	char *sz = cJSON_Print(el);
	PRINTF("%s\n", sz);
	free(sz);
}

static cJSON *jsonRead(char *path) {
	struct stat statTest;
	if (stat(path, &statTest) != 0 || statTest.st_size == 0)
		return NULL;
	int size = statTest.st_size + 16;
	char *sz = malloc(size);
	FILE *f = fopen(path, "r");
	if (f) {
		fread(sz, size, 1, f);
		fclose(f);
	}
	cJSON *ret = cJSON_Parse(sz);
	free(sz);
	return ret;
}

static void jsonWrite(cJSON *el, char *path) {
	FILE *f = fopen(path, "w");
	if (f) {
		char *sz = cJSON_Print(el);
		fwrite(sz, strlen(sz), 1, f);
		fclose(f);
		free(sz);
	}
}

void modulesSetup() {
	cJSON *modulesDefault = jsonRead(LOCAL_PATH "MyDongleCloud/modules.json");
	cJSON *modules = jsonRead(ADMIN_PATH "MyDongleCloud/modules.json");
	if (modules == NULL)
		modules = cJSON_CreateObject();
	int hasInit = cJSON_IsTrue(cJSON_GetObjectItem(modules, "initDone"));

	for (int i = 0; i < cJSON_GetArraySize(modulesDefault); i++) {
		cJSON *elModule = cJSON_GetArrayItem(modulesDefault, i);
		cJSON *elModule2 = cJSON_GetObjectItem(modules, elModule->string);

		if (strcmp(elModule->string, "Apache2") == 0) {
			if (cJSON_IsTrue(cJSON_GetObjectItem(elModule2, "overwrite"))) {
				PRINTF("Apache2: No creation of main.conf\n");
			} else
				buildApache2Conf(modulesDefault, modules);
#ifndef DESKTOP
			reloadApache2Conf();
#endif
		} else if (strcmp(elModule->string, "Audiobookshelf") == 0) {
		} else if (strcmp(elModule->string, "Bugzilla") == 0) {
		} else if (strcmp(elModule->string, "Busybox") == 0) {
		} else if (strcmp(elModule->string, "Certbot") == 0) {
		} else if (strcmp(elModule->string, "ChangeDetection") == 0) {
		} else if (strcmp(elModule->string, "Clang") == 0) {
		} else if (strcmp(elModule->string, "CMake") == 0) {
		} else if (strcmp(elModule->string, "collaboraonline") == 0) {
		} else if (strcmp(elModule->string, "Composer") == 0) {
		} else if (strcmp(elModule->string, "ConvertX") == 0) {
		} else if (strcmp(elModule->string, "Cyberchef") == 0) {
		} else if (strcmp(elModule->string, "Discourse") == 0) {
		} else if (strcmp(elModule->string, "Docker") == 0) {
		} else if (strcmp(elModule->string, "Doodle") == 0) {
		} else if (strcmp(elModule->string, "Dovecot") == 0) {
		} else if (strcmp(elModule->string, "FFmpeg") == 0) {
		} else if (strcmp(elModule->string, "Fileflows") == 0) {
		} else if (strcmp(elModule->string, "Flarum") == 0) {
		} else if (strcmp(elModule->string, "FreshRSS") == 0) {
		} else if (strcmp(elModule->string, "fscrypt") == 0) {
		} else if (strcmp(elModule->string, "GCC") == 0) {
		} else if (strcmp(elModule->string, "Git") == 0) {
		} else if (strcmp(elModule->string, "Gitea") == 0) {
		} else if (strcmp(elModule->string, "Go") == 0) {
		} else if (strcmp(elModule->string, "GoAccess") == 0) {
		} else if (strcmp(elModule->string, "GoodIP") == 0) {
		} else if (strcmp(elModule->string, "Grav") == 0) {
		} else if (strcmp(elModule->string, "HomeAssistant") == 0) {
		} else if (strcmp(elModule->string, "Html5-QRCode") == 0) {
		} else if (strcmp(elModule->string, "Hugo") == 0) {
		} else if (strcmp(elModule->string, "ImageMagick") == 0) {
		} else if (strcmp(elModule->string, "Immich") == 0) {
		} else if (strcmp(elModule->string, "IOPaint") == 0) {
		} else if (strcmp(elModule->string, "Java") == 0) {
		} else if (strcmp(elModule->string, "Jellyfin") == 0) {
		} else if (strcmp(elModule->string, "JitsiMeet") == 0) {
		} else if (strcmp(elModule->string, "Joomla") == 0) {
		} else if (strcmp(elModule->string, "Joplin") == 0) {
		} else if (strcmp(elModule->string, "Karakeep") == 0) {
		} else if (strcmp(elModule->string, "Kernel") == 0) {
		} else if (strcmp(elModule->string, "LibrePhotos") == 0) {
		} else if (strcmp(elModule->string, "Libreqr") == 0) {
		} else if (strcmp(elModule->string, "LimeSurvey") == 0) {
		} else if (strcmp(elModule->string, "Make") == 0) {
		} else if (strcmp(elModule->string, "MantisBugTracker") == 0) {
		} else if (strcmp(elModule->string, "Maybe") == 0) {
		} else if (strcmp(elModule->string, "MeTube") == 0) {
		} else if (strcmp(elModule->string, "MinIO") == 0) {
		} else if (strcmp(elModule->string, "MongoDB") == 0) {
		} else if (strcmp(elModule->string, "Mosquitto") == 0) {
		} else if (strcmp(elModule->string, "MydongleCloud") == 0) {
		} else if (strcmp(elModule->string, "MySQL") == 0) {
		} else if (strcmp(elModule->string, "Nginx") == 0) {
		} else if (strcmp(elModule->string, "Node") == 0) {
		} else if (strcmp(elModule->string, "Ollama") == 0) {
		} else if (strcmp(elModule->string, "OneTimeEmail") == 0) {
		} else if (strcmp(elModule->string, "OpenWebUI") == 0) {
		} else if (strcmp(elModule->string, "osTicket") == 0) {
#ifndef DESKTOP
			if (!fileExists(ADMIN_PATH "osTicket/ost-config.php")) {
				mkdir(ADMIN_PATH "osTicket", 0775);
				copyFile(LOCAL_PATH "osTicket/include/ost-sampleconfig.php", ADMIN_PATH "osTicket/ost-config.php", NULL);
			}
#endif
		} else if (strcmp(elModule->string, "Pandoc") == 0) {
		} else if (strcmp(elModule->string, "PhotoPrism") == 0) {
		} else if (strcmp(elModule->string, "PhotoView") == 0) {
		} else if (strcmp(elModule->string, "PHP") == 0) {
		} else if (strcmp(elModule->string, "phpBB") == 0) {
		} else if (strcmp(elModule->string, "phpList") == 0) {
		} else if (strcmp(elModule->string, "Postfix") == 0) {
		} else if (strcmp(elModule->string, "PrivateBin") == 0) {
		} else if (strcmp(elModule->string, "ProjectSend") == 0) {
		} else if (strcmp(elModule->string, "PyMCUProg") == 0) {
		} else if (strcmp(elModule->string, "Python") == 0) {
		} else if (strcmp(elModule->string, "Python") == 0) {
		} else if (strcmp(elModule->string, "Qdrant") == 0) {
		} else if (strcmp(elModule->string, "QRCodeGenerator") == 0) {
		} else if (strcmp(elModule->string, "Radaar") == 0) {
		} else if (strcmp(elModule->string, "RethinkDB") == 0) {
		} else if (strcmp(elModule->string, "RoundCube") == 0) {
		} else if (strcmp(elModule->string, "rspamd") == 0) {
		} else if (strcmp(elModule->string, "Sonarr") == 0) {
		} else if (strcmp(elModule->string, "SQLite") == 0) {
		} else if (strcmp(elModule->string, "StirlingPDF") == 0) {
		} else if (strcmp(elModule->string, "SunriseCMS") == 0) {
		} else if (strcmp(elModule->string, "Superset") == 0) {
		} else if (strcmp(elModule->string, "Syncthing") == 0) {
		} else if (strcmp(elModule->string, "Transmission") == 0) {
		} else if (strcmp(elModule->string, "TriliumNotes") == 0) {
		} else if (strcmp(elModule->string, "TubeArchivist") == 0) {
		} else if (strcmp(elModule->string, "uMTP-Responder") == 0) {
		} else if (strcmp(elModule->string, "Unmanic") == 0) {
		} else if (strcmp(elModule->string, "Uptime") == 0) {
		} else if (strcmp(elModule->string, "Webtrees") == 0) {
		} else if (strcmp(elModule->string, "WordPress") == 0) {
		} else if (strcmp(elModule->string, "Write.as") == 0) {
		} else if (strcmp(elModule->string, "YOURLS") == 0) {
		} else if (strcmp(elModule->string, "Youtube-dl") == 0) {
		} else if (strcmp(elModule->string, "Zigbee2MQTT") == 0) {
		}
	}

	if (!hasInit)
		cJSON_AddBoolToObject(modules, "initDone", cJSON_True);
#ifndef DESKTOP
	jsonWrite(modules, ADMIN_PATH "MyDongleCloud/modules.json");
#endif
	cJSON_Delete(modules);
	cJSON_Delete(modulesDefault);
}
