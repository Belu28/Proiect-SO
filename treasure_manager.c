#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

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

void addLog(char *operation, char *huntID, int treasureID)
{
  char huntPath[PATHS_LEN];
  char logPath[PATHS_LEN];
  char symlinkPath[PATHS_LEN];
  int retval;

  if (snprintf(huntPath, sizeof(huntPath), "./%s", huntID) >= sizeof(huntPath))
  {
    perror("huntPath buffer error(too small)\n");
    return;
  }
  if (snprintf(logPath, sizeof(logPath), "%s/logged_hunt", huntPath) >= sizeof(logPath))
  {
    perror("huntPath buffer error(too small)\n");
    return;
  }

  retval = open(logPath, O_WRONLY | O_CREAT | O_APPEND, 0777);
  if (retval < 0)
  {
    perror("Failed to open log file");
    return;
  }

  if (snprintf(symlinkPath, sizeof(symlinkPath), "logged_hunt-%s", huntID) >= sizeof(symlinkPath))
  {
    perror("symlinkPath buffer error(too small)\n");
    return;
  }

  if (access(symlinkPath, F_OK) == -1)
  {
    if (symlink(logPath, symlinkPath) == -1)
    {
      perror("Failed to create symbolic link");
    }
  }

  time_t now = time(NULL);
  struct tm *tm_info = localtime(&now);
  char timeStr[64];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);

  char Entry[512];

  if (strcmp(operation, "ADD") == 0 || strcmp(operation, "VIEW") == 0 || strcmp(operation, "REMOVE-TREASURE") == 0)
  {
    if (snprintf(Entry, sizeof(Entry), "[%s] Operation: %s | Treasure ID: %d\n", timeStr, operation, treasureID) >= sizeof(Entry))
    {
      perror("Entry buffer error(too small)\n");
      return;
    }
  }
  else
  {
    if (snprintf(Entry, sizeof(Entry), "[%s] Operation: %s \n", timeStr, operation) >= sizeof(Entry))
    {
      perror("Entry buffer error(too small)\n");
      return;
    }
  }

  int len = strlen(Entry);
  if (write(retval, Entry, len) != len)
  {
    perror("Failed to write Entry Log to file");
  }

  if (close(retval) == -1)
  {
    perror("Failed to close file");
    return;
  }
}

void addTreasure(Treasure *t, char *huntID)
{
  char huntPath[PATHS_LEN];
  char filePath[PATHS_LEN];
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

  if (snprintf(filePath, sizeof(filePath), "%s/treasure.dat", huntPath) >= sizeof(filePath))
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
  getchar();

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
    if (close(retval) == -1)
    {
      perror("Failed to close file");
      return;
    }
    return;
  }

  if (close(retval) == -1)
  {
    perror("Failed to close file");
    return;
  }
  printf("Treasure added successfully.\n");
}

void listTreasures(char *huntID)
{
  char huntPath[PATHS_LEN];
  char filePath[PATHS_LEN];
  struct stat st;
  int retval;

  if (snprintf(huntPath, sizeof(huntPath), "./%s", huntID) >= sizeof(huntPath))
  {
    perror("huntPath buffer error(too small)");
    return;
  }

  if (snprintf(filePath, sizeof(filePath), "%s/treasure.dat", huntPath) >= sizeof(filePath))
  {
    perror("filePath buffer error(too small)");
    return;
  }

  if (stat(filePath, &st) == -1)
  {
    perror("Failed to get file info");
    return;
  }

  printf("HuntID: %s \n", huntID);
  printf("Total file size: %ld bytes\n", st.st_size);

  char timeStr[64];
  struct tm *tm_info = localtime(&st.st_mtime);
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
  printf("Last modified: %s\n\n", timeStr);

  retval = open(filePath, O_RDONLY);
  if (retval < 0)
  {
    perror("Failed to open treasure file");
    return;
  }

  Treasure t;
  int k = 0;
  while (read(retval, &t, sizeof(Treasure)) == sizeof(Treasure))
  {
    printf("Treasure %d:\n", ++k);
    printTreasure(&t);
    printf("\n\n\n");
  }

  if (k == 0)
  {
    printf("No treasures found.\n");
  }

  if (close(retval) == -1)
  {
    perror("Failed to close file");
    return;
  }
}

void viewTreasure(char *huntID, int treasureID)
{
  char huntPath[PATHS_LEN];
  char filePath[PATHS_LEN];
  struct stat st;
  int retval;

  if (snprintf(huntPath, sizeof(huntPath), "./%s", huntID) >= sizeof(huntPath))
  {
    perror("huntPath buffer error(too small)");
    return;
  }

  if (snprintf(filePath, sizeof(filePath), "%s/treasure.dat", huntPath) >= sizeof(filePath))
  {
    perror("filePath buffer error(too small)");
    return;
  }

  if (stat(filePath, &st) == -1)
  {
    perror("Failed to get file info");
    return;
  }

  retval = open(filePath, O_RDONLY);
  if (retval < 0)
  {
    perror("Failed to open treasure file");
    return;
  }

  Treasure t;
  int k = 0;

  while (read(retval, &t, sizeof(Treasure)) == sizeof(Treasure))
  {
    if (t.ID == treasureID)
    {
      printf("\nTreasure found:\n");
      printTreasure(&t);
      k = 1;
      break;
    }
  }

  if (k == 0)
  {
    printf("Could not find a treasure with the ID %d in hunt '%s'.\n", treasureID, huntID);
  }

  if (close(retval) == -1)
  {
    perror("Failed to close file");
  }
}

