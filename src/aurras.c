
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

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
    else if (flag)
        printf("Opened fifo for writing only");
    else 
        printf("Opened fifo for reading only");
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

void exec_tranforma(int len, char *cmds[],int fd_s){
    int i=0;
    for(i=0; i<len; i++){
        escreve(fd_s, cmds[i] , strlen(cmds[i]));
    }
}


int  main(int argc, char *argv[]){


    printf("[CLIENTE]\n");
    char buff_wr[MAX_BUFF_SIZE];
    char buff_rd[MAX_BUFF_SIZE];
    int status = argc > 1 && !strcmp(argv[1],"status");
    int transform = argc > 1 && !strcmp(argv[1],"transform");

    int bytes_input , bytes_server, fd_fifo_s, fd_fifo_c;

    fd_fifo_s = open_fifo("tmp/FifoS",O_WRONLY); //pipe escrita
    
    fd_fifo_c = open_fifo("tmp/FifoC",O_RDONLY); //pipe leitura
     
    check_if(status, "0" , fd_fifo_s , fd_fifo_c);
    check_if(transform, "1" , fd_fifo_s , fd_fifo_c);
    
    while((bytes_input = read(0 ,buff_wr ,MAX_BUFF_SIZE)) > 0){  //escreve para o servidor, mas se a primeira letra inserida for c, recebe uma mensagem
        
        escreve(fd_fifo_s, buff_wr, bytes_input);

        if((bytes_server= read(fd_fifo_c, buff_rd, MAX_BUFF_SIZE)) > 0){
            escreve(1,buff_rd,bytes_server);
        }
        
    }

    close(fd_fifo_s);
    close(fd_fifo_c);

    return 0; 
}

//Termina quando o ficheiro processado está disponível (while)

