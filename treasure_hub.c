#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

pid_t mpid = -1;
int mrun = 0;

void start_monitor()
{
    if (mrun)
    {
        printf("The monitor is already running\n");
        return;
    }

    mpid = fork();
    if (mpid == -1)
    {
        perror("Monitor starting failed");
        return;
    }
    else if (mpid == 0)
    {
        perror("Failed to exec monitor");
        exit(1);
    }
    else
    {
        mrun = 1;
        printf("Monitor with PID %d\n has started", mpid);
    }
}

void stop_monitor()
{
    if (!mrun)
    {
        printf("Monitor was not running.\n");
        return;
    }
    send_command("stop_monitor");
}

void handler(int sig)
{
    int status;
    waitpid(mpid, &status, 0);
    printf("Monitor exit with status %d\n", WEXITSTATUS(status));
    mrun = 0;
}

int main()
{
    struct sigaction semnal;
    semnal.sa_handler = handler;
    sigemptyset(&semnal.sa_mask); // initializam masca ca goala
    semnal.sa_flags = 0;
    sigaction(SIGCHLD, &semnal, NULL);

    char command[100];
    while (1)
    {
        printf("Hub : ");
        fgets(command, sizeof(command), stdin);
        input[strcspn(command, "\n")] = 0;

        if (strcmp(command, "start_monitor") == 0)
        {
            start_monitor();
        }
        else if (strcmp(command, "stop_monitor") == 0)
        {
            stop_monitor();
        }
        else if (strcmp(command, "exit") == 0)
        {
            if (mrun)
            {
                printf("The monitor is running, you cannot exit.\n");
            }
            else
            {
                printf("Exiting.\n");
                break;
            }
        }
        else
        {
            printf("Unknown command - Try the following commands : \n-start_monitor\n-list_hunts\n-list_treasures\n-view_treasure\n-stop_monitor\n-exit\n");
        }
    }

    return 0;
}
