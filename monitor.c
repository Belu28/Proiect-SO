#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

void handler(int sig)
{
    int retval = open("monitorcommands.txt", O_RDONLY);
    if (retval < 0)
    {
        perror("Failed to open command file");
        return;
    }
}

int main()
{
    struct sigaction semnal;
    semnal.sa_handler = handler;
    sigemptyset(&semnal.sa_mask);
    semnal.sa_flags = 0;
    sigaction(SIGUSR1, &semnal, NULL);

    return 0;
}