#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include "cgiRequestStruct.h"
#define CGI_SMALL_BUF 1024
typedef struct cgi_request_params_t{
    char *remote_host_v ;
    char script_file_path_prefix[100];
    char *file_name ;
    char *auth_type_v ;
    char *path_info_v ;
    char *gateway_interface_v;
    char *remote_addr_v;
    char *server_name_v;
    char *server_port_v;
    char *server_protocol_v;
    char *server_software_v;
    char *query_string_v;
    char *request_method_v;
    char *content_type_v;
    char *content_length_v;
}cgi_request_params;

void init_cgi_request_params(cgi_request_params* params);
int generate_request_params_stream(char* out,cgi_request_params* params);
void add(int * len,char * src,char *key,char *value);
void cgi_request(FILE * fp);

/*void init_cgi_request_params(cgi_request_params* params){
    params->remote_host_v="";
    params->auth_type_v = "";
    params->path_info_v = "\0";
    params->gateway_interface_v = "CGI/1.1";
    params->remote_addr_v = "139.129.92.22";
    params->server_name_v = "139.129.92.22";
    params->server_port_v = "8888";
    params->server_protocol_v = "HTTP/1.1";
    params->server_software_v = "PANGINE/1.0";
    params->query_string_v = "\0";
    params->request_method_v = "GET";
    params->content_type_v = "\0";
    params->content_length_v = "\0";
    strcpy(params->script_file_path_prefix,"/var/www/");
}
int generate_request_params_stream(char*out,cgi_request_params* params){
    
    char *script_filename_k = "SCRIPT_FILENAME";
    char *auth_type_k="AUTH_TYPE";
    char *gateway_interface_k="GATEWAY_INTERFACE";
    char *path_info_k = "PATH_INFO";
    char *remote_addr_k = "REMOTE_ADDR";
    char *server_name_k = "SERVER_NAME";
    char *server_port_k = "SERVER_PORT";
    char *server_protocol_k = "SERVER_PROTOCOL";
    char *server_software_k = "SERVER_SOFTWARE";
    char *query_string_k = "QUERY_STRING";
    char *request_method_k = "REQUEST_METHOD";
    char *content_type_k = "CONTENT_TYPE";
    char *content_length_k = "CONTENT_LENGTH";
    char *script_name_k = "SCRIPT_NAME";
    char *remote_host_k = "REMOTE_HOST";
    strcat(params->script_file_path_prefix,params->file_name);
    int len = 0;
    int *length = &len;
    
    add(length,out,remote_host_k,params->remote_host_v);
    add(length,out,script_name_k,params->file_name);
    add(length,out,script_filename_k,params->script_file_path_prefix);
    add(length,out,auth_type_k,params->auth_type_v);
    add(length,out,path_info_k,params->path_info_v);
    add(length,out,gateway_interface_k,params->gateway_interface_v);
    add(length,out,remote_addr_k,params->remote_addr_v);
    add(length,out,server_name_k,params->server_name_v);
    add(length,out,server_port_k,params->server_port_v);
    add(length,out,server_protocol_k,params->server_protocol_v);
    add(length,out,server_software_k,params->server_software_v);
    add(length,out,query_string_k,params->query_string_v);
    add(length,out,request_method_k,params->request_method_v);
    add(length,out,content_type_k,params->content_type_v);
    add(length,out,content_length_k,params->content_length_v);
    return len;
}
void add(int * len,char * src,char * key,char * value){
    int ken_len = strlen(key);
    int value_len = strlen(value);
    sprintf(src+(*len),"%c",ken_len);
    (*len)++;
    if(value_len==0){
        sprintf(src+(*len),"\000");
    }else{
        sprintf(src+(*len),"%c",value_len);
    }
    (*len)++;
    printf("value length %d\n",strlen(value));
    sprintf(src+(*len),"%s%s",key,value);
    (*len)+=(ken_len+value_len);
}*/

