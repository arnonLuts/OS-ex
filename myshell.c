#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define COMM_LIMIT 100
#define CHAR_LIMIT 100
char history[COMM_LIMIT][CHAR_LIMIT];
int hist_s = 0;
int hist_e = 0;

void add_to_hist(char* comm){
    strncpy(history[hist_e], comm, CHAR_LIMIT);
    hist_e = (hist_e + 1) % COMM_LIMIT;
    if (hist_e == hist_s)
    {
        hist_s = (hist_s + 1) % COMM_LIMIT;
    }
}
void print_hist(){
    int count;
    int curr = hist_s;
    if (hist_e < hist_s){
        count = COMM_LIMIT;
    }else{
        count = hist_e - hist_s;
    }
    for (int i = 0; i < count; i++)
    {
        printf("%s\n",history[curr] );
        curr = (curr + 1) % COMM_LIMIT;
    }
    
}
void pwd(){
    char dir[CHAR_LIMIT];
    if(getcwd(dir,CHAR_LIMIT) != NULL){
        printf("%s\n", dir);
    }else{
        printf("no pwd");
    }
}
int main(int argc, char* argv[]){
    char* curr_path = getenv("PATH");
    for (int i = 1; i < argc; i++)
    {
        strcat(curr_path,":");
        strcat(curr_path, argv[i]);
    }
    if(setenv("PATH", curr_path, 1) != 0) {
        perror("setenv failed");
        exit(1);
    }

    while (1)
    {
        printf("$ ");
        fflush(stdout);
        char comm[COMM_LIMIT];
        if(fgets(comm, CHAR_LIMIT, stdin) == NULL){
            break;
        }
        comm[strcspn(comm, "\n")] = '\0';

        if ( !strlen(comm) )
        {
            continue;
        }
        add_to_hist(comm);
        
        char* args[CHAR_LIMIT/2 + 1]; //there can be up to 51 args because they are seperated by spacebar
        int counter = 0;
        char* tok = strtok(comm," ");
        
        while (tok)
        {
            args[counter++] = tok;
            tok = strtok(NULL, " ");
        }
        args[counter] = NULL;
        
        if (!strcmp(args[0], "history")){
            print_hist();
        }else if (!strcmp(args[0], "pwd")){
            pwd();
        }else if (!strcmp(args[0], "cd")){
            if (args[1])
            {
                chdir(args[1]);
            }
        }else if (!strcmp(args[0], "exit")){
            break;
        }else{
            int stat, waited, ret_code;
            pid_t pid;
            pid = fork();
            if (pid < 0)
            {
                  perror("fork failed");
                    exit(-1);
            }else if (pid == 0){
                /* Child - this will be where we execute the command */
                ret_code = execvp(args[0], args);
                if (ret_code == -1)
                {
                    perror("exec failed");
                    exit(-1);
                }
            }else{
                if(waited = wait(&stat) < 0 ){
                    perror("wait failed");
                    exit(-1);
                }
            }
        }
        
        
    }

    return 0;
}