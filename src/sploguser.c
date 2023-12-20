#include "http.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


char *splog_get_pair(struct pair *pairs, int count, char *key) {
    for (int i = 0; i < count; i++) {
        if (strcmp(key, pairs[i].key) == 0) {
            return pairs[i].value;
        }
    }
    return NULL;
}

void *get_response(void) {
    struct response *resp = calloc(1, sizeof(struct response));
    return (void*)resp;
}