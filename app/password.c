#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include "macro.h"

// Struct
typedef struct {
	const char *old_pwd;
	const char *new_pwd;
} pam_password_data;

static int pamConversationCheck(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr) {
	pam_password_data *pwd_data = (pam_password_data *)appdata_ptr;
	*resp = calloc(num_msg, sizeof(struct pam_response));
	if (*resp == NULL)
		return PAM_BUF_ERR;

	for (int i = 0; i < num_msg; i++) {
		if (msg[i]->msg_style == PAM_PROMPT_ECHO_OFF) {
			if (pwd_data && pwd_data->old_pwd) {
				(*resp)[i].resp = strdup(pwd_data->old_pwd);
			} else {
				return PAM_CONV_ERR;
			}
		}
	}
	return PAM_SUCCESS;
}

static int pamConversationChange(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr) {
	pam_password_data *pwd_data = (pam_password_data *)appdata_ptr;
	*resp = calloc(num_msg, sizeof(struct pam_response));
	if (*resp == NULL)
		return PAM_BUF_ERR;
	for (int i = 0; i < num_msg; i++) {
		if (msg[i]->msg_style == PAM_PROMPT_ECHO_OFF) {
			if (strstr(msg[i]->msg, "Current password")) {
				if (pwd_data && pwd_data->old_pwd) {
					(*resp)[i].resp = strdup(pwd_data->old_pwd);
				} else {
					return PAM_CONV_ERR;
				}
			} else if (strstr(msg[i]->msg, "New password") || strstr(msg[i]->msg, "new password")) {
				if (pwd_data && pwd_data->new_pwd) {
					(*resp)[i].resp = strdup(pwd_data->new_pwd);
				} else {
					return PAM_CONV_ERR;
				}
			}
		}
	}
	return PAM_SUCCESS;
}

int passwordCheck(char *username, char *pwd) {
	pam_handle_t *pamh = NULL;
	int pam_err;
	pam_password_data pwd_data = { .old_pwd = pwd, .new_pwd = NULL };
	struct pam_conv conv = {
		pamConversationCheck,
		&pwd_data
	};
	pam_err = pam_start("passwd", username, &conv, &pamh);
	if (pam_err != PAM_SUCCESS) {
		PRINTF("Password: Error start %s\n", pam_strerror(pamh, pam_err));
		return -1;
	}
	pam_err = pam_authenticate(pamh, PAM_SILENT);
	pam_end(pamh, pam_err);
	if (pam_err == PAM_SUCCESS)
		return 0;
	else {
		PRINTF("Password: Error authenticate %s\n", pam_strerror(pamh, pam_err));
		return -1;
	}
}

// Corrected passwordChange function
int passwordChange(char *username, char *pwdOld, char *pwdNew) {
	pam_handle_t *pamh = NULL;
	int pam_err;
	pam_password_data pwd_data = { .old_pwd = pwdOld, .new_pwd = pwdNew };
	struct pam_conv conv = { pamConversationChange, &pwd_data };
	pam_err = pam_start("passwd", username, &conv, &pamh);
	if (pam_err != PAM_SUCCESS) {
		PRINTF("Password: Error start %s\n", pam_strerror(pamh, pam_err));
		return -1;
	}
	pam_err = pam_chauthtok(pamh, PAM_CHANGE_EXPIRED_AUTHTOK);
	if (pam_err != PAM_SUCCESS) {
		PRINTF("Password: Error change %s\n", pam_strerror(pamh, pam_err));
		pam_end(pamh, pam_err);
		return -1;
	}
	pam_end(pamh, PAM_SUCCESS);
	return 0;
}
