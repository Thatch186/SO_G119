
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

void exec_tranform(int len, char *cmds[],int fd_s, int fd_c){
    char *string = array_to_string(cmds,len);
    int bytes_lidos;
    char buff[200];
    escreve(fd_s, string , strlen(string));
    while((bytes_lidos = read(fd_c, buff , 200)) > 0){
        escreve(1, buff , bytes_lidos);
    }
}

void exec_status(int fd_s, int fd_c){
    escreve(fd_s, "status " , 7);
    int bytes_lidos;
    char buff[200];
    while((bytes_lidos = read(fd_c, buff , 200)) > 0){
        escreve(1, buff , bytes_lidos);
        if(!strcmp(buff,"Completed\n"))
            break;
    }
}


//-----------------------------------------------------------------

int  main(int argc, char *argv[]){

    char buff_wr[MAX_BUFF_SIZE];
    char buff_rd[MAX_BUFF_SIZE];

    if(argc == 1){
        printf("./aurras status\n");
        printf("./aurras transform input-filename output-filename filter-id-1 filter-id-2 ...\n");
        return -1;
    }

    int status = argc > 1 && !strcmp(argv[1],"status");
    int transform = argc > 1 && !strcmp(argv[1],"transform");

    int bytes_input , bytes_server, fd_fifo_s, fd_fifo_c;

    fd_fifo_s = open_fifo("tmp/FifoS",O_WRONLY); //pipe escrita
    
    fd_fifo_c = open_fifo("tmp/FifoC",O_RDONLY); //pipe leitura

    if(status){
        exec_status(fd_fifo_s, fd_fifo_c);
    }
    else if(transform){
        exec_tranform(argc-1 ,argv+1 ,fd_fifo_s ,fd_fifo_c);
    }

    close(fd_fifo_s);
    close(fd_fifo_c);

    return 0; 
}

//Termina quando o ficheiro processado está disponível (while)

