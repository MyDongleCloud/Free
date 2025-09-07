#ifndef JSON_H
#define JSON_H

//Global functions
void jsonDump(cJSON *el);
cJSON *jsonRead(char *path);
void jsonWrite(cJSON *el, char *path);
void jsonPrintArray(int tabN, char *before0, char *before1, char *sub, cJSON *el, char *after, FILE *pf);

#endif
