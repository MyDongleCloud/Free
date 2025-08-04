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
	sprintf(sz, "\tCustomLog /disk/admin/.log/%s/web.log combined\n", name);
	fwrite(sz, strlen(sz), 1, pf);
}

void reloadApache2Conf() {
#if !defined(DESKTOP) && !defined(WEB)
	system("sudo /usr/bin/systemctl reload apache2");
	PRINTF("Apache2: Reloaded\n");
#endif
}

void buildApache2Conf(cJSON *modulesDefault, cJSON *modules, char *domain) {
#ifdef DESKTOP
	FILE *pf = fopen("/tmp/main.conf", "w");
#else
	FILE *pf = fopen(ADMIN_PATH "Apache2/main.conf", "w");
#endif
	char sz[2048];
	strcpy(sz, "LoadModule mydonglecloud_module /usr/local/modules/Apache2/mod_mydonglecloud.so\n\
\n\
<Macro Macro_Redirect>\n\
	<Directory /usr/local/modules/Apache2/pages>\n\
		Require all granted\n\
	</Directory>\n\
	ErrorDocument 401 /MyDongleCloud/unauthorized.php\n\
	ErrorDocument 403 /MyDongleCloud/denied.php\n\
	ErrorDocument 404 /MyDongleCloud/notpresent.php\n\
</Macro>\n\
<Macro Macro_SSL>\n\
    Include /usr/local/modules/Apache2/options-ssl-apache.conf\n\
    SSLCertificateFile /usr/local/modules/MyDongleCloud/fullchain.pem\n\
    SSLCertificateKeyFile /usr/local/modules/MyDongleCloud/privkey.pem\n\
</Macro>\n\n");
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
			sprintf(sz, "\
<Macro Macro_%s>\n\
	MyDongleCloudModule %s\n\
	RewriteEngine On\n", elModule->string, elModule->string);
			fwrite(sz, strlen(sz), 1, pf);

			cJSON *elEnabled = cJSON_GetObjectItem(elModule, "enabled");
			if (cJSON_HasObjectItem(elModule2, "enabled"))
				elEnabled = cJSON_GetObjectItem(elModule2, "enabled");
			if (cJSON_IsTrue(elEnabled)) {
				cJSON *elAuthorized = cJSON_GetObjectItem(elModule, "authorized");
				if (cJSON_HasObjectItem(elModule2, "authorized") && !cJSON_IsFalse(cJSON_GetObjectItem(elModule, "overwrite")))
					elAuthorized = cJSON_GetObjectItem(elModule2, "authorized");
				char *authorized = cJSON_GetStringValue(elAuthorized);

				if (cJSON_HasObjectItem(elModule, "reverseProxy")) {
					int port = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(elModule, "reverseProxy"));
					sprintf(sz, "\
	ProxyRequests Off\n\
	ProxyPreserveHost on\n\
	ProxyPass / http://localhost:%d/\n\
	ProxyPassReverse / http://localhost:%d/\n\t<Proxy *>\n", port, port);
					fwrite(sz, strlen(sz), 1, pf);
				} else {
					sprintf(sz, "\tDocumentRoot %s\n\t<Directory %s>\n", path, path);
					fwrite(sz, strlen(sz), 1, pf); 
				}
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

				if (cJSON_HasObjectItem(elModule, "reverseProxy"))
					strcpy(sz, "\t</Proxy>\n");
				else
					strcpy(sz, "\t</Directory>\n");
				fwrite(sz, strlen(sz), 1, pf);

				if (strcmp(elModule->string, "Apache2") == 0) {
					for (int ii = 0; ii < cJSON_GetArraySize(modulesDefault); ii++) {
						cJSON *el_Module = cJSON_GetArrayItem(modulesDefault, ii);
						int port_ = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(el_Module, "reverseProxy"));
						if (port_ <= 0)
							port_ = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(el_Module, "fallbackPort"));
						if (port_ > 0) {
							sprintf(sz, "\
	RewriteRule ^/(MyDongleCloud|m)/%s.* http://%s.%s [NC,L]\n\
    RewriteCond %%{HTTP_HOST} ^(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})$\n\
	RewriteRule ^/(MyDongleCloud|m)/%s.* http://%%{HTTP_HOST}:%d [NC,L]\n", el_Module->string, el_Module->string, domain, el_Module->string, port_);
							fwrite(sz, strlen(sz), 1, pf);
						}
					}
					strcpy(sz, "\nAlias /MyDongleCloud /usr/local/modules/Apache2/pages\nAlias /m /usr/local/modules/Apache2/pages\n");
					fwrite(sz, strlen(sz), 1, pf);
				}
				strcpy(sz, "\tUse Macro_Redirect\n");
				fwrite(sz, strlen(sz), 1, pf);
			} else {
				sprintf(sz, "\tRewriteRule ^/.*$ http://%s/MyDongleCloud/disabled.php?m=%s\n\n", domain, elModule->string);
				fwrite(sz, strlen(sz), 1, pf);
			}
			writeLog(elModule->string, pf);
			sprintf(sz, "</Macro>\n");
			fwrite(sz, strlen(sz), 1, pf);

			sprintf(sz, "<VirtualHost *:80>\n");
			fwrite(sz, strlen(sz), 1, pf);
			if (domain != NULL) {
				if (strcmp(elModule->string, "Apache2") == 0)
					sprintf(sz, "\tServerName %s\n", domain);
				else
					sprintf(sz, "\tServerName %s.%s\n", elModule->string, domain);
				fwrite(sz, strlen(sz), 1, pf);
				if (cJSON_HasObjectItem(elModule, "alias")) {
					sprintf(sz, "\tServerAlias %s.%s\n", cJSON_GetStringValue(cJSON_GetObjectItem(elModule, "alias")), domain);
					fwrite(sz, strlen(sz), 1, pf);
				}
			}
			sprintf(sz, "\tUse Macro_%s\n</VirtualHost>\n", elModule->string);
			fwrite(sz, strlen(sz), 1, pf);

			sprintf(sz, "<IfModule mod_ssl.c>\n\t<VirtualHost *:443>\n");
			fwrite(sz, strlen(sz), 1, pf);
			if (domain != NULL) {
				if (strcmp(elModule->string, "Apache2") == 0)
					sprintf(sz, "\t\tServerName %s\n", domain);
				else
					sprintf(sz, "\t\tServerName %s.%s\n", elModule->string, domain);
				fwrite(sz, strlen(sz), 1, pf);
				if (cJSON_HasObjectItem(elModule, "alias")) {
					sprintf(sz, "\t\tServerAlias %s.%s\n", cJSON_GetStringValue(cJSON_GetObjectItem(elModule, "alias")), domain);
					fwrite(sz, strlen(sz), 1, pf);
				}
			}
			sprintf(sz, "\t\tUse Macro_%s\n\t\tUse Macro_SSL\n\t</VirtualHost>\n</IfModule>\n", elModule->string);
			fwrite(sz, strlen(sz), 1, pf);
			int fallbackPort = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(elModule, "fallbackPort"));
			if (fallbackPort > 0) {
				sprintf(sz, "<VirtualHost *:%d>\n", fallbackPort);
				fwrite(sz, strlen(sz), 1, pf);
				if (domain != NULL) {
					sprintf(sz, "\tServerName %s\n", domain);
					fwrite(sz, strlen(sz), 1, pf);
				}
				sprintf(sz, "\tUse Macro_%s\n</VirtualHost>\n", elModule->string);
				fwrite(sz, strlen(sz), 1, pf);
			}
			strcpy(sz, "\n\n");
			fwrite(sz, strlen(sz), 1, pf);
		}
	}
	fclose(pf);
	PRINTF("Apache2: Creation of main.conf\n");
}
