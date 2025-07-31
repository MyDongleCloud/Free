#ifndef APACHE2_H
#define APACHE2_H

//Global functions
void reloadApache2Conf();
void buildApache2Conf(cJSON *modulesDefault, cJSON *modules);

#endif
