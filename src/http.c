#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "http.h"

#define close closesocket

void pretty_print_ip(long ip) {
    unsigned char one = ip >> 24;
    unsigned char two = ip >> 16;
    unsigned char three = ip >> 8;
    unsigned char four = ip;

    printf("%d.%d.%d.%d\n", one, two, three, four);
}

void pretty_ip(long ip, char *buf) {
    unsigned char one = ip >> 24;
    unsigned char two = ip >> 16;
    unsigned char three = ip >> 8;
    unsigned char four = ip;

    snprintf(buf, 16, "%d.%d.%d.%d", one, two, three, four);
}

enum method http_get_method(char *method) {
    if (strcmp(method, "GET") == 0) {
        return GET;
    } else if (strcmp(method, "POST") == 0){
        return POST;
    } else {
        return ERROR_METHOD;
    }
}

int http_get_tcp_socket(long host, short port) {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in mysockaddr;
    mysockaddr.sin_family = AF_INET;
    mysockaddr.sin_port = htons(port);
    mysockaddr.sin_addr.s_addr = htonl(host);

    int bindresult = bind(sock, (struct sockaddr*)(&mysockaddr), sizeof(mysockaddr));
    int listenresult = listen(sock, 5);
    return sock;
}


int http_accept_connection(int sock, long *addr) {
    struct sockaddr_in client_sockadddr;
    int addrlen = sizeof(struct sockaddr_in);
    int conn_sock = accept(sock, (struct sockaddr*)&client_sockadddr, 
        &addrlen);
    if (addr) *addr = client_sockadddr.sin_addr.s_addr;
    return conn_sock;
}


char *http_get_header_value(struct pair *headers, int headers_count, char *key) {
    for (int i = 0; i < headers_count; i++) {
        if (strcmp(key, headers[i].key) == 0) {
            return headers[i].value;
        }
    }

    return NULL;
}


struct request http_parse_request(char *buf, int len) {
    struct request request;
    memset(&request, 0, sizeof(struct request));

    // get method, path and version
    char *method = buf;
    char *path;
    char *version;
    int i;
    int sc = 0;
    for (i = 0; i < len; i++) {
        if (buf[i] == ' ') {
            sc++;
            if (sc == 1) {
                buf[i] = '\0';
                path = buf + i + 1;
            } else if (sc == 2) {
                buf[i] = '\0';
                version = buf + i + 1;
            }
        }

        if (buf[i] == '\r') {
            buf[i] = '\0';
            break;
        }
    }

    request.path = path;
    if (http_get_method(method) == ERROR_METHOD) return request;
    request.method = http_get_method(method);

    // parse query string
    int query_start = 0;
    for (int j = 0; j < strnlen_s(path, 1000); j++) {
        if (path[j] == '?') {
            query_start = j;
            break;
        }
    }

    if (query_start) {
        struct pair *queries = malloc(sizeof(struct pair));
        int query_size = 1;
        int query_count = 0;
        int in_key = 1;
        int pathlen = strnlen_s(path, 1000);
        for (int j = query_start + 1; j < pathlen; j++) {
            if (query_count == query_size) {
                query_size += 2;
                queries = realloc(queries, sizeof(struct pair) * query_size);
            }

            if (in_key && (query_count == 0 || path[j-1] == '&')) {
                queries[query_count].key = path + j;
                in_key = 0;
                path[j-1] = '\0';
            }

            if (path[j] == '=') {
                path[j] = '\0';
                queries[query_count++].value = path + 1 + j;
                in_key = 1;
            }
        }

        request.queries = queries;
        request.queries_count = query_count;
    }
    

    // parse headers
    if (buf[i+2] == '\r') return request;
    struct pair *headers = calloc(5, sizeof(struct pair));
    int headers_count = 0;
    int headers_size = 5;
    int in_key = 1;
    for (i = i + 2; i < len; i++) {
        if (headers_count == headers_size) {
            headers_size+=2;
            headers = realloc(headers, headers_size*sizeof(struct pair));
        }

        if (buf[i] == ':' && buf[i+1] == ' ' && in_key) {
            in_key = 0;
            buf[i] = '\0';
        } else if (in_key && buf[i-1] == '\n') {
            buf[i-2] = '\0';
            headers[headers_count].key = buf + i;
        } else if (!in_key && buf[i] == ' ') {
            buf[i] = '\0';
            headers[headers_count++].value = buf + i + 1;
            in_key = 1;
        }

        if (buf[i] == '\r' && buf[i+1] == '\n' && buf[i+2] == '\r' && buf[i+3] == '\n') {
            break;
        }
    }

    request.headers = headers_count ? headers : NULL;
    request.headers_count = headers_count;

    // parse body
    if (request.method == POST) {
        char *content_length = http_get_header_value(headers, headers_count, "Content-Length");
        if (content_length) {
            int content_length_int = atoi(content_length);
            buf[i + 4 + content_length_int] = '\0';
            request.body = buf + i + 4;
        }
    }

    return request;
}