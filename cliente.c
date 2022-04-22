#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#define MAX 1024

int main(int argc, char *argv[])
{
    int fd_exec = open("tmp/pipe_exec", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_exec < 0)
    {
        perror("Exec Error");
    }

    if (argc == 2 && strcmp(argv[1], "status") == 0)
    {
        write(fd_exec, argv[1], strlen(argv[1]));

        int fd_status = open("/tmp/pipe_status", O_RDONLY);
        if (fd_status < 0)
        {
            perror("Status Error");
        }
        int read_bytes = 0;
        char *buffer = malloc(sizeof(char) * 1024);

        while ((read_bytes = read(fd_status, buffer, MAX)) > 0)
        {
            write(1,buffer,read_bytes);
            
        }
        free(buffer);
        exit(1);
    }

    else if (argc > 3 && strcmp(argv[1], "nop") == 0)
    {
        char * buffer = malloc(sizeof(char)*1024);
    }
    


    return 0;
}
