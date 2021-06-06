
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFF_SIZE 1024

int  main(int argc, char *agrv[]){

    //mkfifo("tmp/FifoC", 0666);

    printf("In cliente\n");
    char buff[MAX_BUFF_SIZE];
    int bytes_read, fd;

    if((fd = open("tmp/FifoS",O_WRONLY)) == -1){ //pipe ou escrita ou leitura
        perror("open");
    }else{
        printf("opened fifo for writing");
    }

    while( (bytes_read = read(0 ,buff ,MAX_BUFF_SIZE)) > 0){
        if(write(fd,buff,bytes_read) == -1){
            perror("write");
        }
    }

    close(fd);

    return 0; 
}