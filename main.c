//
//  stringhash.c
//  A pure C hash of strings
//
//  Created by Sam Bingner on 09/14/2019.
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
#include <sys/time.h>
#include <sys/queue.h>
#include "stringhash.h"

// Example / test code
int main()
{
    stringhash_t hash = stringhash_create(0x10000);
    stringhash_setKey(hash, "test", "has a value");
    char *value = stringhash_copyValueForKey(hash, "test");
    printf("value for hash{test}: \"%s\"\n", value);
    if (value) free(value);
    stringhash_setKey(hash, "test", "has a new value");
    value = stringhash_copyValueForKey(hash, "test");
    printf("value for hash{test}: \"%s\"\n", value);
    if (value) free(value);
    printf("Number of entries in hash: %zu\n", stringhash_count(hash));
    char key[65];
    struct timeval t1,t2;
    double avg = 0;
    size_t i;
    for (i=0; i<0x10000; i++) {
        snprintf(key, 64, "%zu", i);
        gettimeofday(&t1, NULL);
        stringhash_setKey(hash, key, key);
        gettimeofday(&t2, NULL);
        avg = (avg*i + (t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec)) / (i + 1.0);
    }
    printf("Tested %zu sets.  Average time %d ns\n", i, (int)(avg*1000));
    size_t count = stringhash_count(hash);
    if (count != i+1) printf("Error incorrect count\n");
    for (i=0; i<0x10000; i++) {
        snprintf(key, 64, "%zu", i);
        gettimeofday(&t1, NULL);
        char *value = stringhash_copyValueForKey(hash, key);
        gettimeofday(&t2, NULL);
        avg = (avg*i + (t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec)) / (i + 1.0);
        if (!value || strcmp(value, key) != 0) {
           printf("Error unexpected key value for key %s\n", key);
           break;
        }
        free(value);
    }
    printf("Tested %zu retrieves.  Average time %d ns\n", i, (int)(avg*1000));
    printf("Number of entries in hash: %zu\n", stringhash_count(hash));
    value = stringhash_copyValueForKey(hash, "test");
    printf("value for hash{test}: \"%s\"\n", value);
    free(value);
    stringhash_removeKey(hash, "test");
    count--;
    value = stringhash_copyValueForKey(hash, "test");
    printf("value for hash{test}: \"%s\"\n", value);
    char **allKeys = stringhash_copyAllKeys(hash);
    i=0;
    if (allKeys) {
        for (char **key = allKeys; *key; key++) {
            i++;
            char *value = stringhash_copyValueForKey(hash, *key);
            // printf("Key %zu/%zu: \"%s\" = \"%s\"\n", i, count-1, *key, stringhash_copyValueForKey(hash, *key));
            if (!value) {
                printf("Error: unable to retrieve value for key %s (%zu/%zu)\n", *key, i, count);
                return -1;
            }
            free(value);
        }
        free(allKeys);
    }
    printf("Destroying hash\n");
    stringhash_destroy(hash);
    printf("Tests Completed\n");
}
