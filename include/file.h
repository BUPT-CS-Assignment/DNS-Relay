#include "cache.h"
#include "protocol.h"
#include "utils/hash.h"
#include "utils/list.h"
#include <stdlib.h>
#include <string.h>

int file_init(hash *map);
int file_find(DNS_entry *entry, DNS_entry **result, hash *map);
int file_free(hash *map);