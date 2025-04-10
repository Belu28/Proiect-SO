#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define USERNAME_LEN 50
#define CLUE_LEN 150
#define PATHS_LEN 530
#define HUNTID_LEN (PATHS_LEN - 20)

typedef struct Treasure
{
  int ID;
  char username[USERNAME_LEN];
  float latitude;
  float longitude;
  char clue[CLUE_LEN];
  int value;
} Treasure;

void printTreasure(Treasure *t)
{
  printf("Treasure ID: %d\n", t->ID);
  printf("Username: %s\n", t->username);
  printf("Latitude : %.3f --- Longitude : %.3f\n", t->latitude, t->longitude);
  printf("Clue Text: %s\n", t->clue);
  printf("Value: %d\n", t->value);
}

void addTreasure(Treasure *t, char *huntID)
{
  char huntPath[PATHS_LEN];
  char filePath[PATHS_LEN];
  char fullPath[PATHS_LEN];
  char symlinkPath[PATHS_LEN];
  int retval;

  if (strlen(huntID) > HUNTID_LEN)
  {
    perror("Hunt ID too long\n");
    return;
  }

  if (snprintf(huntPath, sizeof(huntPath), "./%s", huntID) >= sizeof(huntPath))
  {
    perror("huntPath buffer error(too small)\n");
    return;
  }

  if (mkdir(huntPath, 0777) == -1 && errno != EEXIST) // We check if mkdir() provides an error while making a directory, and if it is not because the directory already exists, we signal it
  {
    perror("Failed to create hunt directory");
    return;
  }

  if (snprintf(filePath, sizeof(filePath), "%s/treasure.dat", huntPath) >= sizeof(filePath)) // .dat? or .txt .csv etc
  {
    perror("filePath buffer error(too small)\n");
    return;
  };

  retval = open(filePath, O_WRONLY | O_CREAT | O_APPEND, 0777);

  if (retval < 0) // Checking the open() function return value
  {
    perror("Failed to open treasure file");
    return;
  }

  printf("Enter Treasure ID: ");
  scanf("%d", &t->ID);
  getchar(); // for flushing

  printf("Enter Username: ");
  fgets(t->username, USERNAME_LEN, stdin);
  t->username[strcspn(t->username, "\n")] = 0;

  printf("Enter Latitude: ");
  scanf("%f", &t->latitude);
  printf("Enter Longitude: ");
  scanf("%f", &t->longitude);
  getchar();

  printf("Enter Clue: ");
  fgets(t->clue, CLUE_LEN, stdin);
  t->clue[strcspn(t->clue, "\n")] = 0;

  printf("Enter Value: ");
  scanf("%d", &t->value);

  if (write(retval, t, sizeof(Treasure)) != sizeof(Treasure))
  {
    perror("Failed to write treasure to file");
    close(retval);
    return;
  }

  close(retval);
  printf("Treasure added successfully.\n");

  if (snprintf(fullPath, sizeof(fullPath), "%s/hunt_notes", huntPath) >= sizeof(fullPath))
  {
    perror("fullPath overflow\n");
    return;
  }

  if (snprintf(symlinkPath, sizeof(symlinkPath), "hunt_notes-%s", huntID) >= sizeof(symlinkPath))
  {
    perror("symlinkPath overflow\n");
    return;
  }

  unlink(symlinkPath); // If a previous symlink exists, we unlink it.

  if (symlink(fullPath, symlinkPath) == -1)
  {
    perror("Failed to create symbolic link");
  }
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    perror("Too few arguments\n\nCorect call : treasure_manager <flag> \n\n Possible flags:\n--add <hunt id>\n--list <hunt id>\n--view <hunt id> <id>\n--remove_treasure <hunt id> <id>\n--remove_hunt <hunt id>\n");
    exit(1);
  }

  Treasure *treasure = NULL;

  if (strcmp(argv[1], "--add") == 0)
  {
    if (argc != 3)
    {
      perror("You should enter a hunt name\n");
      exit(1);
    }
    if (treasure == NULL)
    {
      if ((treasure = (Treasure *)malloc(sizeof(Treasure))) == NULL)
      {
        perror("Memory allocation failure for -treasure- \n");
        exit(1);
      }
    }

    addTreasure(treasure, argv[2]);
    // addLogs(op,argv[2],NULL);
  }
  else if (strcmp(argv[1], "--list") == 0)
  {
    if (argc != 3)
    {
      perror("You should enter a hunt name\n");
      exit(1);
    }

    // listTreasure(argv[2]);
    //  addLogs(op,argv[2],NULL);
  }
  else if (strcmp(argv[1], "--view") == 0)
  {
    if (argc != 4)
    {
      perror("You should enter a hunt name and a specific treasure you want to see\n");
      exit(1);
    }

    // viewHunt(argv[2], argv[3]);
    //  addLogs(op, argv[2], argv[3]);
  }
  else if (strcmp(argv[1], "--remove_treasure") == 0)
  {
    if (argc != 4)
    {
      perror("You should enter a hunt name and also the name of a specific trasure you want to remove\n");
      exit(1);
    }

    // removeTreasure(argv[2], argv[3]);
    //  addLogs(op, argv[2], argv[3]);
  }
  else if (strcmp(argv[1], "--remove_hunt") == 0)
  {
    if (argc != 3)
    {
      perror("You should enter a name of a hunt you want to remove\n");
      exit(1);
    }

    // removeHunt(argv[2]);
  }
  else
  {
    printf("You've entered an unknown command\n");
  }

  return 0;
}
