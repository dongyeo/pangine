#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "cgi.h"
#include "cgiRequestParams.h"
void cgi_request(FILE * fp){
    int sd;
    struct sockaddr_in serv_adr;
    sd = socket(PF_INET,SOCK_STREAM,0);
    
    pangine_http_fast_cgi_header_t response_header;
   
    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_adr.sin_port = htons(9000);
    connect(sd,(struct sockaddr*)&serv_adr,sizeof(serv_adr));
    //pangine_http_fast_cgi_header_t header;
    //memset(&header,0,sizeof(header));
    //write(sd,(void*)&header,sizeof(header));
    //int size = sizeof(pangine_http_fast_cgi_request);
    pangine_http_fast_cgi_request start={
        {
            1,
            PAG_HTTP_FASTCGI_BEGIN_REQUEST,
            0,
            1,
            0,
            sizeof(pangine_http_fast_cgi_begin_request_t),
            0,
            0
        },{
            0,
            PAG_HTTP_FASTCGI_RESPONDER,
            0,
            {0,0,0,0,0}
        }
    };
   //char a[299]="\017\021SCRIPT_FILENAME/var/www/info.php\011\000AUTH_TYPE\021\007GATEWAY_INTERFACECGI/1.1\011\000PATH_INFO\013\013REMOTE_ADDR30.9.192.69\013\000REMOTE_HOST\013\015SERVER_NAME139.129.92.22\013\004SERVER_PORT8888\017\010SERVER_PROTOCOLHTTP/1.1\017\013SERVER_SOFTWAREPANGINE/1.0\014\000QUERY_STRING\016\003REQUEST_METHODGET\014\000CONTENT_TYPE\016\000CONTENT_LENGTH\013\010SCRIPT_NAMEinfo.php";
    
    char * a=(char*) malloc(sizeof(char)*CGI_SMALL_BUF);
    cgi_request_params *params =(cgi_request_params*) malloc(sizeof(cgi_request_params));
    params->file_name="info.php";
    init_cgi_request_params(params);
    int len = generate_request_params_stream(a,params);
    int low = len & 0xff;
    int high = (len >> 8) & 0xff;
    write(sd,(void*)&start,sizeof(start));
    pangine_http_fast_cgi_header_t header={
        1,
        PAG_HTTP_FASTCGI_PARAMS,
        0,
        1,
        high,
        low,
        0,
        0
    };
    write(sd,(void*)&header,sizeof(header));
    write(sd,(void *)a,len);
    header.type=PAG_HTTP_FASTCGI_PARAMS;
    header.content_length_b0=0;
    header.content_length_b1=0;
    write(sd,(void*)&header,sizeof(header));
    header.type=PAG_HTTP_FASTCGI_STDIN;
    header.content_length_b0=0;
    header.content_length_b1=0;
    write(sd,(void*)&header,sizeof(header));
    char buf[1024];
    int length;
    int real_size = 0;
    memset(&response_header,0,sizeof(response_header));
    while((length=read(sd,(char*)&response_header+real_size,sizeof(response_header)-real_size))!=0){
        real_size+=length;
        if(real_size == 8){
            fputs("response type is :",stderr);
            fputs((char*)&(response_header.type),stderr);
            fputs("\n",stderr);
            if(response_header.type == PAG_HTTP_FASTCGI_STDOUT){ 
                int out_size = (response_header.content_length_b1<<8)+response_header.content_length_b0+response_header.padding_length;
                int no_padding_out_size = out_size-response_header.padding_length;
                int reach = 0;
                int real_out_size = 0;
                if(out_size ==0){
                   break;
                }
                while((length=read(sd,buf,(out_size-real_out_size)>1024?1024:out_size-real_out_size))!=0){
                    real_out_size+=length;
                    if(reach == 0){
                        if(real_out_size>no_padding_out_size){
                            reach = 1;
                            fwrite((void*)buf,1,length-real_out_size+no_padding_out_size,fp);
                        }else{
                           fwrite((void*)buf,1,length,fp);
                        }
                    }
                    if(real_out_size==out_size){
                        real_size = 0;
                        break;
                    }
                }
            }
        }
    }
    
  // while((length=read(sd,buf,1024))!=0)
   //     fwrite((void*)buf,1,length,fp);
    close(sd);
}
void init_cgi_request_params(cgi_request_params* params){
    params->remote_host_v="";
    params->auth_type_v = ""; 
    params->path_info_v = "\0";
    params->gateway_interface_v = "CGI/1.1";
    params->remote_addr_v = "139.129.92.22";
    params->server_name_v = "139.129.92.22";
    params->server_port_v = "8888";
    params->server_protocol_v = "HTTP/1.0";
    params->server_software_v = "PANGINE/1.0";
    params->query_string_v = "\0";
    params->request_method_v = "GET";
    params->content_type_v = "text/html";
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
}

