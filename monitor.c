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

    char command[256] = {0};

    ssize_t nrb = read(retval, command, sizeof(command) - 1);
    if (nrb < 0)
    {
        perror("Failed to read command");
        if (close(retval) == -1)
        {
            perror("Failed to close file");
        }
        return;
    }

    command[nrb] = '\0';

    if (close(retval) == -1)
    {
        perror("Failed to close file");
        return;
    }

    if (strncmp(command, "list_hunts", 10) == 0)
    {
        printf("List of hunts:\n");
        system("ls -d Hunt*"); // We print only the directories that start with Hunt
    }
    else if (strncmp(command, "list_treasures", 14) == 0)
    {
        char huntName[100];
        sscanf(command, "list_treasures %s", huntName);
        pid_t pid = fork();
        if (pid == 0)
        {
            execl("./treasure_manager", "treasure_manager", "--list", huntName, NULL);
            perror("execl failed");
            exit(1);
        }
        else if (pid > 0)
        {
            wait(NULL);
        }
        else
        {
            perror("fork failed");
        }
    }
    else if (strncmp(command, "view_treasure", 13) == 0)
    {
        char huntName[100];
        int trsid;
        sscanf(command, "view_treasure %s %d", huntName, &trsid);
        char treasureID[12];
        snprintf(treasureID, sizeof(treasureID), "%d", trsid);
        pid_t pid = fork();
        if (pid == 0)
        {
            execl("./treasure_manager", "treasure_manager", "--view", huntName, treasureID, NULL);
            perror("execl failed");
            exit(1);
        }
        else if (pid > 0)
        {
            wait(NULL);
        }
        else
        {
            perror("fork failed");
        }
    }
    else if (strncmp(command, "stop_monitor", 12) == 0)
    {
        printf("Stopping monitor..\n");
        sleep(1);
        exit(0);
    }
    else
    {
        printf("Unknown command : - %s\n Try the following commands : \n-start_monitor\n-list_hunts\n-list_treasures\n-view_treasure\n-stop_monitor\n-exit\n", command);
    }
}

int main()
{
    struct sigaction semnal;
    semnal.sa_handler = handler;
    sigemptyset(&semnal.sa_mask);
    semnal.sa_flags = 0;
    sigaction(SIGUSR1, &semnal, NULL);

    printf("The Monitor is ready and waiting for commands.\n");

    while (1)
    {
        pause();
    }
    return 0;
}