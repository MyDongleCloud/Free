#ifndef APACHE2_H
#define APACHE2_H

//Global functions
void buildApache2Conf(cJSON *cloud, cJSON *modulesDefault, cJSON *modules, cJSON *fqdn);
void buildApache2ConfBeforeSetup();

#endif
