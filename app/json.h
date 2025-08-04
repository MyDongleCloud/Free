#ifndef JSON_H
#define JSON_H

//Global functions
void jsonDump(cJSON *el);
cJSON *jsonRead(char *path);
void jsonWrite(cJSON *el, char *path);

#endif
