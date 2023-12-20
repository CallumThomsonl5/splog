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


struct response splog_handle_request(struct request req, struct route *routes, int routes_len) {
    int found = 0;
    struct response response;
    for (int i = 0; i < routes_len; i++) {
        if (strcmp(routes[i].path, req.path) == 0 && req.method == routes[i].method) {
            found = 1;
            response = routes[i].fptr(req);
            break;
        }
    }

    if (!found) {
        response.headers = NULL;
        response.headers_count = 0;
        response.body = "404 response";
    }

    return response;
}


int _splog_run(struct route *routes, int routes_len) {
    int sock = http_get_tcp_socket(0x7F000001, 4000);
    for (;;) {
        int conn = http_accept_connection(sock, NULL);
        char buf[1001] = {'\0'};
        int len = recv(conn, buf, sizeof(buf)-1, 0);

        struct request request = http_parse_request(buf, len);
        printf("[SPLOG] %d %s\n", request.method, request.path);
        struct response response = splog_handle_request(request, routes, routes_len);

        // debug send
        char status_msg[] = "HTTP/1.1 200 OK\r\n\r\n";
        char *response_buf = calloc(1000, 1);
        strcat(response_buf, status_msg);
        strcat(response_buf, response.body);

        send(conn, response_buf, strlen(response_buf), 0);
        free(response_buf);
        close(conn);
    }
}


// int splog_run(struct route *routes, int routes_len) {
//     int sock = setup_server();
//     while (1) {
//         struct sockaddr_in client_sockadddr;
//         int addrlen = sizeof(struct sockaddr_in);
//         int conn_sock = accept(sock, (struct sockaddr*)&client_sockadddr, 
//             &addrlen);

//         long addr = client_sockadddr.sin_addr.s_addr;
//         char string_addr[16];
//         pretty_ip(ntohl(addr), string_addr);

//         char buf[1001];
//         memset(buf, 0, sizeof(buf));

//         int len = recv(conn_sock, buf, sizeof(buf), 0);
//         buf[len] = '\0';

//         int space_count = 0;
//         char *method = buf;
//         char *path;
//         char *version;
//         for (int i = 0; i < len; i++) {
//             if (buf[i] == ' ') {
//                 space_count++;
//                 if (space_count == 1) {
//                     buf[i] = '\0';
//                     path = buf + i + 1;
//                 } else if (space_count == 2) {
//                     buf[i] = '\0';
//                     version = buf + i + 1;
//                 }
//             }

//             if (buf[i] == '\r') {
//                 buf[i] = '\0';
//                 break;
//             }
//         }

//         printf("[%s] %s %s %s\n", string_addr, method, path, version);
//         struct request request = {
//             .path = path,
//             .headers = NULL,
//             .headers_count = 0,
//             .method = http_get_method(method),
//             .body = "this is a test"
//         };

//         struct response response = splog_handle_request(request, routes, routes_len);
//         if (response.headers_count) {
//             free(response.headers);
//         }

//         char status_msg[] = "HTTP/1.1 200 OK\r\n\r\n";
//         char *response_buf = calloc(1000, 1);
//         strcat(response_buf, status_msg);
//         strcat(response_buf, response.body);

//         send(conn_sock, response_buf, strlen(response_buf), 0);
//         free(response_buf);
//         close(conn_sock);
//     }
// }


// int splog_run(struct route *routes, int routes_len) {  
//     // pretend we parsed this
//     struct request request = {
//         .path = "/fuck",
//         .headers = NULL,
//         .headers_count = 0,
//         .body = "this is a request"
//     };

//     struct response response = splog_handle_request(request, routes, routes_len);
//     splog_print_response(&response);
//     if (response.headers_count) {
//         free(response.headers);
//     }

//     return 0;
// }

// int splog_run(struct route *routes, int routes_len) {
//     while (1) {
//         char path[100];
//         char body[10000];
//         putchar('>'); putchar(' ');
//         scanf("%s %s", path, body);

//         // pretend we parsed this
//         struct request request = {
//             .path = path,
//             .headers = NULL,
//             .headers_count = 0,
//             .body = body
//         };

//         struct response response = splog_handle_request(request, routes, routes_len);
//         splog_print_response(&response);
//     }

//     return 0;
// }