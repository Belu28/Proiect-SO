#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

typedef struct Treasure
{
  char ID[30];
  char username[30];
  float latitude;
  float longitude;
  char clue[100];
  int value;
} Treasure;

void makeDirectory(char huntID[])
{
}

void addTreasure(char huntID[])
{
  makeDirectory(huntID);
  Treasure t;

  printf("Enter treasure ID:");
  scanf("%d", &t.ID);
  printf("Enter username:");
  scanf("%30s", &t.username);
  printf("Enter latitude:");
  scanf("%f", &t.latitude);
  printf("Enter longitude:");
  scanf("%f", &t.longitude);
  printf("Enter clue:");
  scanf("%100s", &t.clue);
  printf("Enter value:");
  scanf("%d", &t.value);
}

int main(void)
{
  printf("adada");
  return 0;
}
