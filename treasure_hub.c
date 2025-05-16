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
        execl("./monitor", "./monitor", NULL);
        perror("Failed to exec monitor");
        exit(1);
    }
    else
    {
        mrun = 1;
        printf("Monitor with PID %d has started\n", mpid);
    }
}

void send_command(const char *command)
{
    if (!mrun)
    {
        printf("The monitor is not running. It has to be started first\n");
        return;
    }

    int retval = open("monitorcommands.txt", O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (retval < 0)
    {
        perror("Failed to open command file");
        return;
    }
    write(retval, command, strlen(command));
    if (close(retval) == -1)
    {
        perror("Failed to close file");
        return;
    }

    kill(mpid, SIGUSR1);
    sleep(1);
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
    sigemptyset(&semnal.sa_mask); // We initialize the mask to empty
    semnal.sa_flags = 0;
    sigaction(SIGCHLD, &semnal, NULL);

    char command[100];
    while (1)
    {
        printf("Hub : ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "start_monitor") == 0)
        {
            start_monitor();
        }
        else if (strncmp(command, "list_hunts", 10) == 0)
        {
            send_command("list_hunts");
        }
        else if (strncmp(command, "list_treasures", 14) == 0) // we only verify the first word (in this case list_treasures) to be the same, and if so, we then send all input, including the name of a hunt
        {
            send_command(command);
        }
        else if (strncmp(command, "view_treasure", 13) == 0)
        {
            send_command(command);
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
