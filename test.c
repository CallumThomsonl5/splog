#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/sploguser.h"

response test_resp(request req) {
    response resp = create_response();

    struct pair *parameters;
    int parameters_count = get_parameters(req, &parameters);

    for (int i = 0; i < parameters_count; i++) {
        printf("key: %s, value: %s\n", parameters[i].key, parameters[i].value);
    }

    append_body(resp, "<h1>test</h1>");
    set_header(resp, "Content-Type", "text/html");

    return resp;
}

response fuck_resp(request req) {
    response resp = create_response();
    printf("req body: %s\n", get_body(req));
    append_body(resp, "fuck was called");
    return resp;
}

response megabyte_resp(request req) {
    response resp = create_response();

    char *buf = malloc(1e6);
    memset(buf, 'F', 1e6-1);
    buf[999999] = '\0';
    append_body_m(resp, buf);

    return resp;
}

response home_resp(request req) {
    response resp = create_response();

    set_header(resp, "Content-Type", "text/html");
    append_body(resp, "<h1>Home page</h1>");

    char *user_agent = get_header(req, "User-Agent");
    if (user_agent) {
        printf("user agent is %s\n", user_agent);
    }

    return resp;
}

response add_resp(request req) {
    response resp = create_response();

    struct pair *parameters;
    int parameters_count = get_parameters(req, &parameters);
    int total = 0;
    for (int i = 0; i < parameters_count; i++) {
        total += atoi(parameters[i].value);
    }

    char buf[10] = {0};
    snprintf(buf, 10, "%d", total);
    append_body(resp, buf);

    return resp;
}

response indexhtml_resp(request req) {
    response resp = create_response();
    append_file_body(resp, "./static/index.html");
    set_header(resp, "Content-Type", "text/html");
    return resp;
}

response favicon_resp(request req) {
    response resp = create_response();
    append_file_body(resp, "./static/favicon.ico");
    set_header(resp, "Content-Type", "image/png");
    set_header(resp, "Cache-Control", "max-age=604800");
    
    return resp;
}

response notfound_resp(request req) {
    response resp = create_response();
    append_body(resp, "cannot get ");
    append_body(resp, get_path(req));
    return resp;
}

int main(void) {
    struct route routes[] = {
        {.path = "/fuck", .method = POST, .fptr = fuck_resp},
        {.path = "/", .method = GET, .fptr = home_resp},
        {.path = "/test", .method = GET, .fptr = test_resp},
        {.path = "/add", .method = GET, .fptr = add_resp},
        {.path = "/megabyte", .method = GET, .fptr = megabyte_resp},
        {.path = "/index.html", .method = GET, .fptr = indexhtml_resp},
        {.path = "/favicon.ico", .method = GET, .fptr = favicon_resp}
    };

    splog_run(routes, notfound_resp);
    return 0;
}
