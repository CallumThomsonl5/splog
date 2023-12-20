#ifndef SPLOG_H
#define SPLOG_H

#include "http.h"

struct route {
    char *path;
    int method;
    struct response (*fptr)(struct request);
};

int _splog_run(struct route *routes, int routes_len);
#define splog_run(routes) (_splog_run(routes, sizeof(routes)/sizeof(struct route)))


#endif