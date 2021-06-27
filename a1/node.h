#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_BUFFER 80

struct Node
{
    char name[MAX_BUFFER];
    int messages;
    int current;
};

struct Node *Node(char[]);

char *getName(struct Node *node);

int getMessages(struct Node *node);

int getCurrent(struct Node *node);

void incrementMessage(struct Node *node);

void incrementCurrent(struct Node *node);

int checkName(char *x, char *y);

void updateDeleteCounter(struct Node *node);
