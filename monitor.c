#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/wait.h>

#define USERNAME_LEN 50
#define CLUE_LEN 150
#define PATHS_LEN 530

int pfd[2];
typedef struct Treasure
{
    int ID;
    char username[USERNAME_LEN];
    float latitude;
    float longitude;
    char clue[CLUE_LEN];
    int value;
} Treasure;

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
        char *path = "."; // We put the path as the current directory, because the hunts are in the here
        DIR *dir = opendir(path);
        if (!dir)
        {
            perror("Failed to open hunts directory");
            return;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) // We skip the current and parent directory so we only get the hunts.
            {
                continue;
            }
            char treasure_path[512];
            snprintf(treasure_path, sizeof(treasure_path), "%s/%s/treasure.dat", path, entry->d_name);

            int retval = open(treasure_path, O_RDONLY);
            if (retval == -1)
            {
                continue;
            }

            int count = 0;
            Treasure t;

            ssize_t bytes;
            while ((bytes = read(retval, &t, sizeof(Treasure))) == sizeof(Treasure))
            {
                count++;
            }

            if (close(retval) == -1)
            {
                perror("Failed to close file");
            }

            char result[512];
            snprintf(result, sizeof(result), "Hunt: %s - Total Treasures: %d\n", entry->d_name, count);
            write(pipefd[1], result, strlen(result));
        }

        if (closedir(dir) == -1)
        {
            perror("Failed to close directory");
        }
    }
    else if (strncmp(command, "list_treasures", 14) == 0)
    {
        char huntName[100];
        sscanf(command, "list_treasures %s", huntName);

        char treasurePath[PATHS_LEN] ș if (snprintf(treasurePath, sizeof(treasurePath), "./%s/treasure.dat", huntName))
        {
            perror("treasurePath buffer error(too small)");
            return;
        }
        int retval = open(treasurePath, O_RDONLY);
        if (retval < 0)
        {
            perror("Failed to open treasure file");
            return;
        }

        Treasure t;
        int k;
        while ((k = read(retval, &t, sizeof(Treasure))) == sizeof(Treasure))
        {
            char result[512];
            snprintf(result, sizeof(result), "Treasure ID: %d\nUsername: %s\nLatitude: %.2f\nLongitude: %.2f\nClue: %s\nValue: %d\n", t.ID, t.username, t.latitude, t.longitude, t.clue, t.value);
            write(pipefd[1], result, strlen(result));
        }

        if (close(retval) == -1)
        {
            perror("Failed to close file");
        }
    }
    else if (strncmp(command, "view_treasure", 13) == 0)
    {
        char huntName[100];
        int trsid;
        sscanf(command, "view_treasure %s %d", huntName, &trsid);

        char treasurePath[PATHS_LEN];
        if (snprintf(treasurePath, sizeof(treasurePath), "./%s/treasure.dat", huntName) >= sizeof(treasurePath))
        {
            perror("treasurePath buffer error(too small)");
            return;
        }

        int retval = open(treasurePath, O_RDONLY);
        if (retval < 0)
        {
            perror("Failed to open treasure file");
            return;
        }

        Treasure t;
        int k = 0;
        int ok = 0;

        while ((k = read(retval, &t, sizeof(Treasure))) == sizeof(Treasure))
        {
            if (t.ID == trsid)
            {
                char result[512];
                snprintf(result, sizeof(result), "Treasure ID: %d\nUsername: %s\nLatitude: %.2f\nLongitude: %.2f\nClue: %s\nValue: %d\n", t.ID, t.username, t.latitude, t.longitude, t.clue, t.value);
                write(pipefd[1], result, strlen(result));
                ok = 1;
                break;
            }
        }

        if (ok == 0)
        {
            char result[512];
            snprintf(result, sizeof(result), "Treasure with ID %d not found in hunt '%s'.\n", trsid, huntName);
            write(pipefd[1], result, strlen(result));
        }

        if (close(retval) == -1)
        {
            perror("Failed to close file");
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
    if (pipe(pfd) == -1)
    {
        perror("Failed to create pipe");
        return 1;
    }

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