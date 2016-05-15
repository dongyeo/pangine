#include <stdio.h>

#define PAG_HTTP_FASTCGI_RESPONDER      1


#define PAG_HTTP_FASTCGI_KEEP_CONN      1

#define PAG_HTTP_FASTCGI_BEGIN_REQUEST  1
#define PAG_HTTP_FASTCGI_ABORT_REQUEST  2
#define PAG_HTTP_FASTCGI_END_REQUEST    3
#define PAG_HTTP_FASTCGI_PARAMS         4
#define PAG_HTTP_FASTCGI_STDIN          5
#define PAG_HTTP_FASTCGI_STDOUT         6
#define PAG_HTTP_FASTCGI_STDERR         7
#define PAG_HTTP_FASTCGI_DATA           8

typedef unsigned char u_char;

typedef struct {
    u_char version;
    u_char type;
    u_char request_id_b1;
    u_char request_id_b0;
    u_char content_length_b1;
    u_char content_length_b0;
    u_char padding_length;
    u_char revserved;
} pangine_http_fast_cgi_header_t;

typedef struct {
    u_char role_b1;
    u_char role_b0;
    u_char flags;
    u_char reserved[5];
} pangine_http_fast_cgi_begin_request_t;

typedef struct {
    u_char app_status_b3;
    u_char app_status_b2;
    u_char app_status_b1;
    u_char app_status_b0;
    u_char protocol_status;
    u_char reserved[3];
} pangine_http_fast_cgi_end_request_t;
typedef struct {
    pangine_http_fast_cgi_header_t header;
    pangine_http_fast_cgi_begin_request_t request;
} pangine_http_fast_cgi_request;
