#include "node.h"

struct Node *Node(char id[])
{
    struct Node *temp = (struct Node *)malloc(sizeof(struct Node *));
    strcpy(temp->name, id);
    temp->messages = 0;
    temp->current = 0;
    return temp;
}

char *getName(struct Node *node)
{
    char *c;
    c = (char *)malloc(MAX_BUFFER);
    strcpy(c, node->name);
    return c;
}

int getMessages(struct Node *node)
{
    return node->messages;
}

int getCurrent(struct Node *node)
{
    return node->current;
}

void incrementMessage(struct Node *node)
{
    int temp = node->messages;
    node->messages = temp + 1;
}

void incrementCurrent(struct Node *node)
{
    int curr = node->current;
    int total = node->messages;
    node->current = (curr + 1) % total;
}

int checkName(char *x, char *y)
{
    if (strlen(x) != strlen(y))
        return 0;
    if (strcmp(x, y) == 0)
        return 1;
    return 0;
}

void updateDeleteCounter(struct Node *node)
{
    int curr = node->current;
    int total = node->messages;
    node->messages = total - 1;
    if (total == 1)
    {
        node->current = 0;
    }
    else
    {
        node->current = curr % (total - 1);
    }
}