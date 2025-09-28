#include <stdio.h>
#include <string.h>
#include <db.h>
#include "macro.h"

void dbBerkeleyCreate(const char *pathVal, const char *pathDB) {
	DB *dbp;
	if (db_create(&dbp, NULL, 0) == 0 && dbp->open(dbp, NULL, pathDB, NULL, DB_HASH, DB_CREATE, 0664) == 0) {
		FILE *pf = fopen(pathVal, "r");
		if (pf) {
			char line[1024];
			while (fgets(line, 1024, pf) != 0) {
				line[strcspn(line, "\n")] = '\0';
				char *space_pos = strpbrk(line, " \t");
				if (!space_pos) continue;
				*space_pos = '\0';
				char *key_str = line;
				char *val_str = space_pos + 1;
				DBT key, data;
				memset(&key, 0, sizeof(key));
				key.data = key_str;
				key.size = strlen(key_str) + 1;
				memset(&data, 0, sizeof(data));
				data.data = val_str;
				data.size = strlen(val_str) + 1;
				dbp->put(dbp, NULL, &key, &data, 0);
			}
			fclose(pf);
		}
		dbp->close(dbp, 0);
	}
}
