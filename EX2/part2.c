#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
// #include <cstdio>
#include <string.h>

void write_message(const char *message, int count) {
    for (int i = 0; i < count; i++) {
        printf("%s\n", message);
        usleep((rand() % 100) * 1000); // Random delay between 0 and 99 milliseconds
    }
}
void busy_wait(const char *message, int count){
    while (1)
    {
        int fd = open("lockfile.lock", O_CREAT | O_EXCL , S_IRWXU);
        if (fd < 0) { 
            usleep(100000);
        } else{
            write_message(message, count);
            close(fd);
            remove("lockfile.lock");
            break;
        }
    }
}
int main(int argc, char* argv[]){
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <message1> <message2> ... <count>", argv[0]);
        return 1;
    }
    int fd = open("output2.txt", O_CREAT | O_RDWR | O_TRUNC, S_IRWXU);
    if (fd < 0) 
    { 
        perror("open error"); 
        exit(1); 
    } 
    pid_t pid_arr[argc - 2];
    int count = atoi(argv[argc-1]);
    for (int i = 0; i < argc - 2; i++)
    {
        if ((pid_arr[i] = fork()) < 0) 
            printf("fork error");	
        else	{ 
            if (pid_arr[i] == 0) {	
                busy_wait(argv[i+1],count);
                exit(0);
            }
        }
        if (waitpid(pid_arr[i], NULL, 0) != pid_arr[i])
            printf("waitpid error");
    }

    

    
    return 0;
}

