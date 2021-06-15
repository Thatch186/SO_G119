#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

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

char **add_to_array(char **array, int array_len , char *line){
    array = realloc(array, array_len+1);
    array[array_len]=malloc(strlen(line));
    array[array_len]=line;
    return array; 
}

char **string_to_array(char *buff , int *len){
    char *line=strdup(buff);
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
typedef struct task{
    int index;
    char *cmd;
}*Task;


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

void clone_filters(Filtro *novo[], Filtro filtros[] , int nr_filtros){
    (*novo) = malloc(sizeof(Filtro) * nr_filtros);
    for(int i=0 ; i<nr_filtros ; i++){
        (*novo)[i] = malloc(sizeof(struct filtro));
        (*novo)[i]->nick=strdup(filtros[i]->nick);
        (*novo)[i]->name=strdup(filtros[i]->name);
        (*novo)[i]->max=filtros[i]->max;
        (*novo)[i]->running=filtros[i]->running;
    }
}

int not_max(Filtro f){
    return (f->running < f->max) ? 1 : 0;
}

int filtros_validos(Filtro filters[], int nr_filtros , char *args[] ,int nr_cmds, char *filter_path){


    Filtro *aux_filters = malloc(sizeof(Filtro *));
    clone_filters(&aux_filters, filters ,nr_filtros); //estrutura de segurança caso os filtros introduzidos estejam em quantidades erradas

    Filtro *filtro_args;
    int len = constroi_filtros( filter_path, &filtro_args);

    int i,j , equals , valid=1 ;

    for(i=0; i<nr_cmds-4 && valid; i++){
        for(j=0 , equals=0 ; j<len && !equals ; j++)         //percorre os argumentos e verifica se estes são inválidos
            equals = !strcmp(filtro_args[j]->nick ,args[i+4]);
        if(equals){
            filtro_args[j-1]->running++;
            if(filtro_args[j-1]->running > filtro_args[j-1]->max)
                valid=0;
        }
        else
            valid=0;
    }

    for(i=0; i< nr_cmds-4 && (valid==1) ; i++){  // para cada nos argumentos:

        for(j=0 , equals=0 ; j<nr_filtros && !equals ; j++)         //percorremos a lista de filtros até encontrar o filtro
            equals = !strcmp(aux_filters[j]->nick ,args[i+4]);

        if(equals && not_max(aux_filters[j-1])){    // e incrementamos caso não seja máximo
            aux_filters[j-1]->running++;
        }
        else if(equals){                            //se não tiver os filtros disponiveis retorna 2
            valid = 2;
        }
    }
    
    return valid;
}

void inc_filters(Filtro *filters[], int nr_filtros , char *args[] ,int nr_cmds){

    int i,j , equals;
    
    for(i=0; i< nr_cmds-4; i++){                                                                      // para cada nos argumentos:
        for(j=0 , equals=0 ; j<nr_filtros && !equals ; j++){         // percorremos a lista de filtros até encontrar o filtro
            equals = !strcmp((*filters)[j]->nick , args[i+4]); // e decrementamos o seu uso dos filtros
        }

        if(equals){    
            (*filters)[j-1]->running++;
        }
    }
}

void dec_filters(Filtro *filters[], int nr_filtros ,int **to_decrement, Task *tasks[], int *nr_to_dec, int *nr_tasks){

    int i,j ,k ,nr_args, equals;
    char **args=malloc(sizeof(char **));
    
    for(i=0 ; i<1024 && (*nr_to_dec) >= 0 ; i++){  
        
        if( (*to_decrement)[i] != 0){
            //printf("%s %d\n",tasks[i]->cmd, i);
            nr_args=0;
            args=string_to_array((*tasks)[i]->cmd , &nr_args);
            
            for(k=5; k<nr_args ; k++){                                                 // para cada nos argumentos:
                for(j=0, equals=0 ; j<nr_filtros && !equals ; j++)         // percorremos a lista de filtros até encontrar o filtro
                    equals = !strcmp((*filters)[j]->nick ,args[k]);       // e decrementamos o seu uso dos filtros

                if(equals && ((*filters)[j-1]->running > 0)){    
                    (*filters)[j-1]->running--;
                }
            }
            (*tasks)[i]=NULL;
            (*nr_tasks)--;
            (*to_decrement)[i]=0;
            (*nr_to_dec)--;
        }
    }
}
//---------------- TRANSFORM  ------------------------


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

void exec_transform(char *args[] ,Filtro *filters[], int nr_cmds, int nr_filtros , char *filter_path){

    Filtro *aux_filters = malloc(sizeof(Filtro *));
    clone_filters(&aux_filters, (*filters) ,nr_filtros); //estrutura de segurança caso os filtros introduzidos estejam em quantidades erradas

    int fd_in = open( args[2] , O_RDONLY ,0666); //abre o ficheiro de input (sample)
    int fd_out = open( args[3] , O_CREAT | O_TRUNC | O_WRONLY , 0666); // sabre o ficheiro de output (.mp3)
    int p[2] , status; //pipe e status
    pid_t fork_pid;
    int i,j , equals;

    for(i=0; i< nr_cmds-4; i++){  // para cada nos argumentos:

        for(j=0 , equals=0 ; j<nr_filtros && !equals ; j++)         //percorremos a lista de filtros até encontrar o filtro
            equals = !strcmp((*filters)[j]->nick ,args[i+4]);

        if(equals){    // e mudamos nos args pelo nome completo, acrescentando o path
            args[i+4] = add_path(args[i+4], filter_path, (*filters)[j-1]->name);
        }
    }


    redirecionamento(fd_in, fd_out);
    
    for(i=0; i<nr_cmds-5; i++){
        if(pipe(p) == -1){
            perror("pipe");
            return ;
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
            //wait(&status[i]);
        }
    }
    if((fork_pid=fork())==0){
        execl(args[nr_cmds-1], args[nr_cmds-1],(char *)NULL);
        _exit(0);
    }
    else{
        waitpid(fork_pid, &status, 0);
        if(!WIFEXITED(status)){
            perror("BAD EXIT");
        }
    }
}
//_________________________________________________________________________
//
//_________________  STATUS ______________________________________________

void sig_term_handler(){
    
}

Task create_task(int index , char *line){
    Task new = malloc(sizeof(struct task));
    new->cmd=strdup(line);
    new->index=index;
    return new;
}

void add_task(Task *tasks[],int *nr_tasks, Task new, int index){
    //(*tasks) = realloc((*tasks), sizeof(struct task) * ((*nr_tasks) +1));
    (*tasks)[index] = create_task(new->index , new->cmd);
    (*nr_tasks)++;
}

void remove_task(Task *tasks[],int *nr_tasks, int index){

    if((*tasks)[index] != NULL){
        free((*tasks)[index]->cmd);
        (*tasks)[index]=NULL;
        (*nr_tasks)--;
    }
}

void get_status(Task tasks[], Filtro filters[],int nr_tasks, int nr_filtros, int fd , int pid){
    int i;
    char buff[1024];
    char *final=malloc(1024);
    strcpy(final,"");
    for(i=0; i<1024 ; i++){
        if(tasks[i]!=NULL){
            sprintf(buff, "task #%d: %s \n", tasks[i]->index , tasks[i]->cmd);
            escreve(fd, buff, strlen(buff));
        }
    }
    for(i=0; i< nr_filtros ; i++){
        sprintf(buff, "filter %s: %d/%d (running/max)\n", filters[i]->nick, filters[i]->running, filters[i]->max);
        escreve(fd, buff, strlen(buff));
     }
    sprintf(buff, "pid: %d\n",pid);
    escreve(fd, buff, strlen(buff));

    free(final);
}

char ** add_queue(char *line, char *queue[], int size){
    char **res = (char **)realloc(queue, sizeof(char *) * (size +1));
    res[size] = strdup(line);
    return res;
}

char  **clone_str_array(char *cmds[] , int nr_cmds){
    char **novo = malloc(sizeof(char *)* nr_cmds);
    for(int i=0; i<nr_cmds ; i++)
        novo[i]=strdup(cmds[i]);
    return novo;
}
//________________________________________________________________________
//-------------------   MAIN    --------------------------------------
int main(int argc, char *argv[]){
    
    argc+=2;
    argv[1] = "etc/aurrasd.conf";
    argv[2] = "bin/aurrasd-filters";

    //---constroi filtros a partir da config
    Filtro *filtros;
    int nr_filtros = constroi_filtros(argv[1] , &filtros);
    int validos;

    //--------------------------------------

    Task *tasks = malloc(sizeof(Task )*1024);
    Task new = NULL;
    int nr_tasks = 0;
    
    //-------------------------------------
    int fd_fifo_s , fd_fifo_c , fd_aux , i=0,j; 
    int bytes_read ; //bytes_input;
    char **queue = malloc(sizeof(char **));
    int queue_size = 0;
    int process_index = 0;
    int *to_decrement=malloc(sizeof(int)*1024);
    int nr_to_decrement=0;
    pid_t terminated_status;
    int status;

    for(i=0; i<1024; i++) tasks[i] = NULL;
    for(i=0; i<1024; i++) to_decrement[i] = 0;

    mkfifo("tmp/FifoS", 0666); //Leitura pelo Servidor
    mkfifo("tmp/FifoC", 0666); //Leitura pelo Cliente

    printf("[SERVER]\n");
    
    fd_fifo_s = open_fifo("tmp/FifoS", O_RDONLY);
    fd_aux = open_fifo("tmp/FifoS", O_WRONLY);

    if(signal(SIGTERM, sig_term_handler) == SIG_ERR){
        perror("SIGTERM failed");
    }
    
    while(1){ 
        
        char *buff_read = malloc(MAX_BUFF_SIZE);
        if((bytes_read = read(fd_fifo_s ,buff_read ,MAX_BUFF_SIZE))>0 || (queue_size>0) ){
            
            int nr_cmds=0;
            char **comandos = string_to_array(buff_read, &nr_cmds);
            
            if(!strcmp(comandos[0], "decrementa")){
                dec_filters(&filtros, nr_filtros, &to_decrement , &tasks , &nr_to_decrement, &nr_tasks);
            }
 
            if(queue_size){
                
                queue = add_queue(buff_read, queue , queue_size);
                queue_size++;
                strcpy(buff_read,queue[0]);
                for(j=0; j<queue_size-1; j++){                  
                    queue[j]=strdup(queue[j+1]);
                }
                free(queue[--queue_size]);
                nr_cmds=0;
                comandos = string_to_array(buff_read, &nr_cmds);
            }
            
            if(nr_cmds > 1 && !strcmp(comandos[1] , "status")){

                fd_fifo_c = open_fifo("tmp/FifoC", O_WRONLY);
                get_status(tasks, filtros, nr_tasks, nr_filtros, fd_fifo_c, getpid());
                close(fd_fifo_c);

            }
            if(nr_cmds > 2 && !strcmp(comandos[2], "transform")){
                
                int pid = atoi(comandos[0]);
                comandos++; //avança o array
                nr_cmds--;

                if((validos = filtros_validos(filtros, nr_filtros, comandos, nr_cmds,argv[1])) == 1){

                        new = create_task(process_index, buff_read);
                        add_task(&tasks, &nr_tasks ,new, process_index);
                        nr_to_decrement++;
                        to_decrement[process_index]=1;
                        process_index++;

                        kill(pid,SIGUSR1);
                        inc_filters(&filtros, nr_filtros, comandos, nr_cmds);

                    if(!fork()){
                        pid_t exec_pid = fork();
                        if(exec_pid == 0){
                            exec_transform(comandos, &filtros , nr_cmds , nr_filtros , argv[2]);
                            _exit(0);
                        }
                        else{
                            terminated_status = waitpid(exec_pid, &status, 0);
                            if(WIFEXITED(status)){
                                kill(pid,SIGUSR2);
                                //strsep(&buff_read," ");
                            }
                            else{
                                perror("BAD EXIT");
                            }
                            _exit(0);
                        }   
                    }                                              
                }                                                       
                else if(validos == 2){
                    queue = add_queue(buff_read, queue , queue_size);
                    queue_size++;
                }
                else{
                    for(i=0; i<nr_cmds; i++){
                        free(comandos[i]);
                    }
                    kill(pid,SIGQUIT);
                }
            }
        }
    }

    close(fd_fifo_s);
    close(fd_aux);
    return 0;
}

