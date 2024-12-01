#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char* argv[])
{
    pid_t cpid;
    const char* fifo_path = "/tmp/channel";
    if (mkfifo(fifo_path, 0666) == -1)
    {
        write(STDERR_FILENO, strerror(errno), strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        exit(-1);
    }

    if ((cpid = fork()) < 0)
    {
        printf("Error of creating child process!\n");
        exit(EXIT_FAILURE);
    }

    int fd;
    char wr_buf[512], rd_buf[512];
    if (cpid)
    {
        // Parent process
        if ((fd = open(fifo_path, O_WRONLY)) < 0) 
        {
            write(STDERR_FILENO, strerror(errno), strlen(strerror(errno)));
            write(STDERR_FILENO, "\n", 1);
            unlink(fifo_path);
            exit(-1);      
        }

        fgets(wr_buf, sizeof(wr_buf), stdin);

        if (write(fd, wr_buf, strlen(wr_buf) + 1) < 0) {
            write(STDERR_FILENO, strerror(errno), strlen(strerror(errno)));
            write(STDERR_FILENO, "\n", 1);
            close(fd);
            unlink(fifo_path);
            exit(-1);
        }
        
        close(fd);
        wait(NULL);
        unlink(fifo_path);
    }
    else
    {
        // Child process
        if ((fd = open(fifo_path, O_RDONLY)) < 0) 
        {
            write(STDERR_FILENO, strerror(errno), strlen(strerror(errno)));
            write(STDERR_FILENO, "\n", 1);
            exit(-1);      
        }

        if (read(fd, rd_buf, sizeof(rd_buf)) < 0) {
            write(STDERR_FILENO, strerror(errno), strlen(strerror(errno)));
            write(STDERR_FILENO, "\n", 1);
            close(fd);
            exit(-1);
        }

        write(STDOUT_FILENO, rd_buf, strlen(rd_buf));
        write(STDERR_FILENO, "\n", 1);
        close(fd);

        exit(0);
    }

    return 0;
}