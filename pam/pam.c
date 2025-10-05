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
#include "common.h"


int authentify(const char *username, const char *password) {
	char buf[1024];
	char post[256];
	snprintf(post, sizeof(post), "{\"username\":\"%s\", \"password\":\"%s\"}", username, password);
	int ret = downloadURLBuffer("http://localhost:8091/MyDongleCloud/Auth/sign-in/username", buf, "Content-Type: application/json", post);
	if (ret == 0) {
			ret = -2;
			cJSON *el = cJSON_Parse(buf);
			cJSON *el2 = cJSON_GetObjectItem(el, "user");
			if (el2) {
					char *username2 = cJSON_GetStringValue2(el2, "username");
					if (strcmp(username, username2) == 0)
							ret = 0;
			}
			cJSON_Delete(el);
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
