#include <stdlib.h>
#include <stdio.h>
#include "macro.h"
#include "cJSON.h"
#include "json.h"
#include "modules.h"

//Function
void spaceSetup() {
	cJSON *space = jsonRead(ADMIN_PATH "mydonglecloud/space.json");
	modulesSetup(space);
	cJSON_Delete(space);
}
