#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFF_SIZE 1024

int main(int argc, char *argv[]){

    mkfifo("tmp/FifoS", 0666);
    mkfifo("tmp/FifoC", 0666);

    printf("[SERVER]\n");
    int fd_fifo_s , fd_fifo_c; 
    int bytes_read , bytes_input;
    char buff_read[MAX_BUFF_SIZE];
    char buff_write[MAX_BUFF_SIZE];

    if((fd_fifo_s=open("tmp/FifoS", O_RDONLY)) == -1){
        perror("open1");
    }
    else{
        printf("Opened fifo for read only");
    }

    if((fd_fifo_c=open("tmp/FifoC", O_WRONLY)) == -1){
        perror("open2");
    }
    else{
        printf("Opened fifoC for writing only");
    }

    while( (bytes_read = read(fd_fifo_s ,buff_read ,MAX_BUFF_SIZE)) > 0){ //recebe do cliente, mas se a primeira letra recebida for 'c' manda uma mensagem
        if(write(1,buff_read,bytes_read) == -1){
            perror("write");
        }
        if(buff_read[0] == 'c'){ 
            bytes_input = read(0,buff_write ,MAX_BUFF_SIZE);
            if(write(fd_fifo_c,buff_write,bytes_input) == -1){
                perror("write");
            }
            //break;
        }
        bytes_read=0;
        bytes_input=0;
    }
    return 0;
}

