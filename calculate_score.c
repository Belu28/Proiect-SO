#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define USERNAME_LEN 50
#define CLUE_LEN 150

typedef struct Treasure
{
    int ID;
    char username[USERNAME_LEN];
    float latitude;
    float longitude;
    char clue[CLUE_LEN];
    int value;
} Treasure;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <treasure.dat>\n", argv[0]);
        return 1;
    }

    char *treasurePath = argv[1];

    int retval = open(treasurePath, O_RDONLY);
    if (retval < 0)
    {
        perror("Failed to open treasure file");
        return 1;
    }

    Treasure t;
    int users[100] = {0}; // We assume a maximum of 100 users
    char usernames[100][USERNAME_LEN];
    int k = 0;
    int nr = 0;

    while ((k = read(retval, &t, sizeof(Treasure))) == sizeof(Treasure))
    {
        int ok = 0;

        for (int i = 0; i < nr; i++)
        {
            if (strcmp(usernames[i], t.username) == 0)
            {
                users[i] = users[i] + t.value;
                ok = 1;
                break;
            }
        }

        if (ok == 0)
        {
            strcpy(usernames[nr], t.username);
            users[nr] = t.value;
            nr++;
        }
    }

    if (k == -1)
    {
        perror("Failed to read treasure file");
        if (close(retval) == -1)
        {
            perror("Failed to close file");
        }
        return 1;
    }

    if (close(retval) == -1)
    {
        perror("Failed to close file");
    }

    for (int i = 0; i < nr; i++)
    {
        printf("User: %s, Total Score: %d\n", usernames[i], users[i]);
    }

    return 0;
}