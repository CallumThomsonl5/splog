#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

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


int http_get_tcp_socket(long host, short port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, 4);
    if (sock == -1) return -1;

    struct sockaddr_in mysockaddr;
    mysockaddr.sin_family = AF_INET;
    mysockaddr.sin_port = htons(port);
    mysockaddr.sin_addr.s_addr = htonl(host);

    if (bind(sock, (struct sockaddr*)(&mysockaddr), sizeof(mysockaddr)) != 0) return -1;
    if (listen(sock, 5) != 0) return -1;

    return sock;
}


int http_accept_connection(int sock, long *addr) {
    struct sockaddr_in client_sockadddr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
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


void http_req_add_para(struct request *req, char *key, char *value) {
    if (!req->parameters_size) {
        req->parameters_size = 1;
        req->parameters = calloc(req->parameters_size, sizeof(struct pair));
    }

    if (req->parameters_count + 1 > req->parameters_size) {
        req->parameters_size += 2;
        req->parameters = realloc(req->parameters, req->parameters_size * sizeof(struct pair));
    }

    req->parameters[req->parameters_count].key = key;
    req->parameters[req->parameters_count++].value = value;
}


void http_req_add_header(struct request *req, char *key, char *value) {
    if (!req->headers_size) {
        req->headers_size = 15;
        req->headers = calloc(req->headers_size, sizeof(struct pair));
    }

    if (req->headers_count + 1 > req->headers_size) {
        req->headers_size += 2;
        req->headers = realloc(req->headers, req->headers_size * sizeof(struct pair));
    }

    req->headers[req->headers_count].key = key;
    req->headers[req->headers_count++].value = value;
}


int http_parse_start_line(struct request *req, char *buf, int len) {
    enum {
        sl_start,
        sl_method,
        sl_before_path,
        sl_path,
        sl_para_start,
        sl_para_key,
        sl_para_value_start,
        sl_para_value,
        sl_scheme_H,
        sl_scheme_HT,
        sl_scheme_HTT,
        sl_scheme_HTTP,
        sl_version_slash,
        sl_version_start,
        sl_version,
        sl_almost_done,
        sl_done,
    } state;

    state = sl_start;
    char *i, c;
    char *method_start;

    char *para_key_start = 0;
    char *para_value_start = 0;

    for (i = buf; i < (buf + len); i++) {
        c = *i;

        switch (state) {
            case sl_start:
                if (c < 'A' || c > 'Z') return HTTP_INVALID_REQUEST;
                method_start = i;
                state = sl_method;
                break;
            case sl_method:
                if (c == ' ') {
                    state = sl_before_path;
                    *i = '\0';

                    if (strncmp(method_start, "GET", 3) == 0) {
                        req->method = GET;
                    } else if (strncmp(method_start, "POST", 4) == 0) {
                        req->method = POST;
                    } else {
                        return HTTP_INVALID_REQUEST;
                    }

                } else if ((c < 'A' || c > 'Z')) return HTTP_INVALID_REQUEST;
                break;
            case sl_before_path:
                if (c != '/') return HTTP_INVALID_REQUEST;
                state = sl_path;
                req->path = i;
                break;
            case sl_path:
                if (c == ' ') {
                    req->path_len = i - req->path;
                    *i = '\0';
                    state = sl_scheme_H;
                } else if (c == '?') {
                    req->path_len = i - req->path;
                    *i = '\0';
                    state = sl_para_start;
                } else if ( !((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '-'
                        || c == '_' || c == '.' || c == '/') ) return HTTP_INVALID_REQUEST;
                break;
            case sl_para_start:
                if (c == ' ') {
                    state = sl_scheme_H;
                } else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c == '-') {
                    state = sl_para_key;
                    para_key_start = i;
                } else return HTTP_INVALID_REQUEST;
                break;
            case sl_para_key:
                if (c == '=') {
                    state = sl_para_value_start;
                    *i = '\0';
                } else if (c == '&') {
                    state = sl_para_start;
                    *i = '\0';
                    http_req_add_para(req, para_key_start, NULL);
                } else if (c == ' ') {
                    state = sl_scheme_H;
                    *i = '\0';
                    http_req_add_para(req, para_key_start, NULL);
                } else if ( !((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '-' || c == '_') )
                    return HTTP_INVALID_REQUEST;
                break;
            case sl_para_value_start:
                if (c == ' ') {
                    state = sl_scheme_H;
                    *i = '\0';
                    http_req_add_para(req, para_key_start, NULL);
                } else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_') {
                    state = sl_para_value;
                    para_value_start = i;
                } else return HTTP_INVALID_REQUEST;
                break;
            case sl_para_value:
                if (c == ' ') {
                    state = sl_scheme_H;
                    *i = '\0';
                    http_req_add_para(req, para_key_start, para_value_start);
                } else if (c == '&') {
                    state = sl_para_start;
                    *i = '\0';
                    http_req_add_para(req, para_key_start, para_value_start);
                } else if ( !((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '-' || c == '_' || (c >= '0' && c <= '9')) )
                    return HTTP_INVALID_REQUEST;
                break;
            case sl_scheme_H:
                if (c != 'H') return HTTP_INVALID_REQUEST;
                state = sl_scheme_HT;
                break;
            case sl_scheme_HT:
                if (c != 'T') return HTTP_INVALID_REQUEST;
                state = sl_scheme_HTT;
                break;
            case sl_scheme_HTT:
                if (c != 'T') return HTTP_INVALID_REQUEST;
                state = sl_scheme_HTTP;
                break;
            case sl_scheme_HTTP:
                if (c != 'P') return HTTP_INVALID_REQUEST;
                state = sl_version_slash;
                break;
            case sl_version_slash:
                if (c != '/') return HTTP_INVALID_REQUEST;
                state = sl_version_start;
                break;
            case sl_version_start:
                if (c < '1' || c > '3') return HTTP_INVALID_REQUEST;
                req->major_version = c - '0';
                state = sl_version;
                break;
            case sl_version:
                if (c == '.') break;
                if (c == '0' || c == '1') {
                    req->minor_version = c - '0';
                    req->version = req->major_version * 1000 + req->minor_version;
                    state = sl_almost_done;
                } else
                    return HTTP_INVALID_REQUEST;
                break;
            case sl_almost_done:
                if (c != '\r') return HTTP_INVALID_REQUEST;
                state = sl_done;
                break;
            case sl_done:
                if (c != '\n') return HTTP_INVALID_REQUEST;
                goto done;
                break;
            default:
                return HTTP_INVALID_REQUEST;
                break;
        }
    }

    if (state != sl_done) return HTTP_INVALID_REQUEST;

    done:
    if (i + 2 < buf + len) {
        req->pos = i + 2;
        if (*(i+1) == '\r' && *(i+2) == '\n') return HTTP_DONE;
        else if (*(i+1) == '\r') return HTTP_INVALID_REQUEST;
        req->pos = i + 1;
        return HTTP_CONTINUE;
    }
    
    return HTTP_INVALID_REQUEST;
}


int http_parse_headers(struct request *req, char *buf, int len) {
    enum {
        h_start,
        h_start_2nd,
        h_key,
        h_before_value,
        h_value_start,
        h_value,
        h_lf,
        h_done
    } state;

    state = h_start;
    char *i, c;

    char *key_start;
    char *value_start;

    for (i = req->pos; i < buf + len; i++) {
        c = *i;
        
        switch (state) {
            case h_start:
                if ( !((c >= 'A'&& c <= 'Z') || (c >= 'a' && c <= 'z') || c == '-' || c == '_') ) {
                    return HTTP_INVALID_REQUEST;
                }
                state = h_key;
                key_start = i;
                break;
            case h_key:
                if (c == ':') {
                    state = h_before_value;
                    *i = '\0';
                } else if ( !((c >= 'A'&& c <= 'Z') || (c >= 'a' && c <= 'z') || c == '-' || c == '_') )
                    return HTTP_INVALID_REQUEST;
                break;
            case h_before_value:
                if (c != ' ') return HTTP_INVALID_REQUEST;
                state = h_value_start;
                break;
            case h_value_start:
                if (c == '\n' || c == '\r') return HTTP_INVALID_REQUEST;
                value_start = i;
                state = h_value;
                break;
            case h_value:
                if (c == '\r') {
                    state = h_lf;
                    *i = '\0';
                    http_req_add_header(req, key_start, value_start);
                } else if (c == '\n') return HTTP_INVALID_REQUEST;
                break;
            case h_lf:
                if (c != '\n') return HTTP_INVALID_REQUEST;
                state = h_start_2nd;
                break;
            case h_start_2nd:
                if (c == '\r') {
                    state = h_done;
                } else if ( !((c >= 'A'&& c <= 'Z') || (c >= 'a' && c <= 'z') || c == '-' || c == '_') ) {
                    return HTTP_INVALID_REQUEST;
                } else {
                    state = h_key;
                    key_start = i;
                }
                break;
            case h_done:
                if (c != '\n') return HTTP_INVALID_REQUEST;
                goto done;
                break;
        }
    }

    if (state != h_done) return HTTP_INVALID_REQUEST;

    done:
    req->pos = i + 1;
    return HTTP_DONE;
}


int http_parse_body(struct request *req, char *buf, int len) {
    char *content_length_buf = http_get_header_value(req->headers, req->headers_count, "Content-Length");

    if (content_length_buf == NULL) return HTTP_EMPTY;

    for (char *i = content_length_buf; *i != '\0'; i++) {
        if (*i < '0' || *i > '9') return HTTP_INVALID_REQUEST;
    }

    int content_length = atoi(content_length_buf);
    req->body = req->pos;
    char *body_end = &req->pos[content_length];
    if (body_end <= buf + len) {
        *body_end = '\0';
        req->body_len = content_length;
    } else {
        return HTTP_INVALID_REQUEST;
    }
    return HTTP_DONE;
}


void http_get_status(int status, char *msg) {
    // msg should be 50 bytes
    switch (status) {
        case STATUS_OK:
            strcpy(msg, STATUS_OK_MSG "\r\n");
            break;
        case STATUS_NOTFOUND:
            strcpy(msg, STATUS_NOTFOUND_MSG "\r\n");
            break;
        case STATUS_BAD_REQUEST:
            strcpy(msg, STATUS_BAD_REQUEST_MSG "\r\n");
            break;
        default:
            strcpy(msg, STATUS_INTERNAL_SERVER_ERROR_MSG "\r\n");
            break;
    }
}


int http_create_response(struct response response, char **output) {
    size_t buf_size = 100;
    char *buf = malloc(buf_size);
    size_t pos = 0;

    // add status
    memcpy(buf, "HTTP/1.1 ", 9);
    pos = 9;
    char msg[50] = {0};
    http_get_status(response.status, msg);
    memcpy(&buf[pos], msg, strlen(msg));
    pos += strlen(msg);

    // add headers
    if (40 + pos > buf_size) {
        buf_size += 40 + pos + 10;
        buf = realloc(buf, buf_size);
    }

    memcpy(&buf[pos], "Content-Length: ", 16);
    pos += 16;
    char cl_buf[10] = {0};
    snprintf(cl_buf, 10, "%zu", response.body_len);
    memcpy(&buf[pos], cl_buf, strlen(cl_buf));
    pos += strlen(cl_buf);
    memcpy(&buf[pos], "\r\n", 2);
    pos += 2;

    for (int i = 0; i < response.headers_count; i++) {
        int key_len =  strlen(response.headers[i].key);
        int value_len = strlen(response.headers[i].value);

        if (key_len + value_len + 4 + pos > buf_size) {
            buf_size += key_len + value_len + 4 + pos + 10;
            buf = realloc(buf, buf_size);
        }

        memcpy(&buf[pos], response.headers[i].key, key_len);
        pos += key_len;
        memcpy(&buf[pos], ": ", 2);
        pos += 2;
        memcpy(&buf[pos], response.headers[i].value, value_len);
        pos += value_len;
        memcpy(&buf[pos], "\r\n", 2);
        pos += 2;
    }

    memcpy(&buf[pos], "\r\n", 2);
    pos += 2;

    // add body
    if (pos + response.body_len > buf_size) {
        buf_size = pos + response.body_len + 1;
        buf = realloc(buf, buf_size);
    }

    memcpy(&buf[pos], response.body, response.body_len);

    *output = buf;
    return pos + response.body_len;
}
