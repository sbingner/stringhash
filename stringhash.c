//
//  offsetcache.c
//  An offset cache that can be stored in kernel memory
//
//  Created by Sam Bingner on 03/28/2019.
//  Copyright © 2019 Sam Bingner. All rights reserved.
//

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/queue.h>
#include "stringhash.h"

//-----------------------------------------------------------------------------
// MurmurHash2, 64-bit versions, by Austin Appleby

// The same caveats as 32-bit MurmurHash2 apply here - beware of alignment 
// and endian-ness issues if used across multiple platforms.

// 64-bit hash for 64-bit platforms

uint64_t MurmurHash64A ( const void * key, int len, uint64_t seed )
{
    const uint64_t m = 0xc6a4a7935bd1e995;
    const int r = 47;

    uint64_t h = seed ^ (len * m);

    const uint64_t * data = (const uint64_t *)key;
    const uint64_t * end = data + (len/8);

    while(data != end)
    {
	uint64_t k = *data++;

	k *= m; 
	k ^= k >> r; 
	k *= m; 

	h ^= k;
	h *= m; 
    }

    const unsigned char * data2 = (const unsigned char*)data;

    switch(len & 7)
    {
	case 7: h ^= (uint64_t)(data2[6]) << 48;
	case 6: h ^= (uint64_t)(data2[5]) << 40;
	case 5: h ^= (uint64_t)(data2[4]) << 32;
	case 4: h ^= (uint64_t)(data2[3]) << 24;
	case 3: h ^= (uint64_t)(data2[2]) << 16;
	case 2: h ^= (uint64_t)(data2[1]) << 8;
	case 1: h ^= (uint64_t)(data2[0]);
		h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
} 


static size_t indexOf(stringhash_t hash, const char *key, size_t keylen) {
    return MurmurHash64A(key, strlen(key), 0xcafebabedeadbeef) % hash->size; // Random seed chosen by fair dice roll
}

stringhash_t create_stringhash(size_t size) {
    stringhash_t hash = malloc(sizeof(struct stringhash) + size * sizeof(struct hash_head));
    hash->size = size;
    for (int i=0; i<size; i++) {
        TAILQ_INIT(&hash->heads[i]);
    }
    return hash;
}

void destroy_stringhash(stringhash_t hash) {
    for (int i=0; i<hash->size; i++)  {
        hash_entry_t entry, temp;
        TAILQ_FOREACH_SAFE(entry, &hash->heads[i], entries, temp) {
            TAILQ_REMOVE(&hash->heads[i], entry, entries);
            free(entry);
        }
    }
    free(hash);
}

static hash_entry_t entryForKey(struct hash_head *head, const char *key, size_t keylen) {
    char *value = NULL;
    hash_entry_t entry;
    TAILQ_FOREACH(entry, head, entries) {
        if (keylen == entry->keylen && memcmp(entry->key, key, keylen) == 0) return entry;
    }
    return NULL;
}

const char *stringhash_getKey(stringhash_t hash, const char *key) {
    size_t keylen = strlen(key);
    size_t idx = indexOf(hash, key, keylen);
    hash_entry_t entry = entryForKey(&hash->heads[idx], key, keylen);
    return entry?entry->value:NULL;
}

void stringhash_setKey(stringhash_t hash, const char *key, const char *value) {
    size_t keylen = strlen(key);
    size_t valuelen = strlen(value);
    size_t idx = indexOf(hash, key, keylen);
    hash_entry_t entry = entryForKey(&hash->heads[idx], key, keylen);
    if (entry) {
        TAILQ_REMOVE(&hash->heads[idx], entry, entries);
        if (entry->valuelen < valuelen) {
            hash_entry_t newentry = malloc(sizeof(struct hash_entry) + keylen + valuelen + 2);
            memcpy(newentry, entry, sizeof(struct hash_entry) + keylen + 1);
            free(entry);
            entry = newentry;
        }
    } else {
        entry = malloc(sizeof(struct hash_entry) + keylen + valuelen + 2);
        entry->keylen = keylen;
        memcpy(entry->key, key, keylen+1);
    }
    memcpy(entry->key+keylen+1, value, valuelen+1);
    entry->valuelen = valuelen;
    entry->value = entry->key+keylen+1;
    TAILQ_INSERT_HEAD(&hash->heads[idx], entry, entries);
}

void stringhash_removeKey(stringhash_t hash, const char *key) {
    size_t keylen = strlen(key);
    size_t idx = indexOf(hash, key, keylen);
    hash_entry_t entry = entryForKey(&hash->heads[idx], key, keylen);
    if (entry) {
        TAILQ_REMOVE(&hash->heads[idx], entry, entries);
        free(entry);
    }
}

#ifdef MAIN
// Example / test code
int main()
{
    stringhash_t hash = create_stringhash(65536);
    stringhash_setKey(hash, "test", "has a value");
    printf("%s\n", stringhash_getKey(hash, "test"));
}
#endif
