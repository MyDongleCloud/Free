#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "macro.h"
#include "cJSON.h"

//Functions
static void writePermissions(cJSON *elLocalRanges, char *authorized, FILE *pfM) {
//ex: _all_,_dongle_,_localnetwork_,_allusers_,admin,user1
	if (strstr(authorized, "_all_") != NULL) {
		char sz[] = "\t\tRequire all granted\n";
		fwrite(sz, strlen(sz), 1, pfM);
	} else {
		char *authorized_ = strdup(authorized);
		char *token = strtok(authorized_, ",");
		int requireNb = 0;
		char list[256];
		strcpy(list, "");
		while (token != NULL) {
			if (strcmp(token, "_dongle_") == 0) {
				char sz[] = "\t\tRequire local\n";
				fwrite(sz, strlen(sz), 1, pfM);
			} else if (strcmp(token, "_localnetwork_") == 0) {
				cJSON *item = NULL;
				cJSON_ArrayForEach(item, elLocalRanges) {
					char sz[256];
					sprintf(sz, "\t\tRequire ip %s\n", item->valuestring);
					fwrite(sz, strlen(sz), 1, pfM);
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
			fwrite(list, strlen(list), 1, pfM);
		}
		if (requireNb > 1) {
				char sz[] = "\t\tSatisfy any\n";
				fwrite(sz, strlen(sz), 1, pfM);
		}
	}
}

static void writeSymlinks(int b, FILE *pfM) {
	char sz[256];
	sprintf(sz, "\t\tOptions %sFollowSymLinks\n", b ? "+" : "-");
	fwrite(sz, strlen(sz), 1, pfM);
}

static void writeIndexes(int b, FILE *pfM) {
	char sz[256];
	sprintf(sz, "\t\tOptions %sIndexes\n", b ? "+" : "-");
	fwrite(sz, strlen(sz), 1, pfM);
}

static void writeDirectoryIndex(char *s, FILE *pfM) {
	char sz[256];
	sprintf(sz, "\t\tDirectoryIndex %s\n", s);
	fwrite(sz, strlen(sz), 1, pfM);
}

static void writeLog(char *name, FILE *pfM) {
	char sz[256];
	sprintf(sz, "/disk/admin/.log/%s", name);
	mkdir(sz, 755);
	sprintf(sz, "\tCustomLog /disk/admin/.log/%s/web.log combined\n", name);
	fwrite(sz, strlen(sz), 1, pfM);
}

void reloadApache2Conf() {
#ifndef DESKTOP
	PRINTF("Apache2: Reloading\n");
	system("sudo /usr/bin/systemctl reload apache2");
#endif
}

void buildApache2Conf(cJSON *modulesDefault, cJSON *modules, char *fqdn) {
	PRINTF("Modules:Apache2: Enter\n");
#ifdef DESKTOP
	FILE *pfP = fopen("/tmp/ports.conf", "w");
#else
	FILE *pfP = fopen(ADMIN_PATH "Apache2/ports.conf", "w");
#endif
	char sz[2048];
	strcpy(sz, "\
Listen 80\n\
<IfModule ssl_module>\n\
	Listen 443\n\
</IfModule>\n");
	fwrite(sz, strlen(sz), 1, pfP);
#ifdef DESKTOP
	FILE *pfM = fopen("/tmp/main.conf", "w");
#else
	FILE *pfM = fopen(ADMIN_PATH "Apache2/main.conf", "w");
#endif
	strcpy(sz, "LoadModule mydonglecloud_module /usr/local/modules/Apache2/mod_mydonglecloud.so\n\
\n\
<Macro Macro_Redirect>\n\
	Alias /MyDongleCloud /usr/local/modules/Apache2/pages\n\
	<Directory /usr/local/modules/Apache2/pages>\n\
		Require all granted\n\
	</Directory>\n\
	ErrorDocument 401 /MyDongleCloud/unauthorized.php\n\
	ErrorDocument 403 /MyDongleCloud/denied.php\n\
	ErrorDocument 404 /MyDongleCloud/notpresent.php\n\
</Macro>\n\
<Macro Macro_SSL>\n\
	Include /usr/local/modules/Apache2/options-ssl-apache.conf\n\
	SSLCertificateFile /disk/admin/.modules/Apache2/fullchain.pem\n\
	SSLCertificateKeyFile /disk/admin/.modules/Apache2/privkey.pem\n\
</Macro>\n\n");
	fwrite(sz, strlen(sz), 1, pfM);
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
			sprintf(sz, "\
<Macro Macro_%s>\n\
	MyDongleCloudModule %s\n\
	RewriteEngine On\n", elModule->string, elModule->string);
			fwrite(sz, strlen(sz), 1, pfM);
			if (cJSON_HasObjectItem(elModule, "rewriteRule")) {
				sprintf(sz, "\t%s\n", cJSON_GetStringValue(cJSON_GetObjectItem(elModule, "rewriteRule")));
				fwrite(sz, strlen(sz), 1, pfM);
			}
			if (cJSON_HasObjectItem(elModule2, "rewriteRule")) {
				sprintf(sz, "\t%s\n", cJSON_GetStringValue(cJSON_GetObjectItem(elModule2, "rewriteRule")));
				fwrite(sz, strlen(sz), 1, pfM);
			}


			cJSON *elEnabled = cJSON_GetObjectItem(elModule, "enabled");
			if (cJSON_HasObjectItem(elModule2, "enabled"))
				elEnabled = cJSON_GetObjectItem(elModule2, "enabled");
			if (cJSON_IsTrue(elEnabled)) {
				cJSON *elAuthorized = cJSON_GetObjectItem(elModule, "authorized");
				if (cJSON_HasObjectItem(elModule2, "authorized") && !cJSON_IsFalse(cJSON_GetObjectItem(elModule, "overwrite")))
					elAuthorized = cJSON_GetObjectItem(elModule2, "authorized");
				char *authorized = cJSON_GetStringValue(elAuthorized);

				if (cJSON_HasObjectItem(elModule, "reverseProxy")) {
					strcpy(sz, "\tProxyRequests Off\n\tProxyPreserveHost on\n");
					fwrite(sz, strlen(sz), 1, pfM);
					cJSON *elReverseProxy = cJSON_GetObjectItem(elModule, "reverseProxy");
					for (int j = 0; j < cJSON_GetArraySize(elReverseProxy); j++) {
						cJSON *elReverseProxy_ = cJSON_GetArrayItem(elReverseProxy, j);
						char *type_ = cJSON_GetStringValue(cJSON_GetObjectItem(elReverseProxy_, "type"));
						char *path_ = cJSON_GetStringValue(cJSON_GetObjectItem(elReverseProxy_, "path"));
						int port_ = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(elReverseProxy_, "port"));
						sprintf(sz, "\tProxyPass %s %s://localhost:%d%s\n\tProxyPassReverse %s %s://localhost:%d%s\n", path_, type_, port_, path_, path_, type_, port_, path_);
						fwrite(sz, strlen(sz), 1, pfM);
					}
					strcpy(sz, "\t<Proxy *>\n");
					fwrite(sz, strlen(sz), 1, pfM);
				} else {
					sprintf(sz, "\tDocumentRoot %s\n\t<Directory %s>\n", path, path);
					fwrite(sz, strlen(sz), 1, pfM);
				}
				if (cJSON_HasObjectItem(elModule, "addLineInDirectory")) {
					sprintf(sz, "\t\t%s\n", cJSON_GetStringValue(cJSON_GetObjectItem(elModule, "addLineInDirectory")));
					fwrite(sz, strlen(sz), 1, pfM);
				}
				if (cJSON_HasObjectItem(elModule2, "addLineInDirectory")) {
					sprintf(sz, "\t\t%s\n", cJSON_GetStringValue(cJSON_GetObjectItem(elModule2, "addLineInDirectory")));
					fwrite(sz, strlen(sz), 1, pfM);
				}
				writePermissions(elLocalRanges, authorized, pfM);
				if (cJSON_HasObjectItem(elModule2, "DirectoryIndex"))
					writeDirectoryIndex(cJSON_GetStringValue(cJSON_GetObjectItem(elModule2, "DirectoryIndex")), pfM);
				else if (cJSON_HasObjectItem(elModule, "DirectoryIndex"))
					writeDirectoryIndex(cJSON_GetStringValue(cJSON_GetObjectItem(elModule, "DirectoryIndex")), pfM);
				if (cJSON_HasObjectItem(elModule2, "FollowSymLinks"))
					writeSymlinks(cJSON_IsTrue(cJSON_GetObjectItem(elModule2, "FollowSymLinks")), pfM);
				else if (cJSON_GetObjectItem(elModule, "FollowSymLinks"))
					writeSymlinks(cJSON_IsTrue(cJSON_GetObjectItem(elModule, "FollowSymLinks")), pfM);
				if (cJSON_HasObjectItem(elModule2, "Indexes"))
					writeIndexes(cJSON_IsTrue(cJSON_GetObjectItem(elModule2, "Indexes")), pfM);
				else if (cJSON_GetObjectItem(elModule, "Indexes"))
					writeIndexes(cJSON_IsTrue(cJSON_GetObjectItem(elModule, "Indexes")), pfM);

				if (cJSON_HasObjectItem(elModule, "reverseProxy"))
					strcpy(sz, "\t</Proxy>\n");
				else
					strcpy(sz, "\t</Directory>\n");
				fwrite(sz, strlen(sz), 1, pfM);

				if (strcmp(elModule->string, "Apache2") == 0) {
					for (int ii = 0; ii < cJSON_GetArraySize(modulesDefault); ii++) {
						cJSON *el_Module = cJSON_GetArrayItem(modulesDefault, ii);
						int port_ = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(el_Module, "reverseProxy"), 0), "port"));
						if (port_ <= 0)
							port_ = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(el_Module, "fallbackPort"));
						if (port_ > 0) {
							sprintf(sz, "\
	RewriteCond %%{HTTP_HOST} ^(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})$\n\
	RewriteRule ^/(MyDongleCloud|m)/%s.* http://%%{HTTP_HOST}:%d [NC,L]\n\
	RewriteRule ^/(MyDongleCloud|m)/%s.* http://%s.%s [NC,L]\n", el_Module->string, port_, el_Module->string, el_Module->string, fqdn);
							fwrite(sz, strlen(sz), 1, pfM);
						}
					}
					strcpy(sz, "\n\tAlias /MyDongleCloud /usr/local/modules/Apache2/pages\n\tAlias /m /usr/local/modules/Apache2/pages\n");
					fwrite(sz, strlen(sz), 1, pfM);
				}
				strcpy(sz, "\tUse Macro_Redirect\n");
				fwrite(sz, strlen(sz), 1, pfM);
			} else {
				sprintf(sz, "\
	RewriteCond %%{HTTP_HOST} ^(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})(:\\d+)?$\n\
	RewriteRule ^/.*$ http://%%1/MyDongleCloud/disabled.php?m=%s\n\
	RewriteRule ^/.*$ https://%s/MyDongleCloud/disabled.php?m=%s\n\n", elModule->string, fqdn, elModule->string);
				fwrite(sz, strlen(sz), 1, pfM);
			}
			writeLog(elModule->string, pfM);
			sprintf(sz, "</Macro>\n");
			fwrite(sz, strlen(sz), 1, pfM);

			sprintf(sz, "<VirtualHost *:80>\n");
			fwrite(sz, strlen(sz), 1, pfM);
			if (fqdn != NULL) {
				if (strcmp(elModule->string, "Apache2") == 0)
					sprintf(sz, "\tServerName %s\n", fqdn);
				else
					sprintf(sz, "\tServerName %s.%s\n", elModule->string, fqdn);
				fwrite(sz, strlen(sz), 1, pfM);
				if (cJSON_HasObjectItem(elModule, "alias")) {
					sprintf(sz, "\tServerAlias %s.%s\n", cJSON_GetStringValue(cJSON_GetObjectItem(elModule, "alias")), fqdn);
					fwrite(sz, strlen(sz), 1, pfM);
				}
			}
			sprintf(sz, "\tUse Macro_%s\n</VirtualHost>\n", elModule->string);
			fwrite(sz, strlen(sz), 1, pfM);

			sprintf(sz, "<IfModule mod_ssl.c>\n\t<VirtualHost *:443>\n");
			fwrite(sz, strlen(sz), 1, pfM);
			if (fqdn != NULL) {
				if (strcmp(elModule->string, "Apache2") == 0)
					sprintf(sz, "\t\tServerName %s\n", fqdn);
				else
					sprintf(sz, "\t\tServerName %s.%s\n", elModule->string, fqdn);
				fwrite(sz, strlen(sz), 1, pfM);
				if (cJSON_HasObjectItem(elModule, "alias")) {
					sprintf(sz, "\t\tServerAlias %s.%s\n", cJSON_GetStringValue(cJSON_GetObjectItem(elModule, "alias")), fqdn);
					fwrite(sz, strlen(sz), 1, pfM);
				}
			}
			sprintf(sz, "\t\tUse Macro_%s\n\t\tUse Macro_SSL\n\t</VirtualHost>\n</IfModule>\n", elModule->string);
			fwrite(sz, strlen(sz), 1, pfM);
			int fallbackPort = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(elModule, "fallbackPort"));
			if (fallbackPort > 0) {
				sprintf(sz, "<VirtualHost *:%d>\n\tUse Macro_%s\n</VirtualHost>\n", fallbackPort, elModule->string);
				fwrite(sz, strlen(sz), 1, pfM);
				sprintf(sz, "Listen %d\n", fallbackPort);
				fwrite(sz, strlen(sz), 1, pfP);
			}
			strcpy(sz, "\n\n");
			fwrite(sz, strlen(sz), 1, pfM);
		}
	}
	fclose(pfP);
	fclose(pfM);
}
