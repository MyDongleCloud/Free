#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "macro.h"
#include "cJSON.h"

//Functions
static void writePermissions(cJSON *elLocalRanges, char *authorized, FILE *pf) {
//ex: _all_,_dongle_,_localnetwork_,_allusers_,admin,user1
	if (strstr(authorized, "_all_") != NULL) {
		char sz[] = "\t\tRequire all granted\n";
		fwrite(sz, strlen(sz), 1, pf);
	} else {
		char *authorized_ = strdup(authorized);
		char *token = strtok(authorized_, ",");
		int requireNb = 0;
		char list[256];
		strcpy(list, "");
		while (token != NULL) {
			if (strcmp(token, "_dongle_") == 0) {
				char sz[] = "\t\tRequire local\n";
				fwrite(sz, strlen(sz), 1, pf);
			} else if (strcmp(token, "_localnetwork_") == 0) {
				cJSON *item = NULL;
				cJSON_ArrayForEach(item, elLocalRanges) {
					char sz[256];
					sprintf(sz, "\t\tRequire ip %s\n", item->valuestring);
					fwrite(sz, strlen(sz), 1, pf);
				}
			} else {
				if (strlen(list) == 0)
					strcpy(list, "\t\tRequire all granted\n\t\tMyDongleCloudModuleAuthorized");
				strcat(list, " ");
				strcat(list, token);
			}
			token = strtok(NULL, ",");
			requireNb++;
		}
		free(authorized_);
		if (strlen(list) != 0) {
			strcat(list, "\n");
			fwrite(list, strlen(list), 1, pf);
		}
		if (requireNb > 1) {
				char sz[] = "\t\tSatisfy any\n";
				fwrite(sz, strlen(sz), 1, pf);
		}
	}
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

static void writeDirectoryIndex(char *s, FILE *pf) {
	char sz[256];
	sprintf(sz, "\t\tDirectoryIndex %s\n", s);
	fwrite(sz, strlen(sz), 1, pf);
}

static void writeLog(char *name, FILE *pf) {
	char sz[256];
	sprintf(sz, "/disk/admin/.log/%s", name);
	mkdir(sz, 755);
	if (strcmp(name, "Apache2") == 0)
		sprintf(sz, "\tCustomLog /disk/admin/.log/%s/web.log combined\n", name);
	else
		sprintf(sz, "\tSetEnvIf Request_URI ^/m/%s %s_log\n\tCustomLog /disk/admin/.log/%s/web.log combined env=%s_log\n", name, name, name, name);
	fwrite(sz, strlen(sz), 1, pf);
}

void reloadApache2Conf() {
#if !defined(DESKTOP) && !defined(WEB)
	system("sudo /usr/bin/systemctl reload apache2");
	PRINTF("Apache2: Reloaded\n");
#endif
}

void buildApache2Conf(cJSON *modulesDefault, cJSON *modules) {
#ifdef DESKTOP
	FILE *pf = fopen("/tmp/main.conf", "w");
#else
	FILE *pf = fopen(ADMIN_PATH "Apache2/main.conf", "w");
#endif
	char sz[1024];
	strcpy(sz, "<VirtualHost *:80>\n\
	RewriteEngine On\n\
	ProxyRequests Off\n\
	ProxyPreserveHost On\n\
	DirectoryIndex index.php index.html\n\
	LoadModule mydonglecloud_module /usr/local/modules/Apache2/mod_mydonglecloud.so\n\
	LogLevel info mydonglecloud_module:info\n\n\
	<Directory /usr/local/modules/Apache2/pages/>\n\
		Require all granted\n\
	</Directory>\n\
	ErrorDocument 401 /m/unauthorized.html\n\
	RewriteRule ^/m/unauthorized.html /usr/local/modules/Apache2/pages/unauthorized.html [L]\n\
	ErrorDocument 403 /m/denied.html\n\
	RewriteRule ^/m/denied.html /usr/local/modules/Apache2/pages/denied.html [L]\n\
	ErrorDocument 404 /m/notpresent.html\n\
	RewriteRule ^/m/notpresent.html /usr/local/modules/Apache2/pages/notpresent.html [L]\n\
	RewriteRule ^/m/disabled.html /usr/local/modules/Apache2/pages/disabled.html [L]\n\
	RewriteRule ^/m/login.html /usr/local/modules/Apache2/pages/login.html [L]\n\
	RewriteRule ^/m/login_submit.php /usr/local/modules/Apache2/pages/login_submit.php [L]\n\n");
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

			if (strcmp(elModule->string, "Apache2") == 0) {
				sprintf(sz, "\tDocumentRoot \"%s\"\n\
	RewriteCond %s/index.html !-f\n\
	RewriteRule ^/(index\\.html)?$ /m/home.html\n\
	RewriteRule ^/m/home.html /usr/local/modules/Apache2/pages/home.html [L]\n", path, path);
				fwrite(sz, strlen(sz), 1, pf);
			}
			cJSON *elEnabled = cJSON_GetObjectItem(elModule, "enabled");
			if (cJSON_HasObjectItem(elModule2, "enabled"))
				elEnabled = cJSON_GetObjectItem(elModule2, "enabled");
			if (cJSON_IsTrue(elEnabled)) {
				if (cJSON_HasObjectItem(elModule, "reverseProxy")) {
					int port = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(elModule, "reverseProxy"));
					sprintf(sz, "\tRewriteRule /m/%s/(.*)$ http://localhost:%d/$1 [P]\n", elModule->string, port);
					fwrite(sz, strlen(sz), 1, pf);
				} else {
					if (strcmp(elModule->string, "Apache2") != 0) {
						sprintf(sz, "\tAlias /m/%s %s/\n", elModule->string, path);
						fwrite(sz, strlen(sz), 1, pf);
					}
					sprintf(sz, "\t<Directory %s/>\n\t\tMyDongleCloudModule %s\n", path, elModule->string);
					fwrite(sz, strlen(sz), 1, pf);
					cJSON *elAuthorized = cJSON_GetObjectItem(elModule, "authorized");
					if (cJSON_HasObjectItem(elModule2, "authorized") && !cJSON_IsFalse(cJSON_GetObjectItem(elModule, "overwrite")))
						elAuthorized = cJSON_GetObjectItem(elModule2, "authorized");
					char *authorized = cJSON_GetStringValue(elAuthorized);
					writePermissions(elLocalRanges, authorized, pf);
					if (cJSON_HasObjectItem(elModule2, "DirectoryIndex"))
						writeDirectoryIndex(cJSON_GetStringValue(cJSON_GetObjectItem(elModule2, "DirectoryIndex")), pf);
					else if (cJSON_HasObjectItem(elModule, "DirectoryIndex"))
						writeDirectoryIndex(cJSON_GetStringValue(cJSON_GetObjectItem(elModule, "DirectoryIndex")), pf);
					if (cJSON_HasObjectItem(elModule2, "FollowSymLinks"))
						writeSymlinks(cJSON_IsTrue(cJSON_GetObjectItem(elModule2, "FollowSymLinks")), pf);
					else if (cJSON_GetObjectItem(elModule, "FollowSymLinks"))
						writeSymlinks(cJSON_IsTrue(cJSON_GetObjectItem(elModule, "FollowSymLinks")), pf);
					if (cJSON_HasObjectItem(elModule2, "Indexes"))
						writeIndexes(cJSON_IsTrue(cJSON_GetObjectItem(elModule2, "Indexes")), pf);
					else if (cJSON_GetObjectItem(elModule, "Indexes"))
						writeIndexes(cJSON_IsTrue(cJSON_GetObjectItem(elModule, "Indexes")), pf);

					strcpy(sz, "\t</Directory>\n");
					fwrite(sz, strlen(sz), 1, pf);
				}
			} else {
				if (strcmp(elModule->string, "Apache2") == 0) {
				} else {
					sprintf(sz, "\tRedirectMatch permanent ^/m/%s(.*)$ /m/disabled.html?m=%s\n", elModule->string, elModule->string);
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
	PRINTF("Apache2: Creation of main.conf\n");
}
