#ifndef MODULE_H
#define MODULE_H

//Struct
typedef struct {
	const char *jwkPemFile;
	const char *jwkPem;
	const char *name;
	cJSON *permissions;
	int autologin;
} config;

//Global variable
extern module AP_MODULE_DECLARE_DATA mydonglecloud_module;

//Global function
int authorization2(request_rec *r);

#endif
