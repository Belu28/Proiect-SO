#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <dirent.h>

#define PATHS_LEN 530

pid_t mpid = -1;
int mrun = 0;
int pfd[2];
int pfd2[2];

void start_monitor()
{
    if (mrun)
    {
        printf("The monitor is already running\n");
        return;
    }

    if (pipe(pfd) == -1)
    {
        perror("Failed to create pipe");
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
        close(pfd[0]);
        dup2(pfd[1], 1); // We redirect stdout to the pipe
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

void read_from_pipe()
{
    char buffer[512];

    if (close(pfd[1]) == -1)
    {
        perror("Failed to close pipe");
    }

    int bytes;
    while ((bytes = read(pfd[0], buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes] = '\0';
        printf("%s", buffer);
    }

    if (bytes == -1)
    {
        perror("Failed to read from pipe");
    }

    if (close(pfd[0]) == -1)
    {
        perror("Failed to close pipe");
    }
}

void send_command(const char *command)
{
    if (!mrun)
    {
        printf("The monitor is not running. It has to be started first\n");
        return;
    }

    if (write(pfd[1], command, strlen(command)) == -1)
    {
        perror("Failed to write command to pipe");
        return;
    }

    kill(mpid, SIGUSR1);
    sleep(2);
    read_from_pipe();
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

void calculate_score(char *HuntID)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        char huntPath[PATHS_LEN];
        if (snprintf(huntPath, sizeof(huntPath), "./%s", HuntID) >= sizeof(huntPath))
        {
            perror("HuntPath buffer error(too small)\n");
            exit(1);
        }

        if (close(pfd2[0]) == -1)
        {
            perror("Failed to close pipe");
        }
        dup2(pfd2[1], 1);
        execl("./calculate_score", "calculate_score", HuntID, NULL);
        perror("Failed to execute calculate_score");
        exit(1);
    }
    else if (pid > 0)
    {
        wait(NULL);
    }
    else
    {
        perror("Fork failed");
        return;
    }
}

void handler_calculate()
{
    DIR *dir = opendir(".");
    if (!dir)
    {
        perror("Failed to open directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        calculate_score(entry->d_name);
    }

    if (closedir(dir) == -1)
    {
        perror("Failed to close directory");
    }
}

void read_from_pipe_calculate()
{
    char buffer[512];

    if (close(pfd2[1]) == -1)
    {
        perror("Failed to close pipe");
    }

    int k;
    while ((k = read(pfd2[0], buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[k] = '\0';
        printf("Scores: %s", buffer);
    }

    if (k == -1)
    {
        perror("Error reading from pipe for calculate_score");
    }

    if (close(pfd2[0]) == -1)
    {
        perror("Failed to close pipe");
    }
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
        else if (strncmp(command, "calculate_score", 15) == 0)
        {
            handler_calculate();
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
            printf("Unknown command - Try the following commands : \n-start_monitor\n-list_hunts\n-list_treasures\n-view_treasure\n-calculate_score\n-stop_monitor\n-exit\n");
        }
    }

    return 0;
}
