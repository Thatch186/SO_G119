#include "../includes/server.h"

void leitura(char *fifo){

    printf("In server\n");
    int fd, bytes_read;
    char buff[MAX_BUFF_SIZE];

    if((fd=open("fifo", O_RDONLY)) == -1){
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
}