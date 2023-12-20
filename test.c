#include <stdio.h>
#include <stdlib.h>
#include "src/sploguser.h"

response add_resp(request req) {
    int a,b;
    struct response resp;
    if (req.queries_count == 2) {
        a = atoi(req.queries[0].value);
        b = atoi(req.queries[1].value);
        
        char *buf = calloc(16, 1);
        _itoa_s(a+b, buf, 16, 10);
        resp.body = buf;
    } else {
        resp.body = "error not enough args";
    }

    resp.headers_count = 0;
    return resp;
}

response fuck_resp(request req) {
    printf("req body: %s\n", req.body);

    struct response example = {
        .body = "fuck was called",
        .headers = NULL,
        .headers_count = 0
    };

    return example;
}

response home_resp(request req) {
    struct pair *headers = malloc(sizeof(struct pair)*2);
    headers[0].key = "Content-Type";
    headers[0].value = "text/html";

    headers[1].key = "Server";
    headers[1].value = "Splog";

    struct response example = {
        .body = "<h1>Home page</h1>",
        .headers = headers,
        .headers_count = 2
    };

    char *user_agent = get_header(req, "User-Agent");
    if (user_agent) {
        printf("useragent: %s\n", user_agent);
    }

    return example;
}

response test_resp(request req) {
    for (int i = 0; i < req.queries_count; i++) {
        printf("key: %s, value: %s\n", req.queries[i].key, req.queries[i].value);
    }

    struct response resp = {
        .body = "<h1>testmethod response</h1>",
        .headers = NULL,
        .headers_count = 0
    };

    return resp;
}

int main() {
    struct route routes[] = {
        {.path = "/fuck", .method = POST, .fptr = fuck_resp},
        {.path = "/", .method = GET, .fptr = home_resp},
        {.path = "/test", .method = GET, .fptr = test_resp},
        {.path = "/add", .method = GET, .fptr = add_resp},
    };

    splog_run(routes);
    return 0;
}