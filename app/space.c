#include <stdlib.h>
#include <stdio.h>
#include "macro.h"
#include "cJSON.h"
#include "json.h"
#include "modules.h"

//Function
void spaceSetup() {
	cJSON *space = jsonRead(ADMIN_PATH "mydonglecloud/space.json");
	if (space == NULL) {
		space = cJSON_CreateObject();
		cJSON_AddStringToObject(space, "name", "placeholder");
	}
	modulesSetup(space);
	cJSON_Delete(space);
}
