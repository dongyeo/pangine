#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include "threadPool.h"

#define BUF_SIZE 1024
#define SMALL_BUF 100
#define EPOLL_SIZE 50
void set_no_blocking_mode(int fd);
void error_handling(char  *msg);
void *worker(void * arg);
void send_data(FILE* fp,char* content,char* file_name);
void send_error(FILE* fp);
char* content_type(char * file);
int epfd;
int main(int argc,char * argv[]){
    int serv_sock,clnt_sock,option;
    struct sockaddr_in serv_adr,clnt_adr;
    socklen_t opt_len, adr_size;
    int str_len,i;
    char buf[BUF_SIZE];
    
    struct epoll_event *ep_events;
    struct epoll_event event;
    int event_cnt;
    if(argc!=2){
        printf("Usage : %s <port> \n",argv[0]);
        exit(1);
    }
    serv_sock = socket(PF_INET,SOCK_STREAM,0);
    opt_len = sizeof(option);
    option = 1;
    setsockopt(serv_sock,SOL_SOCKET,SO_REUSEADDR,(void*)&option,opt_len);
    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));
    if(bind(serv_sock,(struct sockaddr *)&serv_adr,sizeof(serv_adr))==-1){
       error_handling ("bind() error");
    }
    if(listen(serv_sock,5)==-1){
        error_handling("listen() error");
    }
    
    epfd = epoll_create(EPOLL_SIZE);
    ep_events = malloc(sizeof(struct epoll_event)*EPOLL_SIZE);
    
    set_no_blocking_mode(serv_sock);
    event.events = EPOLLIN;
    event.data.fd = serv_sock;
    epoll_ctl(epfd,EPOLL_CTL_ADD,serv_sock,&event);
    
    if(tpool_create(5) !=0){
        error_handling("tpool_create() error");
    }
    
    while(1){
        event_cnt = epoll_wait(epfd,ep_events,EPOLL_SIZE,-1);
        if(event_cnt == -1){
            error_handling("epoll_wait() error");
            break;
        }
        for(i=0;i<event_cnt;i++){
            if(ep_events[i].data.fd == serv_sock){
                adr_size = sizeof(clnt_adr);
                clnt_sock = accept(serv_sock,(struct sockaddr*)&clnt_adr,&adr_size);
                set_no_blocking_mode(clnt_sock);
                event.events = EPOLLIN|EPOLLET;
                event.data.fd = clnt_sock;
                epoll_ctl(epfd,EPOLL_CTL_ADD,clnt_sock,&event);
            }else{
                // int fd = ep_events[i].data.fd;
                tpool_add_work(worker,(void *)&ep_events[i].data.fd);
            }
        }
    }
    close(serv_sock);
    close(epfd);
    return 0;
}

void set_no_blocking_mode(int fd){
    int flag = fcntl(fd,F_GETFL,0);
    fcntl(fd,F_SETFL,flag|O_NONBLOCK);
}

void error_handling(char *msg){
    fputs(msg,stderr);
    fputc('\n',stderr);
    exit(1);
}
void * worker(void* arg){
    int client = *((int *)arg);
    char req_line[SMALL_BUF];
    FILE * clnt_read;
    FILE * clnt_write;
    char method[10];
    char ct[15];
    char file_name[30];
    
    clnt_read = fdopen(client,"r");
    clnt_write = fdopen(dup(client),"w");
    fgets(req_line,SMALL_BUF,clnt_read);
    if(strstr(req_line,"HTTP/") == NULL){
        send_error(clnt_write);
        fclose(clnt_read);
        fclose(clnt_write);
        return;
    }
    strcpy(method,strtok(req_line," /"));
    strcpy(file_name,strtok(NULL," /"));
    strcpy(ct,content_type(file_name));
    if(strcmp(method,"GET") != 0){
        send_error(clnt_write);
        fclose(clnt_read);
        fclose(clnt_write);
        return ;
    }
    fclose(clnt_read);
    send_data(clnt_write,ct,file_name);
    epoll_ctl(epfd,EPOLL_CTL_DEL,client,NULL);
}

void send_data(FILE * fp,char* ct,char* file_name){
    char protocal[] = "HTTP/1.0 200 OK\r\n";
    char server[] = "Server:Linux Web Server Pangine\r\n";
    char cnt_len[SMALL_BUF];
    char cnt_type[SMALL_BUF];
    char buf[BUF_SIZE];
    FILE* send_file;
    sprintf(cnt_type,"Content-type:%s\r\n\r\n",ct);
    send_file = fopen(file_name,"r");
    if(send_file == NULL){
        send_error(fp);
        return ;
    }
    int prev = ftell(send_file);
    fseek(send_file,0,SEEK_END);
    int sz = ftell(send_file);
    fseek(send_file,0,SEEK_SET);
    sprintf(cnt_len,"Content-length:%d\r\n",sz);
    fputs(protocal,fp);
    fputs(server,fp);
    fputs(cnt_len,fp);
    fputs(cnt_type,fp);
    
    while(fgets(buf,BUF_SIZE,send_file) != NULL){
        fputs(buf,fp);
        fflush(fp);
    }
    fflush(fp);
    fclose(fp);
    fclose(send_file);
}

char * content_type(char * file){
    char extension[SMALL_BUF];
    char file_name[SMALL_BUF];
    strcpy(file_name,file);
    strtok(file_name,".");
    strcpy(extension,strtok(NULL,"."));
    
    if(!strcmp(extension,"html") || ! strcmp(extension,"htm"))
        return "text/html";
    else
        return "text/plain";
}

void send_error(FILE * fp){
    char protocal[]="HTTP/1.0 400 Bad Request\r\n";
    char server[]="Server:Linux Web Server Pangine \r\n";
    char cnt_len[]="Content-length:2048\r\n";
    char cnt_type[]="Content-type:text/html\r\n\r\n";
    char content[]="<html><h1>ERROR!<h1></html>";
    
    fputs(protocal,fp);
    fputs(server,fp);
    fputs(cnt_len,fp);
    fputs(cnt_type,fp);
    fputs(content,fp);
    fflush(fp);
}
