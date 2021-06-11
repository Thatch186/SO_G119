#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include<sys/wait.h>

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


//----------- FILTROS (FICHEIRO CONFIG) --------------------
typedef struct filtro{
    char *nick;
    char *name;
    int max;
    int running;
}*Filtro;

int constroi_filtros(char *file , Filtro **filtros){
    //Filtro *filtros;
    char *buff = malloc(sizeof(char));
    char *buffer = malloc(sizeof(char) *200);
    int bytes ,i, j , nr_lines=0;
    int lens[200];
    int fd = open( file , O_RDONLY , 0666);

    j=0;
    while((bytes = read(fd , buff , 1)) > 0){
        j++;
        if((*buff) == '\n'){
            lens[nr_lines]=j;
            j=0;
            nr_lines++;
        }
    }
    close(fd);
    fd = open( file , O_RDONLY , 0666);
    
    *filtros = malloc(sizeof(Filtro) * nr_lines);

    for(i=0; i<nr_lines; i++){
        (*filtros)[i] = malloc(sizeof(struct filtro));
        if((bytes = read(fd, buffer , lens[i])) > 0){
            (*filtros)[i]->nick = strdup(strsep(&buffer, " "));
            (*filtros)[i]->name = strdup(strsep(&buffer, " "));
            (*filtros)[i]->max = atoi(buffer);
            (*filtros)[i]->running = 0;
            
        }
    }

    return nr_lines;
}

void clone_filters(Filtro *novo[], Filtro filtros[] , int nr_filters){
    (*novo) = malloc(sizeof(Filtro) * nr_filters);
    for(int i=0 ; i<nr_filters ; i++){
        (*novo)[i] = malloc(sizeof(struct filtro));
        (*novo)[i]->nick=strdup(filtros[i]->nick);
        (*novo)[i]->name=strdup(filtros[i]->name);
        (*novo)[i]->max=filtros[i]->max;
        (*novo)[i]->running=filtros[i]->running;
    }
    return novo;
}

//---------------- TRANSFORM  ------------------------

int not_max(Filtro f){
    return (f->running < f->max) ? 1 : 0;
}

char *add_path(char *arg, char *path , char *f_name){ //adiciona o path ao argumento
    free(arg);
    arg=malloc(200);
    sprintf(arg, "%s/%s", path , f_name);
    return arg;
}

void redirecionamento(int fd_in, int fd_out){
    dup2(fd_in, 0); close(fd_in); //redirecionamento
    dup2(fd_out, 1); close(fd_out);
}

/*void executa(char *args[], int nr_cmds, Filtro *filters[] , char *used_filters[],int nr_filters){
    int i ,j , equals , p[2];
    int status[nr_cmds-4];

    for(i=0; i<nr_cmds-5; i++){
        if(pipe(p) == -1){
            perror("pipe");
            return -1;
        }
        if(!fork()){
            close(p[0]); 
            dup2(p[1],1);
            close(p[1]);
            execl(args[i+4], args[i+4],(char *)NULL);
            _exit(0);
        }
        else{
            close(p[1]);
            dup2(p[0],0);
            close(p[0]);
        }
    }
    if(!fork()){
        execl(args[nr_cmds-1], args[nr_cmds-1],(char *)NULL);
        _exit(0);
    }
    else{

        wait(&status[nr_cmds-4]);
        for(i=0; i< nr_cmds-4; i++){  // para cada nos argumentos:

            for(j=0 , equals=0 ; j<nr_filters && !equals ; j++)         //percorremos a lista de filtros até encontrar o filtro
                equals = !strcmp((*filters)[j]->nick ,used_filters[i]);

            if(equals){    // e mudamos nos args pelo nome completo, acrescentando o path
                (*filters)[j-1]->running--;
            }
        }
    }
}*/

