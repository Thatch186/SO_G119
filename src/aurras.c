
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define MAX_BUFF_SIZE 1024

void escreve(int fd_fifo, char *buff ,int bytes){
    if(write(fd_fifo,buff,bytes) == -1){
        perror("write in");
    }
}

int open_fifo(char *path , int flag){
    int fd_fifo;
    if((fd_fifo=open(path, flag)) == -1)
        perror("Open");
    return fd_fifo;
}

void check_if(int bool, char *arg , int fd_s , int fd_c){
    int bytes;
    char buff[1024];
    if(bool){
        escreve(fd_s, arg , strlen(arg));
        bytes = read(fd_c , buff , MAX_BUFF_SIZE);
            escreve(1,buff,bytes);
        /*if((bytes = read(fd_c, buff, MAX_BUFF_SIZE)) > 0){
            escreve(1,buff,bytes);
        }*/
    }
}

char *array_to_string(char *a[], int n){
    char *new;
    int i, len=0;
    for(i=0; i<n; i++)
        len+=strlen(a[i]);
    new = malloc(len + n);
    for(i=0; i<n ; i++){
        new = strcat(new, a[i]);
        new = strcat(new, " ");
    }
    return new;
}

void exec_tranform(int len, char *cmds[],int fd_s , int pid){
    char *string = array_to_string(cmds,len);
    char buff[1024];
    sprintf(buff, "%d %s",pid, string);
    escreve(fd_s, buff , strlen(buff));
    printf("Pending\n");
    pause();
    pause();
    escreve(fd_s, "decrementa ", 11);
}

void exec_status(int len, char *cmds[],int fd_s){
    char *string = array_to_string(cmds,len);
    int bytes_lidos;
    char buff[1024];
    escreve(fd_s, string , strlen(string));
    int fd_c = open_fifo("tmp/FifoC",O_RDONLY); //pipe leitura
    while((bytes_lidos = read(fd_c, buff , 200))>0){
        escreve(1, buff , bytes_lidos);
    }
    close(fd_c);
}

//--------------------- SIGNALS------------------------------------

void sig_processing(){
    printf("Processing\n");
}

void sig_completed(){
    printf("Completed\n");
}

void sig_quit(){
    printf("ERRO: Os filtros Introduzidos são inválidos\n");
    kill(getpid(), SIGINT);
}
//-----------------------------------------------------------------

int  main(int argc, char *argv[]){

    //char buff_wr[MAX_BUFF_SIZE];
    //char buff_rd[MAX_BUFF_SIZE];

    if(argc == 1){
        printf("./aurras status\n");
        printf("./aurras transform input-filename output-filename filter-id-1 filter-id-2 ...\n");
        return -1;
    }

    int pid = getpid();
    int status = argc > 1 && !strcmp(argv[1],"status");
    int transform = argc > 1 && !strcmp(argv[1],"transform");

    int  fd_fifo_s;

    fd_fifo_s = open_fifo("tmp/FifoS",O_WRONLY); //pipe escrita
    
    
    //------SIGNALS-----------
    if(signal(SIGUSR1, sig_processing)  == SIG_ERR){
        perror("SIGUSR1 failed");
    }

    if(signal(SIGUSR2, sig_completed) == SIG_ERR){
        perror("SIGUSR2 failed");
    }
    if(signal(SIGQUIT, sig_quit) == SIG_ERR){
        perror("SIGQUIT failed");
    }

    //--------------------------------------
    if(status){
        exec_status(argc ,argv ,fd_fifo_s);
    }
    else if(transform){
        exec_tranform(argc ,argv ,fd_fifo_s , pid);
    }

    close(fd_fifo_s);
    //close(fd_fifo_c);

    return 0; 
}

//Termina quando o ficheiro processado está disponível (while)