void removeTreasure(char *huntID, int treasureID)
{
  char huntPath[PATHS_LEN];
  char filePath[PATHS_LEN];
  char tempPath[PATHS_LEN]; // we create a temporary file for writing, because deleting a treasure from the middle of the file is not possible.
  int retvalr, retvalw;     // we just rewrite the file , but without the treasure we want to delete.
  int k = 0;

  if (snprintf(huntPath, sizeof(huntPath), "./%s", huntID) >= sizeof(huntPath))
  {
    perror("huntPath buffer error(too small)");
    return;
  }

  if (snprintf(filePath, sizeof(filePath), "%s/treasure.dat", huntPath) >= sizeof(filePath))
  {
    perror("filePath buffer error(too small)");
    return;
  }

  if (snprintf(tempPath, sizeof(tempPath), "%s/treasure.tmp", huntPath) >= sizeof(tempPath))
  {
    perror("tempPath buffer error(too small)");
    return;
  }

  retvalr = open(filePath, O_RDONLY);
  if (retvalr < 0)
  {
    perror("Failed to open original treasure file");
    return;
  }

  retvalw = open(tempPath, O_WRONLY | O_CREAT | O_TRUNC, 0777);
  if (retvalw < 0)
  {
    perror("Failed to create temporary file");
    if (close(retvalw) == -1)
    {
      perror("Failed to close file");
    }
    return;
  }

  Treasure t;

  while (read(retvalr, &t, sizeof(Treasure)) == sizeof(Treasure))
  {
    if (t.ID == treasureID)
    {
      k = 1;
      continue;
    }
    if (write(retvalw, &t, sizeof(Treasure)) != sizeof(Treasure))
    {
      perror("Failed to write to temporary file");
      if (close(retvalr) == -1)
      {
        perror("Failed to close file");
      }
      if (close(retvalw) == -1)
      {
        perror("Failed to close file");
      }
      return;
    }
  }

  if (close(retvalr) == -1)
  {
    perror("Failed to close file");
  }
  if (close(retvalw) == -1)
  {
    perror("Failed to close file");
  }

  if (k == 0)
  {
    printf("Could not find the treasure with the ID %d in hunt '%s'.\n", treasureID, huntID);
    unlink(tempPath);
    return;
  }

  if (unlink(filePath) == -1)
  {
    perror("Failed to delete original treasure file");
    return;
  }

  if (rename(tempPath, filePath) == -1)
  {
    perror("Failed to rename the temp file");
    return;
  }

  printf("Treasure with ID %d has been removed from hunt '%s'.\n", treasureID, huntID);
}

void removeHunt(char *huntID)
{
  char huntPath[PATHS_LEN];
  char filePath[PATHS_LEN];
  char logPath[PATHS_LEN];
  char symlinkPath[PATHS_LEN];

  if (snprintf(huntPath, sizeof(huntPath), "./%s", huntID) >= sizeof(huntPath))
  {
    perror("huntPath buffer error(too small)");
    return;
  }

  if (snprintf(filePath, sizeof(filePath), "%s/treasure.dat", huntPath) >= sizeof(filePath))
  {
    perror("filePath buffer error(too small)");
    return;
  }

  if (snprintf(logPath, sizeof(logPath), "%s/logged_hunt", huntPath) >= sizeof(logPath))
  {
    perror("logFilePath buffer error(too small)");
    return;
  }

  if (snprintf(symlinkPath, sizeof(symlinkPath), "logged_hunt-%s", huntID) >= sizeof(symlinkPath))
  {
    perror("symlinkPath buffer error(too small)");
    return;
  }

  if (unlink(filePath) == -1)
  {
    perror("Failed to remove treasure.dat");
  }

  if (unlink(logPath) == -1)
  {
    perror("Failed to remove logged_hunt");
  }

  if (rmdir(huntPath) == -1)
  {
    perror("Failed to remove hunt directory");
    return;
  }

  if (unlink(symlinkPath) == -1)
  {
    perror("Failed to remove the symbolic link");
  }

  printf("Hunt '%s' and its treasures have been removed.\n", huntID);
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
    addLog("ADD", argv[2], treasure->ID);
    free(treasure);
  }
  else if (strcmp(argv[1], "--list") == 0)
  {
    if (argc != 3)
    {
      perror("You should enter a hunt name\n");
      exit(1);
    }

    listTreasures(argv[2]);
    addLog("LIST", argv[2], 0);
  }
  else if (strcmp(argv[1], "--view") == 0)
  {
    if (argc != 4)
    {
      perror("You should enter a hunt name and a specific treasure you want to see\n");
      exit(1);
    }

    int treasureID = atoi(argv[3]);
    viewTreasure(argv[2], treasureID);
    addLog("VIEW", argv[2], treasureID);
  }
  else if (strcmp(argv[1], "--remove_treasure") == 0)
  {
    if (argc != 4)
    {
      perror("You should enter a hunt name and also the name of a specific trasure you want to remove\n");
      exit(1);
    }

    int treasureID = atoi(argv[3]);
    removeTreasure(argv[2], treasureID);
    addLog("REMOVE-TREASURE", argv[2], treasureID);
  }
  else if (strcmp(argv[1], "--remove_hunt") == 0)
  {
    if (argc != 3)
    {
      perror("You should enter a name of a hunt you want to remove\n");
      exit(1);
    }

    removeHunt(argv[2]);
    // addLog("REMOVE-HUNT", argv[2], 0);
  }
  else
  {
    printf("You've entered an unknown command\n");
  }
  return 0;
}
