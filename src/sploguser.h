#ifndef SPLOG_USER_H
#define SPLOG_USER_H

struct pair {
    char *key;
    char *value;
};

typedef struct request {
    char *path;
    struct pair *headers;
    int headers_count;
    struct pair *queries;
    int queries_count;
    char *body;
    int method;
} request;

typedef struct response {
    struct pair *headers;
    int headers_count;
    char *body;
    int status;
} response;
// typedef void* request;
// typedef void* response;

enum method {
    GET,
    POST,
    ERROR_METHOD
};

struct route {
    char *path;
    int method;
    struct response (*fptr)(struct request);
};

int _splog_run(struct route *routes, int routes_len);
#define splog_run(routes) (_splog_run(routes, sizeof(routes)/sizeof(struct route)))

char *splog_get_pair(struct pair *pairs, int count, char *key);
#define get_header(req, key) (splog_get_pair(req.headers, req.headers_count, key))
#define get_query(req, key) (splog_get_pair(req.queries, req.queries_count, key))

void *get_response(void);

#endif