#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include "cJSON.h"
#include "macro.h"

static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
	memcpy(userdata, ptr, MIN2(nmemb, 1024));
	char *p = (char *)userdata;
	p[MIN2(nmemb, 1024)] = '\0';
	return nmemb;
}

int authentify(const char *username, const char *password) {
	int ret = -2;
	CURL *curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8091/MyDongleCloud/Auth/sign-in/username");
		char post_data[256];
		snprintf(post_data, sizeof(post_data), "{\"username\":\"%s\", \"password\":\"%s\"}", username, password);
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Content-Type: application/json");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(post_data));
		char buf[1024];
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, buf);
		ret = curl_easy_perform(curl);
		curl_slist_free_all(headers);
		//PRINTF("Download (ret:%d) %s\n", ret, buf);
		if (ret == 0) {
				ret = -1;
				cJSON *el = cJSON_Parse(buf);
				cJSON *el2 = cJSON_GetObjectItem(el, "user");
				if (el2) {
						char *username2 = cJSON_GetStringValue2(el2, "username");
						if (strcmp(username, username2) == 0)
								ret = 0;
				}
				cJSON_Delete(el);
		}
		curl_easy_cleanup(curl);
	}
	//PRINTF("authentify %d\n", ret);
	return ret;
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
	const char *username;
	int retval = pam_get_user(pamh, &username, NULL);
	if (retval != PAM_SUCCESS)
		return retval;
	const char *password;
	retval = pam_get_authtok(pamh, PAM_AUTHTOK, &password, NULL);
	if (retval != PAM_SUCCESS)
		return retval;
	if (username && password && strcmp(username, "admin") == 0)
		return authentify(username, password) == 0 ? PAM_SUCCESS : PAM_AUTH_ERR;
	return PAM_AUTH_ERR;
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
	return PAM_SUCCESS;
}
