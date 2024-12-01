// POSIX.1-2000

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

void sighandler(int signal)
{
    printf("Handle SIGUSR1 %u\n", getpid());
    exit(0);
}

int main (int argc, char* argv[])
{
    pid_t cpid;

    struct sigaction sa;
    sa.sa_handler = sighandler;
    sa.sa_flags = 0;

    if (sigaction(SIGUSR1, &sa, NULL) == -1) 
    {
        perror("Error setting signal handler");
        exit(EXIT_FAILURE);
    }

    if ((cpid = fork()) < 0)
    {
        printf("Error of creating child process!\n");
        exit(EXIT_FAILURE);
    }

    if (cpid)
    {
        // Parent process section. Process contains child process PID
        if (kill(cpid, SIGUSR1) != 0)
            printf("Error while calling SIGUSR1!\n");
    }
    else
        // Child process
        while(1)
            ;

    int status;
    waitpid(cpid, &status, 0);
    if (WIFEXITED(status)) {
        printf("Child process %u exit with code %u\n", cpid, WEXITSTATUS(status));
    } else {
        printf("Unexpected termination of a child process %u\n", cpid);
    }

    return 0;
}
