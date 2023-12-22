#ifndef HTTP_H
#define HTTP_H

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define close closesocket
#pragma comment (lib, "Ws2_32.lib")

enum method {
    GET,
    POST,
    ERROR_METHOD
};

enum status {
    STATUS_OK = 200,
    STATUS_NOTFOUND = 404,
    STATUS_ERROR = 0
};

struct pair {
    char *key;
    char *value;
};

typedef struct request {
    char *path;
    struct pair *headers;
    int headers_count;
    struct pair *parameters;
    int parameters_count;
    char *body;
    int method;
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

struct request http_parse_request(char *buf, int len);
char *http_get_header_value(struct pair *headers, int headers_count, char *key);
int http_create_response(struct response response, char **output);

#endif
