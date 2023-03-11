#ifndef HASHMAP_UTILS_H
#define HASHMAP_UTILS_H

#include "structs.h"

HashMap_t* hashmap_init(int size);
void hashmap_clean(HashMap_t *hm);
void hashmap_insert(HashMap_t *hm, int key, char* value);
char* hashmap_get(HashMap_t *hm, int key);
void hashmap_delete(HashMap_t *hm, int key);
void hashmap_print(HashMap_t *hm);

#endif
