#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX 1024

char **gargv;
char *final;
int pid;

char *bytes_files(char *file1, char *file2)
{
    int file_bytes = 0;
    int save_bytes = 0;
    char *prog = malloc(sizeof(int) * 100);
    int file = open(file1, O_RDONLY);
    char c;
    while (read(file, &c, 1) == 1)
    {
        file_bytes++;
    }

    close(file);
    int save = open(file2, O_RDONLY);
    char s;
    while (read(save, &s, 1) == 1)
    {
        save_bytes++;
    }
    close(save);

    sprintf(prog, "Concluded (bytes-input: %d, bytes-output: %d)\n", file_bytes, save_bytes);
    return prog;
}

void sigusr1(int signum)
{
    write(1, "Pending\n", strlen("Pending\n"));
}

void sigusr2(int signum)
{
    write(1, "Processing\n", strlen("Processing\n"));
}

void sigchld(int signum)
{
    final = bytes_files(gargv[2], gargv[3]);
    write(1, final, strlen(final));
    exit(1);
}

int main(int argc, char *argv[])
{
    gargv = argv;
    if (signal(SIGUSR1, sigusr1) == SIG_ERR)
    {
        perror("failed sigusr1\n");
        exit(1);
    }

    if (signal(SIGUSR2, sigusr2) == SIG_ERR)
    {
        perror("failed sigusr2\n");
        exit(1);
    }
    if (signal(SIGCHLD, sigchld) == SIG_ERR)
    {
        perror("failed sigchld\n");
        exit(1);
    }

    if (mkfifo("tmp/pipe_exec", 0777) == -1)
    {
        if (errno != EEXIST)
        {
            perror("could not create pipe_exec\n");
            exit(1);
        }
    }
    if (mkfifo("tmp/main_pipe", 0777) == -1)
    {
        if (errno != EEXIST)
        {
            perror("could not create main_pipe\n");
            exit(1);
        }
    }
    if (mkfifo("tmp/pipe_process", 0777) == -1)
    {
        if (errno != EEXIST)
        {
            perror("could not create pipe_process\n");
            exit(1);
        }
    }
    if (mkfifo("tmp/pipe_status", 0777) == -1)
    {
        if (errno != EEXIST)
        {
            perror("could not create pipe_status\n");
            exit(1);
        }
    }

    pid = getpid();

    char *buffer = malloc(sizeof(int) * MAX);

    int fd_exec = open("tmp/pipe_exec", O_WRONLY);
    if (fd_exec < 0)
    {
        perror("Error opening pipe exec");
        exit(1);
    }

    if (argc > 2 && strcmp(argv[1], "proc-file") == 0)
    {
        write(fd_exec, argv[1], 15);
        close(fd_exec);

        int main_pipe = open("tmp/main_pipe", O_WRONLY);

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

        int fd_process = open("tmp/pipe_process", O_WRONLY);
        if (fd_process == -1)
        {
            perror("Error openning process pipe\n");
        }

        char *pid_buffer = malloc(sizeof(char) * 1024);
        sprintf(pid_buffer, "%d", pid);
        write(fd_process, pid_buffer, 1024);
        free(pid_buffer);
        close(fd_process);
    }

    if (argc == 2 && strcmp(argv[1], "status") == 0)
    {
        write(fd_exec, argv[1], strlen(argv[1]));
        close(fd_exec);

        int pipe_status = open("tmp/pipe_status", O_RDONLY);
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

    while (1);

    return 0;
}
