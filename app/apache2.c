#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "macro.h"
#include "cJSON.h"

//Functions
static void cJSON_Dump(cJSON *el) {
	char *sz = cJSON_Print(el);
	PRINTF("%s\n", sz);
	free(sz);
}

//Implement enabled
static void writePermissions(cJSON *elLocalRanges, char *authorized, FILE *pf) {
//ex: _all_,_dongle_,_local_,_allusers_,admin,user1
	char *authorized_ = strdup(authorized);
	char *token = strtok(authorized_, ",");
	while (token != NULL) {
		if (strstr(token, "_all_") != NULL) {
			char sz[] = "\t\tRequire all granted\n";
			fwrite(sz, strlen(sz), 1, pf);
		} else if (strcmp(token, "_dongle_") == 0) {
			char sz[] = "\t\tRequire local\n";
			fwrite(sz, strlen(sz), 1, pf);
		} else if (strcmp(token, "_local_") == 0) {
			cJSON *item = NULL;
			cJSON_ArrayForEach(item, elLocalRanges) {
				char sz[256];
				sprintf(sz, "\t\tRequire ip %s\n", item->valuestring);
				fwrite(sz, strlen(sz), 1, pf);
			}
		} else {
			char sz[] = "\t\tRequire valid-user\n";
			fwrite(sz, strlen(sz), 1, pf);
		}
		token = strtok(NULL, ",");
	}
    free(authorized_);
	char sz[] = "\t\tSatisfy any\n";
	fwrite(sz, strlen(sz), 1, pf);
}

static void writeSymlinks(int b, FILE *pf) {
	char sz[256];
	sprintf(sz, "\t\tOptions %sFollowSymLinks\n", b ? "+" : "-");
	fwrite(sz, strlen(sz), 1, pf);
}

static void writeIndexes(int b, FILE *pf) {
	char sz[256];
	sprintf(sz, "\t\tOptions %sIndexes\n", b ? "+" : "-");
	fwrite(sz, strlen(sz), 1, pf);
}

static void writeLog(char *name, FILE *pf) {
	char sz[256];
	sprintf(sz, "/disk/admin/.log/%s", name);
	mkdir(sz, 755);
	if (strcmp(name, "Apache2") == 0)
		sprintf(sz, "\tCustomLog /disk/admin/.log/%s/web.log combined\n", name);
	else
		sprintf(sz, "\tSetEnvIf Request_URI \"^/m/%s/\" %s_log\n\tCustomLog /disk/admin/.log/%s/web.log combined env=%s_log\n", name, name, name, name);
	fwrite(sz, strlen(sz), 1, pf);
}

void reloadApache2Conf() {
#if !defined(DESKTOP) && !defined(WEB)
	system("sudo /usr/bin/systemctl reload apache2");
#endif
}

