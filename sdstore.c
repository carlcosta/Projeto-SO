#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define MAX 1024

int main(int argc, char *argv[])
{

    int pid = getpid();

    char *buffer = malloc(sizeof(int) * MAX);

    if (mkfifo("pipe_exec", 0777) == -1)
    {
        if (errno != EEXIST)
        {
            perror("could not create pipe_exec\n");
            return 1;
        }
    }

    int fd_exec = open("pipe_exec", O_WRONLY);
    if (fd_exec < 0)
    {
        perror("Error opening pipe exec");
        return 3;
    }

    if (argc > 2 && strcmp(argv[1], "proc-file") == 0)
    {

        write(fd_exec, argv[1], 15);
        close(fd_exec);

        if (mkfifo("main_pipe", 0777) == -1)
        {
            if (errno != EEXIST)
            {
                perror("could not create main_pipe\n");
                return 1;
            }
        }

        int main_pipe = open("main_pipe", O_WRONLY);

        if (main_pipe == -1)
        {
            perror("Error openning main pipe\n");
        }

        int i = 2;
        char *word = malloc(sizeof(int) * MAX);
        strcpy(word, "");

        while (argv[i])
        {
            strcat(word, argv[i]);
            strcat(word, " ");
            i++;
        }

        if (write(main_pipe, word, strlen(word)) == -1)
        {
            perror("Error writing in main pipe");
            return 2;
        }

        free(word);
        free(buffer);
        close(main_pipe);
    }

    if (argc == 2 && strcmp(argv[1], "status") == 0)
    {
        write(fd_exec, argv[1], strlen(argv[1]));

        close(fd_exec);

        int pipe_status = open("pipe_status", O_RDONLY);
        if (pipe_status < 0)
        {
            perror("Error opening status pipe");

            return 4;
        }
        int read_bytes = 0;
        char *buf = malloc(sizeof(char) * 1024);

        while ((read_bytes = read(pipe_status, buf, MAX)) > 0)
        {
            write(STDOUT_FILENO, buf, read_bytes);
        }

        
        free(buf);
        close(pipe_status);

        exit(1);
    }
    int pipe_status = open("pipe_status", O_RDONLY);
    if (pipe_status < 0)
    {
        perror("Error opening status pipe");

        return 4;
    }
    int read_bytes = 0;
    char *buf = malloc(sizeof(char) * 1024);

    while ((read_bytes = read(pipe_status, buf, MAX)) > 0)
    {
        write(STDOUT_FILENO, buf, MAX);
    }
    free(buf);
    close(pipe_status);

    return 0;
}