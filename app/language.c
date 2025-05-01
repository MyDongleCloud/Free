#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "settings.h"
#include "language.h"
#include "macro.h"

//Global variable
mylanguage mylanguages[NB_LANG] = {
	{"English", "en"},
	{"French", "fr"},
	{"Spanish", "es"},
	{"Portuguese", "pt"},
	{"German", "de"},
	{"Italian", "it"},
	{"Dutch", "nl"},
	{"Chinese", "zh"},
	{"Japanese", "ja"},
	{"Korean", "kr"},
};

//Private variables
static unsigned char *strings[][NB_LANG] = {
#include "language-device.h"
};
static int stringsSize = sizeof(strings) / (sizeof(unsigned char *) * NB_LANG);

//Functions
static int cmpfunc(const void *a, const void *b) {
	return strcmp(*(unsigned char**)a, *(unsigned char**)b);
}

unsigned char *LL(unsigned char *a, int b) {
	unsigned char **item = (unsigned char **)bsearch(&a, strings, stringsSize, sizeof(unsigned char *) * NB_LANG, cmpfunc);
	if (item == NULL) {
		PRINTF("LANGUAGE: This string doesn't exist %s\n", a);
		return a;
	}
	long unsigned int pos = (item - strings[0]) / NB_LANG;
	return strings[pos][b];
}

unsigned char *L(unsigned char *a) {
	if (sio.language == 0 || sio.language >= NB_LANG)
		return a;
	else
		return LL(a, sio.language);
}

void languageTest() {
	PRINTF("Testing language begin\n");
	PRINTF("Testing language.h for right order\n");
	int i;
	for (i = 0; i < stringsSize - 1; i++)
		if (strcmp(strings[i][0], strings[i + 1][0]) >= 0)
			PRINTF("ERROR at %s\n", strings[i][0]);
	PRINTF("Testing language end\n");
}
