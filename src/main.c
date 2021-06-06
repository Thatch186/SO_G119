#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFF_SIZE 1024

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

int main(){

    mkfifo("fifo",0666);

    if(!fork()){
        execlp("CLIENTE","CLIENTE", NULL);
        _exit(0);
    }
    //cliente("fifo");
    leitura("fifo");
    return 0;
}