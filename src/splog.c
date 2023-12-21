#include "splog.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "http.h"

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
    if (request.queries_count) {
        free(request.queries);
    }

    if (request.headers_count) {
        free(request.headers);
    }
}


int _splog_run(struct route *routes, int routes_len, struct response* (*notfound_resp)(struct request*)) {
    int sock = http_get_tcp_socket(0x7F000001, 4000);
    for (;;) {
        int conn = http_accept_connection(sock, NULL);
        char buf[1001] = {'\0'};
        int len = recv(conn, buf, sizeof(buf)-1, 0);

        struct request request = http_parse_request(buf, len);
        printf("[SPLOG] %d %s\n", request.method, request.path);
        struct response response = splog_handle_request(request, routes, routes_len, notfound_resp);

        splog_free_request(request);

        char *resp_buf;
        int resp_len = http_create_response(response, &resp_buf);

        send(conn, resp_buf, resp_len, 0);
        free(resp_buf);
        splog_free_response(response);

        close(conn);
    }
}
