#ifndef APACHE2_H
#define APACHE2_H

//Global functions
void buildApache2Conf(cJSON *modulesDefault, cJSON *modules, cJSON *space, cJSON *fqdn, int webViaFrp);

#endif
