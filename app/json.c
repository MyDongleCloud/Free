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
	char *sz = malloc(size + 1);
	strcpy(sz, "");
	FILE *f = fopen(path, "r");
	if (f) {
		int ret2 = fread(sz, 1, size, f);
		sz[ret2] = '\0';
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

void jsonPrintArray(int tabN, char *before0, char *before1, char *sub, cJSON *el, char *after, FILE *pf) {
	cJSON *s;
	int count = 0;
	cJSON_ArrayForEach(s, el) {
		char sz[256];
		snprintf(sz, sizeof(sz), "%s%s%s%s%s%s", tabN == 2 ? "\t\t" : tabN == 1 ? "\t" : "", count++ == 0 ? before0 : before1, sub, strlen(sub) == 0 ? "" : ".", s->valuestring, after);
		fwrite(sz, strlen(sz), 1, pf);
	}
}
