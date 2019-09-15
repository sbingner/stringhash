//
//  offsetcache.h
//  An offset cache that can be stored in kernel memory
//
//  Created by Sam Bingner on 03/28/2019.
//  Copyright © 2019 Sam Bingner. All rights reserved.
//

#ifndef _OFFSETCACHE_H
#define _OFFSETCACHE_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/queue.h>

typedef struct hash_entry {
    TAILQ_ENTRY(hash_entry) entries;
    size_t keylen;
    size_t valuelen;
    char *value;
    char key[];
} *hash_entry_t;

TAILQ_HEAD(hash_head, hash_entry);

typedef struct stringhash {
    size_t size;
    struct hash_head heads[];
} *stringhash_t;

stringhash_t create_stringhash(size_t size);
void destroy_stringhash(stringhash_t hash);
const char *stringhash_getKey(stringhash_t hash, const char *key);
void stringhash_setKey(stringhash_t hash, const char *key, const char *value);
void stringhash_removeKey(stringhash_t hash, const char *key);

void print_cache(void);

#endif