int exec_transform(char *args[] ,Filtro *filters[], int nr_cmds, int nr_filters , char *filter_path,  int fd){

    Filtro *aux_filters = malloc(sizeof(Filtro *));
    clone_filters(&aux_filters, (*filters) ,nr_filters); //estrutura de segurança caso os filtros introduzidos estejam em quantidades erradas

    int fd_in_init = dup(0) , fd_out_init = dup(1); //guardar o stdin e stdout inicial
    int fd_in = open( args[2] , O_RDONLY ,0666); //abre o ficheiro de input (sample)
    int fd_out = open( args[3] , O_CREAT | O_TRUNC | O_WRONLY , 0666); // sabre o ficheiro de output (.mp3)
    int p[2] , status[nr_cmds-4]; //pipe e status

    char **used_filters = malloc(sizeof(char *) * nr_cmds-4);
    int i,j , equals;
    
    for(i=0; i < nr_cmds-4; i++){
        used_filters[i]=strdup(args[i+4]);  //filtros nos argumentos do cliente
    }

    for(i=0; i< nr_cmds-4; i++){  // para cada nos argumentos:

        for(j=0 , equals=0 ; j<nr_filters && !equals ; j++)         //percorremos a lista de filtros até encontrar o filtro
            equals = !strcmp((*filters)[j]->nick ,used_filters[i]);

        if(equals && not_max((*filters)[j-1])){    // e mudamos nos args pelo nome completo, acrescentando o path
            args[i+4] = add_path(args[i+4], filter_path, (*filters)[j-1]->name);
            (*filters)[j-1]->running++;
        }
        else{
            (*filters)=aux_filters;
            return -1;
        }
    }

    redirecionamento(fd_in, fd_out);
    
    for(i=0; i<nr_cmds-5; i++){
        if(pipe(p) == -1){
            perror("pipe");
            return -1;
        }
        if(!fork()){
            close(p[0]); 
            dup2(p[1],1);
            close(p[1]);
            execl(args[i+4], args[i+4],(char *)NULL);
            _exit(0);
        }
        else{
            close(p[1]);
            dup2(p[0],0);
            close(p[0]);
            wait(&status[i]);
        }
    }
    if(!fork()){
        execl(args[nr_cmds-1], args[nr_cmds-1],(char *)NULL);
        _exit(0);
    }
    else{
        wait(&status[nr_cmds-4]);
        dup2(1,fd_out_init);
        dup2(0,fd_in_init);

        for(i=0; i< nr_cmds-4; i++){  
                                                                        // para cada nos argumentos:
            for(j=0 , equals=0 ; j<nr_filters && !equals ; j++)         // percorremos a lista de filtros até encontrar o filtro
                equals = !strcmp((*filters)[j]->nick ,used_filters[i]); // e decrementamos o seu uso dos filtros

            if(equals){    
                (*filters)[j-1]->running--;
            }
        }
    }
    //executa( args, nr_cmds, &filters , &used_filters , nr_filters);
    return 1;
}
//_________________________________________________________________________
//
//_________________  STATUS ______________________________________________

typedef struct task{
    int index;
    char *cmd;
}*Task;

Task create_task(int index , char *line){
    Task new = malloc(sizeof(struct task));
    new->cmd=strdup(line);
    new->index=index;
}

void add_task(Task *tasks[],int nr_tasks, Task new){
    (*tasks) = realloc((*tasks), sizeof(tasks) + sizeof(struct task));
    (*tasks)[nr_tasks] = create_task(new->index , new->cmd);
}

void get_status(Task tasks[], int nr_tasks, int fd){
    int i;
    
    for(i=0; i<nr_tasks ; i++){
        char buff[200];
        sprintf(buff, "task #%02d: %s \n",i , tasks[i]->cmd);
        escreve(fd, buff , strlen(buff));
    }
}


//________________________________________________________________________
//-------------------   MAIN    --------------------------------------
int main(int argc, char *argv[]){
    
    argc+=2;
    argv[1] = "etc/aurrasd.conf";
    argv[2] = "bin/aurrasd-filters";

    //---constroi filtros a partir da config
    Filtro *filtros;
    int nr_filters = constroi_filtros(argv[1] , &filtros);
    //--------------------------------------

    Task *tasks = malloc(sizeof(Task *));
    int nr_tasks = 0;
    
    //-------------------------------------
    int fd_fifo_s , fd_fifo_c , fd_aux , i; 
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
        
        
        if((bytes_read = read(fd_fifo_s ,buff_read ,MAX_BUFF_SIZE))>0){

            Task new = create_task(nr_tasks, buff_read);
            add_task(&tasks, nr_tasks ,new);
            nr_tasks++;
            
            int nr_cmds=0;
            char **comandos = string_to_array(buff_read, &nr_cmds);

            
            if(!strcmp(comandos[1] , "status")){
                get_status(tasks, nr_tasks, fd_fifo_c);
            }
            if(!strcmp(comandos[1], "transform")){
                int ret = exec_transform(comandos, &filtros , nr_cmds , nr_filters , argv[2] , fd_fifo_c);
                if(ret==-1)
                    write(fd_fifo_c, "Filters not avaliable, try again\n", 33);
                else
                    write(fd_fifo_c, "Completed\n", 11);
            }
        }
        i++;
    }

    return 0;
}

