#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/resource.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include "threadPool.h"
#include "mySignal.h"
#define BUF_SIZE 1024
#define SMALL_BUF 100
#define EPOLL_SIZE 50
#define LOCKFILE "/var/run/pangine.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

int already_running(void);
void set_no_blocking_mode(int fd);
void error_handling(char  *msg);
void *worker(void * arg);
void send_data(FILE* fp,char* content,char* file_name);
void send_error(FILE* fp);
void daemonize(const char* cmd);
int lockfile(int fd);
char* content_type(char * file);
int epfd;
int main(int argc,char * argv[]){
    char * cmd;
    if((cmd = strrchr(argv[0],'/')) == NULL)
        cmd = argv[0];
    else
        cmd++;
    daemonize(cmd);
    if(already_running()){
        syslog(LOG_ERR,"daemon already is running");
        exit(1);
    }

    reg_sig();
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
    
    if(tpool_create(5) !=0){
        error_handling("tpool_create() error");
    }
    
    while(1){
        adr_size = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock,(struct sockaddr*)&clnt_adr,&adr_size);
        tpool_add_work(worker,(void*)&clnt_sock);
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
    close(client);
    send_data(clnt_write,ct,file_name);
    // epoll_ctl(epfd,EPOLL_CTL_DEL,client,NULL);
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
    fclose(fp);
}
void daemonize(const char* cmd){
   
    int i,fd0,fd1,fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;
    umask(0);
    if(getrlimit(RLIMIT_NOFILE,&rl)<0){
        error_handling("can't get file limit");
    }
    if((pid = fork())<0){
        error_handling("can't fork");
    }else if(pid !=0){
        exit(0);
    }
    setsid();
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGHUP,&sa,NULL) <0){
        error_handling("can't ignore SIGHUP");
    }
    if((pid = fork())<0){
        error_handling("can't fork");
    }else if(pid !=0){
        exit(0);
    }
    
    if(chdir("/")<0){
        error_handling("can't change directory to /");
    }
    
    if(rl.rlim_max==RLIM_INFINITY)
        rl.rlim_max = 1024;
    for(i = 0;i < rl.rlim_max;i++){
        close(i);
    }
    
    fd0 = open("/dev/null",O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);
    openlog(cmd,LOG_CONS,LOG_DAEMON);
    if(fd0 != 0 || fd1 != 1 || fd2 !=2){
        syslog(LOG_ERR,"unexpected file description %d %d %d",fd0,fd1,fd2);
        exit(1);
    }
}
int already_running(void){
    int fd;
    char buf[16];
    
    fd = open(LOCKFILE,O_RDWR|O_CREAT,LOCKMODE);
    if(fd < 0){
        syslog(LOG_ERR,"can't open %s:%s",LOCKFILE,strerror(errno));
        exit(1);
    }
    if(lockfile(fd) < 0){
        if(errno == EACCES || errno == EAGAIN){
            close(fd);
            return (1);
        }
        syslog(LOG_ERR,"can't lock %s:%s",LOCKFILE,strerror(errno));
        exit(1);
    }
    ftruncate(fd,0);
    sprintf(buf,"%ld",(long)getpid());
    write(fd,buf,strlen(buf)+1);
    return 0;
}
int lockfile(int fd)
{
    struct flock fl;
    
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return(fcntl(fd, F_SETLK, &fl));
}
