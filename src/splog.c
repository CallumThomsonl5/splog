#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <pthread.h>

#include "splog.h"
#include "http.h"

struct response err_response = {
    .body = "invalid request",
    .body_len = 15,
    .body_size = 0,
    .status = 400,
    .headers = NULL,
    .headers_count = 0
};

void splog_print_response(struct response *resp) {
    puts("response:");
    
    if (resp->headers_count) {
        for (int i = 0; i < resp->headers_count; i++) {
            printf("%s: %s\n", resp->headers[i].key, resp->headers[i].value);
        }
        puts("");
    }

    if (resp->body) {
        printf("body: %s\n", resp->body);
    }
}


struct response splog_handle_request(struct request req, struct route *routes, int routes_len, struct response* (*notfound_resp)(struct request*)) {
    int found = 0;
    struct response response;
    for (int i = 0; i < routes_len; i++) {
        if (strcmp(routes[i].path, req.path) == 0 && req.method == routes[i].method) {
            found = 1;
            struct response *response_ptr = routes[i].fptr(&req);
            response = *response_ptr;
            free(response_ptr);
            break;
        }
    }

    if (!found) {
        struct response *response_ptr = notfound_resp(&req);
        response = *response_ptr;
        response.status = 404;
        free(response_ptr);
    }

    return response;
}


void splog_free_response(struct response response) {
    if (response.headers_size) {
        for (int i = 0; i < response.headers_count; i++) {
            free(response.headers[i].key);
        }

        free(response.headers);
    }

    if (response.body_size) {
        free(response.body);
    }
}


void splog_free_request(struct request request) {
    if (request.parameters_count) {
        free(request.parameters);
    }

    if (request.headers_count) {
        free(request.headers);
    }
}

struct handle_request_thread_args {
    int conn;
    struct route *routes;
    int routes_len;
    struct response* (*notfound_resp)(struct request*);
};

void *splog_handle_connection_thread(void *arg) {
    char buf[1001];
    memset(buf, 0, 1001);

    struct handle_request_thread_args vars = *((struct handle_request_thread_args*)arg);
    int conn = vars.conn;
    struct route *routes = vars.routes;
    int routes_len = vars.routes_len;
    struct response* (*notfound_resp)(struct request*) = vars.notfound_resp;

    for (;;) {
        int len = recv(conn, buf, sizeof(buf)-1, 0);

        if (len <= 0) return 0;

        struct request request = {0};
        struct response response;
        char *content_length;
        
        int result = http_parse_start_line(&request, buf, len);
        if (result == HTTP_CONTINUE) {
            result = http_parse_headers(&request, buf, len);
            if (result == HTTP_INVALID_REQUEST) {
                response = err_response;
            } else {
                content_length = http_get_header_value(request.headers, request.headers_count, "Content-Length");
                if (content_length && request.method == POST) {
                    result = http_parse_body(&request, buf, len);
                    if (result == HTTP_INVALID_REQUEST) {
                        response = err_response;
                    } else {
                        response = splog_handle_request(request, routes, routes_len, notfound_resp);
                    }
                } else {
                    response = splog_handle_request(request, routes, routes_len, notfound_resp);
                }
            }
        } else if (result == HTTP_INVALID_REQUEST) {
            response = err_response;
        } else {
            response = splog_handle_request(request, routes, routes_len, notfound_resp);
        }        

        printf("[SPLOG] %d %s\n", request.method, request.path);
        
        splog_free_request(request);

        char *resp_buf;
        size_t resp_len = http_create_response(response, &resp_buf);

        send(conn, resp_buf, resp_len, 0);
        free(resp_buf);
        splog_free_response(response);
    }

    return 0;
}


int _splog_run(struct route *routes, int routes_len, struct response* (*notfound_resp)(struct request*)) {
    int sock = http_get_tcp_socket(0x0, 4000);

    if (sock < 0) {
        puts("failed to get socket");
        exit(1);
    }

    for (;;) {
        int conn = http_accept_connection(sock, NULL);
        puts("connection accepted");

        struct handle_request_thread_args args = {
            .conn = conn,
            .routes = routes,
            .routes_len = routes_len,
            .notfound_resp = notfound_resp
        };

        pthread_t t = 0;
        pthread_create(&t, NULL, splog_handle_connection_thread, (void*)&args);
    }
}
