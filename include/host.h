#ifndef __HOST_H__
#define __HOST_H__

#include "cache.h"
#include "protocol.h"
#include "utils/hash.h"
#include "main.h"

extern char __HOST_DEST__[255];

int hostInit(hash* map);
int hostQuery(DNS_entry* entry, DNS_entry** result, hash* map,int* flag);
int hostFree(hash* map);

#endif
