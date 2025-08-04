#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "macro.h"
#include "cJSON.h"

//Functions
void jsonDump(cJSON *el) {
	char *sz = cJSON_Print(el);
	PRINTF("%s\n", sz);
	free(sz);
}

cJSON *jsonRead(char *path) {
	struct stat statTest;
	if (stat(path, &statTest) != 0 || statTest.st_size == 0)
		return NULL;
	int size = statTest.st_size + 16;
	char *sz = malloc(size);
	FILE *f = fopen(path, "r");
	if (f) {
		fread(sz, size, 1, f);
		fclose(f);
	}
	cJSON *ret = cJSON_Parse(sz);
	free(sz);
	return ret;
}

void jsonWrite(cJSON *el, char *path) {
	FILE *f = fopen(path, "w");
	if (f) {
		char *sz = cJSON_Print(el);
		fwrite(sz, strlen(sz), 1, f);
		fclose(f);
		free(sz);
	}
}
