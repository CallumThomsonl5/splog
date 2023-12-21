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

// response stuff
void append_body(struct response *resp, const char *body) {
    const int append_length = strlen(body);

    if (!resp->body_size) {
        resp->body_size = 1000;
        resp->body = malloc(resp->body_size);
    }

    if (append_length + resp->body_len >= resp->body_size - 1) {
        resp->body_size = append_length + resp->body_len + 100;
        resp->body = realloc(resp->body, resp->body_size);
        memset(&resp->body[resp->body_len], 0, resp->body_size - resp->body_len);
    }

    memcpy_s(&resp->body[resp->body_len], resp->body_size - resp->body_len, body, append_length);
    resp->body_len += append_length;
}

void append_body_m(struct response *resp, char *body) {
    append_body(resp, body);
    free(body);
}

void set_header(struct response *resp, const char *key, const char *value) {
    const int key_len = strlen(key);
    const int value_len = strlen(value);

    if (!resp->headers_size) {
        resp->headers_size = 6;
        resp->headers = calloc(resp->headers_size, sizeof(struct pair));
    }
    
    if (1 + resp->headers_count > resp->headers_size) {
        resp->headers_size *= 1.5;
        resp->headers = realloc(resp->headers, resp->headers_size * sizeof(struct pair));
        memset(&resp->headers[resp->headers_count], 0, (resp->headers_size - resp->headers_count) * sizeof(struct pair));
    }

    const int buf_len = key_len + value_len + 2;
    char *buf = calloc(buf_len, 1);
    memcpy_s(buf, buf_len, key, key_len + 1);
    memcpy_s(&buf[key_len + 1], buf_len - (key_len  + 1), value, value_len + 1);

    resp->headers[resp->headers_count].key = buf;
    resp->headers[resp->headers_count++].value = &buf[key_len + 1];
}

void set_status(struct response *resp, const int status) {
    resp->status = status;
}

struct response *get_response(void) {
    struct response *resp = calloc(1, sizeof(struct response));
    resp->status = 200;
    set_header(resp, "Server", "Splog/0.1");
    return resp;
}

// request stuff
char *get_path(struct request *req) {
    return req->path;
}

char *get_body(struct request *req) {
    return req->body;
}

char *get_header(struct request *req, char *key) {
    return splog_get_pair(req->headers, req->headers_count, key);
}

int get_headers(struct request *req, struct pair** headers) {
    *headers = req->headers;
    return req->headers_count;
}

char *get_query(struct request *req, char *key) {
    return splog_get_pair(req->queries, req->queries_count, key);
}

int get_queries(struct request *req, struct pair** queries) {
    *queries = req->queries;
    return req->queries_count;
}
