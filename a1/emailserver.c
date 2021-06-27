#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include "node.c"
#define MAX_BUFFER 80
#define MAX_FILES 10
#define PORT 8080

void error(const char *message)
{
    perror(message);
    exit(1);
}

// Function designed for chat between client and server.
void func(int newsockfd)
{
    char buffer[MAX_BUFFER], userid[MAX_BUFFER];
    int n;

    struct Node *files[MAX_FILES];
    struct Node *currentFile = NULL;

    for (n = 0; n < MAX_FILES; n++)
    {
        files[n] = NULL;
    }

    FILE *fp;

    // infinite loop for chat
    for (;;)
    {
        bzero(buffer, MAX_BUFFER);
        bzero(userid, MAX_BUFFER);

        // read the message from client and copy it in buffer
        read(newsockfd, buffer, sizeof(buffer));

        printf("Read buffer from client: %s\n", buffer);
        fflush(stdout);

        if (strncmp("LSTU", buffer, 4) == 0)
        {
            bzero(buffer, MAX_BUFFER);
            for (n = 0; n < MAX_FILES; n++)
            {
                if (files[n])
                {
                    strcat(buffer, getName(files[n]));
                    n++;
                    break;
                }
            }
            for (; n < MAX_FILES; n++)
            {
                if (files[n])
                {
                    strcat(buffer, ", ");
                    strcat(buffer, getName(files[n]));
                }
            }
            // strncpy(buffer, "SUCCESS: Listing all the user ids", 34);
            write(newsockfd, buffer, sizeof(buffer));
        }
        else if (strncmp("ADDU ", buffer, 5) == 0)
        {
            n = 5;
            while (n < strlen(buffer) && buffer[n] != '\0' && buffer[n] != '\n')
            {
                userid[n - 5] = buffer[n];
                n++;
            }
            userid[n - 5] = '\0';

            bzero(buffer, MAX_BUFFER);

            for (n = 0; n < MAX_FILES; n++)
            {
                if (files[n] && checkName(files[n]->name, userid))
                    break;
            }
            if (n != MAX_FILES)
            {
                strcpy(buffer, "ERROR: Userid already present");
            }
            else
            {
                for (n = 0; n < MAX_FILES; n++)
                {
                    if (!files[n])
                        break;
                }
                if (n == MAX_FILES)
                {
                    strcpy(buffer, "ERROR: MAX FILES reached. Cannot add more files");
                }
                else
                {
                    files[n] = Node(userid);
                    // Create a file of name name.nxt
                    char *name = getName(files[n]);
                    fp = fopen(strcat(name, ".nxt"), "w");
                    fclose(fp);
                    strcpy(buffer, "SUCCESS: User id was succesfully added ");
                    strcat(buffer, userid);
                }
            }
            write(newsockfd, buffer, sizeof(buffer));
        }
        else if (strncmp("USER ", buffer, 4) == 0)
        {
            n = 5;
            while (n < strlen(buffer) && buffer[n] != '\0' && buffer[n] != '\n')
            {
                userid[n - 5] = buffer[n];
                n++;
            }
            userid[n - 5] = '\0';

            bzero(buffer, MAX_BUFFER);

            for (n = 0; n < MAX_FILES; n++)
            {
                if (files[n] && checkName(getName(files[n]), userid))
                    break;
            }
            if (n == MAX_FILES)
            {
                strcpy(buffer, "ERROR: No such file found with name = ");
            }
            else
            {
                currentFile = files[n];
                strcpy(buffer, "SUCCESS: ");
            }
            strcat(buffer, userid);
            write(newsockfd, buffer, sizeof(buffer));
        }
        else if (strncmp("READM", buffer, 5) == 0)
        {
            bzero(buffer, MAX_BUFFER);
            if (currentFile == NULL)
            {
                strcpy(buffer, "ERROR: Current file is NULL");
            }
            else
            {
                if (getMessages(currentFile) == 0)
                {
                    strcpy(buffer, "No More Mail");
                }
                else
                {
                    char *name = getName(currentFile);
                    int pointer = getCurrent(currentFile);
                    char c;
                    int hashCount;
                    fp = fopen(strcat(name, ".nxt"), "r");
                    // Assuming the pointer is valid and not out of bound

                    // skip this message block
                    for (n = 0; n < pointer; n++)
                    {
                        hashCount = 0;
                        while ((c = fgetc(fp)) != EOF)
                        {
                            if (c == '#')
                                hashCount++;
                            else
                                hashCount = 0;
                            if (hashCount == 3)
                                break;
                        }
                    }
                    // Ignoring the new line
                    c = fgetc(fp);
                    // adding this block in buffer
                    while ((c = fgetc(fp)) != EOF)
                    {
                        strncat(buffer, &c, 1);
                        if (c == '#')
                            hashCount++;
                        else
                            hashCount = 0;
                        if (hashCount == 3)
                            break;
                    }
                    incrementCurrent(currentFile);
                    fclose(fp);
                }
            }
            write(newsockfd, buffer, sizeof(buffer));
        }
        else if (strncmp("DELM", buffer, 4) == 0)
        {
            bzero(buffer, MAX_BUFFER);
            if (currentFile == NULL)
            {
                strcpy(buffer, "ERROR: Current file is NULL");
            }
            else
            {
                if (getMessages(currentFile) == 0)
                {
                    strcpy(buffer, "No More Mail");
                }
                else
                {
                    char *name = getName(currentFile);
                    int pointer = getCurrent(currentFile);
                    int total = getMessages(currentFile);
                    char c;
                    int hashCount;
                    FILE *tp = fopen("tempfile", "w");
                    fp = fopen(strcat(name, ".nxt"), "r");
                    // Assuming the pointer is valid and not out of bound

                    // Add all the messages before the pointer
                    for (n = 0; n < pointer; n++)
                    {
                        hashCount = 0;
                        while ((c = fgetc(fp)) != EOF)
                        {
                            fputc(c, tp);
                            if (c == '#')
                                hashCount++;
                            else
                                hashCount = 0;
                            if (hashCount == 3)
                                break;
                        }
                    }

                    // skip one message - pointer message
                    hashCount = 0;
                    while ((c = fgetc(fp)) != EOF)
                    {
                        if (c == '#')
                            hashCount++;
                        else
                            hashCount = 0;
                        if (hashCount == 3)
                            break;
                    }
                    // The new line character will be at the file pointer now hence no need to add again.
                    if (pointer == 0)
                    {
                        // corner case where we have to ignore the new line as it shouldn't be printed
                        c = fgetc(fp);
                    }

                    // Add all the remaining messages.
                    for (n = pointer + 1; n < total; n++)
                    {
                        hashCount = 0;
                        while ((c = fgetc(fp)) != EOF)
                        {
                            fputc(c, tp);
                            if (c == '#')
                                hashCount++;
                            else
                                hashCount = 0;
                            if (hashCount == 3)
                                break;
                        }
                    }
                    fputc('\n', tp);
                    fclose(tp);
                    fclose(fp);
                    if (remove(name) == 0)
                    {
                        // printf("Removed filed successfully\n");
                        if (rename("tempfile", name) == 0)
                        {
                            // printf("Renamed successfully\n");
                            updateDeleteCounter(currentFile);
                            strcpy(buffer, "Message Deleted");
                        }
                        else
                        {
                            strcpy(buffer, "Couldn't rename temp file\n");
                        }
                    }
                    else
                    {
                        strcpy(buffer, "Couldn't remove file\n");
                    }
                }
            }
            write(newsockfd, buffer, sizeof(buffer));
        }
        else if (strncmp("SEND ", buffer, 5) == 0)
        {
            if (currentFile == NULL)
            {
                bzero(buffer, MAX_BUFFER);
                strcpy(buffer, "ERROR: Current file is NULL");
            }
            else
            {
                n = 5;
                while (n < strlen(buffer) && buffer[n] != '\0' && buffer[n] != '\n' && buffer[n] != ' ')
                {
                    userid[n - 5] = buffer[n];
                    n++;
                }
                userid[n - 5] = '\0';

                char message[MAX_BUFFER];
                int i = 0;
                n++;
                while (n < strlen(buffer) && buffer[n] != '\0')
                {
                    message[i] = buffer[n];
                    n++;
                    i++;
                }
                message[i] = '\0';

                bzero(buffer, MAX_BUFFER);

                for (n = 0; n < MAX_FILES; n++)
                {
                    if (files[n] && checkName(files[n]->name, userid))
                        break;
                }
                if (n == MAX_FILES)
                {
                    strcpy(buffer, "ERROR: Receiver id is incorrect and such file does not exists ");
                }
                else
                {
                    strcpy(buffer, "From: ");
                    strcat(buffer, getName(currentFile));
                    strcat(buffer, "\nTo: ");
                    strcat(buffer, userid);
                    strcat(buffer, "\nDate: ");

                    time_t t;
                    time(&t);
                    strcat(buffer, ctime(&t));
                    strcat(buffer, message);

                    char name[MAX_BUFFER];
                    strcpy(name, userid);
                    fp = fopen(strcat(name, ".nxt"), "a");
                    if (fprintf(fp, "%s\n", buffer) < 0)
                    {
                        printf("Error in file print\n");
                    }
                    else
                    {
                        printf("Success in print\n");
                        incrementMessage(files[n]);
                    }
                    fclose(fp);
                    fflush(stdout);

                    bzero(buffer, MAX_BUFFER);
                    strcpy(buffer, "SUCCESS: Send done from the receiver id ");
                }
                strcat(buffer, userid);
            }
            write(newsockfd, buffer, sizeof(buffer));
        }
        else if (strncmp("DONEU", buffer, 4) == 0)
        {
            currentFile = NULL;
            bzero(buffer, MAX_BUFFER);
            strcpy(buffer, "SUCCESS: Done");
            write(newsockfd, buffer, sizeof(buffer));
        }
        else if (strncmp("QUIT", buffer, 4) == 0)
        {
            // clear data sturctures
            for (n = 0; n < MAX_FILES; n++)
            {
                if (files[n] != NULL)
                {
                    char *name = getName(files[n]);
                    if (remove(strcat(name, ".nxt")) == 0)
                        printf("Deleted file successfully\n");
                    else
                        printf("Unable to delete the file\n");
                    free((void *)files[n]);
                }
            }
            printf("QUIT\n");
            fflush(stdout);
            bzero(buffer, MAX_BUFFER);
            strcpy(buffer, "SUCCESS: Quiting the client\n");
            write(newsockfd, buffer, sizeof(buffer));
        }
        else
        {
            printf("Illegal input to the server");
            bzero(buffer, MAX_BUFFER);
            strcpy(buffer, "ERROR: Incorrect input from client. Please type again");
            write(newsockfd, buffer, sizeof(buffer));
        }

        if (strncmp("exit", buffer, 4) == 0)
        {
            printf("Server Exit...\n");
            break;
        }
        fflush(stdout);
    }
}

// Driver function
int main()
{
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    // socket file descriptor create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        error("socket Creation Failed...\n");
    }
    else
        printf("Socket Successfully Created..\n");

    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    {
        error("socket Bind Failed...\n");
    }
    else
        printf("Socket Successfully Binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0)
    {
        error("Listen Failed...\n");
    }
    else
        printf("Server listening..\n");

    len = sizeof(cli);

    // Accept the data packet from client and verification
    connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
    if (connfd < 0)
    {
        error("Server Acccept Failed...\n");
    }
    else
        printf("Server Accept Client Successful...\n");

    // Function for chatting between client and server
    func(connfd);

    // After chatting close the socket
    close(sockfd);
}