#include <stdlib.h>
#include <stdio.h>
#include "macro.h"
#include "cJSON.h"
#include "json.h"
#include "modules.h"

//Function
void spaceSetup() {
	cJSON *space = jsonRead(ADMIN_PATH "MyDongleCloud/space.json");
	if (space == NULL) {
		space = cJSON_CreateObject();
		cJSON_AddNumberToObject(space, "nameId", 1);
		cJSON_AddStringToObject(space, "space", "placeholder");
		cJSON_AddStringToObject(space, "alias", "ph");
	}
	char *spaceName = cJSON_GetStringValue(cJSON_GetObjectItem(space, "space"));
	char domain[256];
	sprintf(domain, "%s.%s", spaceName, DOMAIN);
	modulesSetup(domain);
	cJSON_Delete(space);
}
