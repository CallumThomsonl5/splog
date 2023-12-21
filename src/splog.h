#ifndef SPLOG_H
#define SPLOG_H

#include "http.h"

struct route {
    char *path;
    int method;
    struct response* (*fptr)(struct request*);
};

int _splog_run(struct route *routes, int routes_len, struct response* (*notfound_resp)(struct request*));
void splog_free_response(struct response response);
void splog_free_request(struct request request);

#endif
