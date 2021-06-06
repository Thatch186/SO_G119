#include "../includes/cliente.h"

int  main(int argc, char *agrv[]){

    printf("In cliente\n");
    char teste[] = "Isto é um teste";
    char buff[MAX_BUFF_SIZE];
    int bytes_read, fd;

    if((fd = open("fifo",O_WRONLY)) == -1){ //pipe ou escrita ou leitura
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