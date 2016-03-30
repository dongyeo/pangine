#include "mySignal.h"

void sig_usr(int signo){
    if(signo==SIGUSR1){
        tpool_destroy();
        exit(0);
    }
}

void reg_sig(){
    if(signal(SIGUSR1,sig_usr)==SIG_ERR){
       fputs("can't catch stop signal\n",stderr);
    }
}
