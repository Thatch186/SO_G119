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

    printf("In server\n");
    int fd, bytes_read;
    char buff[MAX_BUFF_SIZE];

    if((fd=open("tmp/FifoS", O_RDONLY)) == -1){
        perror("open");
    }
    else{
        printf("Opened fifo for read only");
    }
    while( (bytes_read = read(fd ,buff ,MAX_BUFF_SIZE)) > 0){
        if(write(1,buff,bytes_read) == -1){
            perror("write");
        }
    }
    return 0;
}

