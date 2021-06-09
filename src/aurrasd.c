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
    return fd_fifo;
}



char **add_to_array(char **array, int array_len , char *line){
    array = realloc(array, array_len+1);
    array[array_len]=malloc(strlen(line));
    array[array_len]=line;
    return array; 
}

typedef struct task{
    int index;
    int nr_filters;
    char **filters;
}Task;


char **string_to_array(char *line , int *len){
    int i;
    for(i=0; line[i] ; i++)
        if(line[i]==' ')
            (*len)++;
    char **comands = malloc(sizeof(char*) * (*len));
    for(i=0; i<(*len) ; i++){
        comands[i] = strdup(strsep(&line , " "));
    }
    return comands;
}

void get_status(int fd){
    escreve(fd,"Status not defined yet\n",23);
}

void exec_transform(char *args[] , int N , int fd){

    
    int stdin2 = dup(0);
    int stdout2 = dup(1);
    int fd_in = open( args[1] , O_RDONLY ,0666);
    int fd_out = open( args[2] , O_CREAT | O_TRUNC | O_WRONLY , 0666);

    int res1 = dup2(fd_in, 0);
    int res2 = dup2(fd_out, 1);
    int status;
    
    if(!fork()){
        escreve(fd, "In fork\n", 8);
        execl("bin/aurrasd-filters/aurrasd-echo" , "bin/aurrasd-filters/aurrasd-echo" , (char *)NULL);
        _exit(0);
    }
    pid_t terminated_pid = wait(&status);
    escreve(fd, "Returned\n", 9);
    

}

int main(int argc, char *argv[]){

    int estado_processo=0;
    int fd_fifo_s , fd_fifo_c , fd_aux; 
    int bytes_read ; //bytes_input;
    char buff_read[MAX_BUFF_SIZE];
    char buff_write[MAX_BUFF_SIZE];
    char **comands;

    mkfifo("tmp/FifoS", 0666); //Leitura pelo Servidor
    mkfifo("tmp/FifoC", 0666); //Leitura pelo Cliente

    printf("[SERVER]\n");
    
    fd_fifo_s = open_fifo("tmp/FifoS", O_RDONLY);

    fd_fifo_c = open_fifo("tmp/FifoC", O_WRONLY);

    fd_aux = open_fifo("tmp/FifoS", O_WRONLY);
    
    
    while(1){ 
        
        bytes_read = read(fd_fifo_s ,buff_read ,MAX_BUFF_SIZE);

        int len=0;
        char **comandos = string_to_array(buff_read, &len);

        
        if(!strcmp(comandos[0] , "status")){
            get_status(fd_fifo_c);
        }
        if(!strcmp(comandos[0], "transform")){
            exec_transform(comandos, len , fd_fifo_c);
        }

        //escreve(1,buff_read,bytes_read);
        //escreve(1,"\n",1);

        if(estado_processo == 0)
            escreve(fd_fifo_c,"Pending...\n",11);

        else if(estado_processo ==1)
            escreve(fd_fifo_c,"Processing...\n",14);

        else escreve(fd_fifo_c,"Completed.\n",11);
    }

    return 0;
}

