
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


    printf("[CLIENTE]\n");
    char buff_wr[MAX_BUFF_SIZE];
    char buff_rd[MAX_BUFF_SIZE];
    int bytes_input , bytes_server, fd_fifo_s, fd_fifo_c;

    if((fd_fifo_s = open("tmp/FifoS",O_WRONLY)) == -1){ //pipe escrita
        perror("open");
    }else{
        printf("opened fifoS for writing");
    }

    if((fd_fifo_c = open("tmp/FifoC",O_RDONLY)) == -1){ //pipe leitura
        perror("open");
    }else{
        printf("opened fifoC for reading");
    }

    while( (bytes_input = read(0 ,buff_wr ,MAX_BUFF_SIZE)) > 0){  //escreve para o servidor, mas se a primeira letra inserida for c, recebe uma mensagem
        if(write(fd_fifo_s,buff_wr,bytes_input) == -1){
            perror("write in");
        }
        if(buff_wr[0] == 'c'){ 
            bytes_server = read( fd_fifo_c ,buff_rd ,MAX_BUFF_SIZE);
            if(write(1,buff_rd,bytes_server) == -1)
                perror("write of");
        }
        
    }

    close(fd_fifo_s);
    close(fd_fifo_c);

    return 0; 
}