void buildApache2Conf() {
	char sz2[10240];
	FILE *f;
	f = fopen(LOCAL_PATH "MyDongleCloud/modules.json", "r");
	if (f) {
		fread(sz2, sizeof(sz2), 1, f);
		fclose(f);
	}
	cJSON *modulesDefault = cJSON_Parse(sz2);
	f = fopen(ADMIN_PATH "MyDongleCloud/modules.json", "r");
	if (f) {
		fread(sz2, sizeof(sz2), 1, f);
		fclose(f);
	}
	cJSON *modules = cJSON_Parse(sz2);

	if (cJSON_IsTrue(cJSON_GetObjectItem(modules, "overwrite"))) {
		PRINTF("Apache2: main.conf not modified\n");
		reloadApache2Conf();
		return;
	}

#ifdef DESKTOP
	FILE *pf = fopen("/tmp/main.conf", "w");
#else
	FILE *pf = fopen(ADMIN_PATH "Apache2/main.conf", "w");
#endif
	char sz[512];
	strcpy(sz, "<VirtualHost *:80>\n\
	LoadModule mydonglecloud_module /usr/local/modules/Apache2/mod_mydonglecloud.so\n\
	LogLevel info mydonglecloud_module:info\n\
	DocumentRoot /disk/admin/Web\n\
	DirectoryIndex index.php index.html index.htm\n\
	FallbackResource /usr/local/modules/Apache2/home.php\n\
	#ErrorLog /disk/admin/.log/Web/web.log\n\
	ProxyRequests Off\n\
	ProxyPreserveHost On\n\
	RewriteEngine On\n\n");
	fwrite(sz, strlen(sz), 1, pf);

	cJSON *elLocalRanges = cJSON_GetObjectItem(cJSON_GetObjectItem(modulesDefault, "Apache2"), "localRanges");

	for (int i = 0; i < cJSON_GetArraySize(modulesDefault); i++) {
		cJSON *elModule = cJSON_GetArrayItem(modulesDefault, i);
		if (cJSON_HasObjectItem(elModule, "web")) {
			cJSON *elModule2 = cJSON_GetObjectItem(modules, elModule->string);

			char path[128];
			if (cJSON_HasObjectItem(elModule, "path"))
				strcpy(path, cJSON_GetStringValue(cJSON_GetObjectItem(elModule, "path")));
			else
				sprintf(path, "/usr/local/modules/%s", elModule->string);

			cJSON *elEnabled = cJSON_GetObjectItem(elModule, "enabled");
			if (cJSON_HasObjectItem(elModule2, "enabled"))
				elEnabled = cJSON_GetObjectItem(elModule2, "enabled");
			int enabled = cJSON_IsTrue(elEnabled);

			if (enabled) {
				if (cJSON_HasObjectItem(elModule, "reverseProxy")) {
					int port = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(elModule, "reverseProxy"));
					sprintf(sz, "\tRewriteRule \"/m/%s/(.*)$\" \"http://localhost:%d/$1\" [P]\n", elModule->string, port);
					fwrite(sz, strlen(sz), 1, pf);
				} else {
					if (strcmp(elModule->string, "Apache2") != 0) {
						sprintf(sz, "\tAlias \"/m/%s\" \"%s/\"\n", elModule->string, path);
						fwrite(sz, strlen(sz), 1, pf);
					}

					sprintf(sz, "\t<Directory \"%s/\">\n\t\tMyDongleCloudModule %s\n", path, elModule->string);
					fwrite(sz, strlen(sz), 1, pf);
					cJSON *elAuthorized = cJSON_GetObjectItem(elModule, "authorized");
					if (cJSON_HasObjectItem(elModule2, "authorized"))
						elAuthorized = cJSON_GetObjectItem(elModule2, "authorized");
					char *authorized = cJSON_GetStringValue(elAuthorized);
					writePermissions(elLocalRanges, authorized, pf);

					if (cJSON_GetObjectItem(elModule, "symlinks") || cJSON_HasObjectItem(elModule2, "symlinks"))
						writeSymlinks(cJSON_IsTrue(cJSON_GetObjectItem(elModule, "symlinks")) || cJSON_IsTrue(cJSON_GetObjectItem(elModule2, "symlinsk")), pf);

					if (cJSON_GetObjectItem(elModule, "indexes") || cJSON_HasObjectItem(elModule2, "indexes"))
						writeIndexes(cJSON_IsTrue(cJSON_GetObjectItem(elModule, "indexes")) || cJSON_IsTrue(cJSON_GetObjectItem(elModule2, "indexes")), pf);

					strcpy(sz, "\t</Directory>\n");
					fwrite(sz, strlen(sz), 1, pf);
				}
			} else {
				if (strcmp(elModule->string, "Apache2") == 0) {
				} else {
					sprintf(sz, "\tRedirectMatch permanent \"^/m/%s(.*)$\" \"/usr/local/modules/Apache2/disabled.php\"\n", elModule->string);
					fwrite(sz, strlen(sz), 1, pf);
				}
			}
			writeLog(elModule->string, pf);
			strcpy(sz, "\n");
			fwrite(sz, strlen(sz), 1, pf);
		}
	}

	strcpy(sz, "</VirtualHost>\n");
	fwrite(sz, strlen(sz), 1, pf);
	fclose(pf);
	cJSON_Delete(modules);
	cJSON_Delete(modulesDefault);
	PRINTF("Apache2: main.conf created\n");
	reloadApache2Conf();
}
