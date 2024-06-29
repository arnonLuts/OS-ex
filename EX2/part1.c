#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
// #include <cstdio>
#include <string.h>

void write_message(const char *message, int count, int fd) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s\n", message);
    for (int i = 0; i < count; i++) {
        write(fd, buffer, strlen(buffer)); 
        usleep((rand() % 100) * 1000); // Random delay between 0 and 99 milliseconds
    }
}
int main(int argc, char* argv[]){
    if (argc != 5) {
    fprintf(stderr, "Usage: %s <parent_message> <child1_message> <child2_message> <count>", argv[0]);
        return 1;
    }
    int fd = open("output.txt", O_CREAT | O_RDWR | O_TRUNC, S_IRWXU);
    if (fd < 0) 
        { 
            perror("open error"); 
            exit(1); 
        } 
    int count = atoi(argv[4]);
    pid_t	pid, pid2;
	if ((pid = fork()) < 0) 
		printf("fork error");	
	else	{ 
		if (pid == 0) {	/* first child */
            write_message(argv[1],count, fd);
            exit(0);
		}
    }
    if ((pid2 = fork()) < 0)
        printf ("fork error");
    else if(pid2 == 0) {
        /*  We're the second child; */
        sleep(10);
        write_message(argv[2],count, fd);
        exit(0);
    }
    if (waitpid(pid, NULL, 0) != pid)
        printf("waitpid error");
    if (waitpid(pid2, NULL, 0) != pid2)	/* wait for first child */
        printf("waitpid error");
    write_message(argv[3],count, fd);
    return 0;
}

