#ifndef HTTP_H
#define HTTP_H

#include <sys/socket.h>
#include <arpa/inet.h>


#define STATUS_OK_MSG "200 OK"
#define STATUS_NOTFOUND_MSG "404 Not Found"
#define STATUS_BAD_REQUEST_MSG "400 Bad Request"
#define STATUS_INTERNAL_SERVER_ERROR_MSG "500 Internal Server Error"


enum http_request_status {
    HTTP_INVALID_REQUEST,
    HTTP_VALID_REQUEST,
    HTTP_CONTINUE,
    HTTP_DONE,
    HTTP_EMPTY
};

enum method {
    GET,
    POST,
    ERROR_METHOD
};

enum status {
    STATUS_OK = 200,
    STATUS_NOTFOUND = 404,
    STATUS_BAD_REQUEST = 400,
    STATUS_ERROR = 0
};

struct pair {
    char *key;
    char *value;
};

typedef struct request {
    char *path;
    int path_len;

    int method;

    int version;
    int major_version;
    int minor_version;

    struct pair *parameters;
    int parameters_count;
    int parameters_size;

    struct pair *headers;
    int headers_count;
    int headers_size;
    
    char *body;
    int body_len;
    
    char *pos;
} request;

typedef struct response {
    struct pair *headers;
    int headers_count;
    int headers_size;
    char *body;
    int body_len;
    int body_size;
    int status;
} response;

int http_get_tcp_socket(long host, short port);
void pretty_ip(long ip, char *buf);
enum method http_get_method(char *method);
int http_accept_connection(int sock, long *addr);

int http_parse_start_line(struct request *req, char *buf, int len);
int http_parse_headers(struct request *req, char *buf, int len);
int http_parse_body(struct request *req, char *buf, int len);
char *http_get_header_value(struct pair *headers, int headers_count, char *key);
int http_create_response(struct response response, char **output);

#endif
