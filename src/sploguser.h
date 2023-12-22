#ifndef SPLOG_USER_H
#define SPLOG_USER_H

struct pair {
    char *key;
    char *value;
};

typedef void* request;
typedef void* response;

enum method {
    GET,
    POST,
    ERROR_METHOD
};

struct route {
    char *path;
    int method;
    request (*fptr)(response);
};

int _splog_run(struct route *routes, int routes_len, request (*notfound_resp)(response));
#define splog_run(routes, notfound_resp) (_splog_run(routes, sizeof(routes)/sizeof(struct route), notfound_resp))

char *splog_get_pair(struct pair *pairs, int count, char *key);

response get_response(void);
void append_body(response resp, char *body);
void append_body_m(response resp, char *body);
void set_header(response resp, char *key, char *value);
void set_status(response resp, int status);

char *get_path(request req);
char *get_body(request req);
char *get_header(request req, char *key);
char *get_parameter(request req, char *key);
int get_headers(request req, struct pair** headers);
int get_parameters(request req, struct pair** parameters);
#endif
