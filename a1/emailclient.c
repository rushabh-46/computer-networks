#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#define MAX_BUFFER 80
#define PORT 8080
#define SA struct sockaddr

void w_and_r(int sockfd, char buffer[])
{
    write(sockfd, buffer, MAX_BUFFER);
    bzero(buffer, MAX_BUFFER);
    read(sockfd, buffer, MAX_BUFFER);
}

void getThreeHash(char *message)
{
    int n = 0;
    int hashCount = 0;
    while ((n < MAX_BUFFER - 1) && !(((message[n] = getchar()) == '#') && (hashCount == 2)))
    {
        if (message[n] == '#')
            hashCount++;
        else
            hashCount = 0;
        n++;
    }
    message[n + 1] = '\0';
    while (getchar() != '\n')
        ;
    return;
}

/**
 * The network interface for the client-side communication.
 * It sends the message inside the buffer to the server and gets the corresponding 
 * output from the server which is read in the buffer
 * @param sockid - socket file desciptor
 * @param buffer - message string
 */
int network_interface(int sockfd, char buffer[], int instr)
{
    int i, n;
    char userid[MAX_BUFFER];

    fflush(stdout);

    switch (instr)
    {
    case 0: // Listusers
        bzero(buffer, MAX_BUFFER);
        strcpy(buffer, "LSTU");
        w_and_r(sockfd, buffer);
        break;

    case 1: // Adduser <userid>
        i = 7;
        // Ignore white spaces and one word for user
        while (i < strlen(buffer) && buffer[i] == ' ')
        {
            i++;
        }
        n = 0;
        while (i < strlen(buffer) && buffer[i] != '\0' && buffer[i] != ' ' && buffer[i] != '\n')
        {
            userid[n] = buffer[i];
            i++;
            n++;
        }
        userid[n] = '\0';
        bzero(buffer, MAX_BUFFER);
        if (n == 0)
        {
            // empty user id -> not allowed
            strcpy(buffer, "ERROR: Empty filename not allowed.");
            break;
        }
        strcpy(buffer, "ADDU ");
        strcat(buffer, userid);
        w_and_r(sockfd, buffer);
        break;

    case 2: // SetUser <userid>
        i = 7;
        // Ignore white spaces and one word for user
        while (i < strlen(buffer) && buffer[i] == ' ')
        {
            i++;
        }
        n = 0;
        while (i < strlen(buffer) && buffer[i] != '\0' && buffer[i] != ' ' && buffer[i] != '\n')
        {
            userid[n] = buffer[i];
            i++;
            n++;
        }
        userid[n] = '\0';
        bzero(buffer, MAX_BUFFER);
        if (n == 0)
        {
            // empty user id -> not allowed
            strcpy(buffer, "ERROR: Empty filename not allowed.");
            break;
        }
        strcpy(buffer, "USER ");
        strcat(buffer, userid);
        w_and_r(sockfd, buffer);
        if (strncmp(buffer, "ERROR: ", 7) == 0)
            return -1;

        break;

    case 3: // Read
        bzero(buffer, MAX_BUFFER);
        strcpy(buffer, "READM");
        w_and_r(sockfd, buffer);
        break;

    case 4: // Delete
        bzero(buffer, MAX_BUFFER);
        strcpy(buffer, "DELM");
        w_and_r(sockfd, buffer);
        break;

    case 5: // Send <receiverid>
        i = 4;
        // Ignore white spaces and one word for user
        while (i < strlen(buffer) && buffer[i] == ' ')
        {
            i++;
        }
        n = 0;

        while (i < strlen(buffer) && buffer[i] != '\0' && buffer[i] != ' ' && buffer[i] != '\n')
        {
            userid[n] = buffer[i];
            i++;
            n++;
        }
        userid[n] = '\0';
        if (n == 0)
        {
            // empty user id -> not allowed
            bzero(buffer, MAX_BUFFER);
            strcpy(buffer, "ERROR: Empty filename not allowed.");
            break;
        }

        char message[MAX_BUFFER];
        printf("Type message: ");
        getThreeHash(message);

        bzero(buffer, MAX_BUFFER);
        strcpy(buffer, "SEND ");
        strcat(buffer, userid);
        strcat(buffer, " ");
        strcat(buffer, message);

        w_and_r(sockfd, buffer);
        break;

    case 6: // Done
        bzero(buffer, MAX_BUFFER);
        strcpy(buffer, "DONEU");
        w_and_r(sockfd, buffer);
        break;

    case 7: // Quit
        bzero(buffer, MAX_BUFFER);
        strcpy(buffer, "QUIT");
        w_and_r(sockfd, buffer);
        break;

    default:
        w_and_r(sockfd, buffer);
    }

    // print the server response
    printf("%s\n", buffer);
    return 0;
}

/**
 * The user input interface for the client-side communication.
 * @param sockid - socket file descriptor
 */
void func(int sockfd)
{
    char buffer[MAX_BUFFER];
    int n;
    // main-prompt => prompt==0
    // sub-prompt  => prompt==1
    int prompt = 0;
    char subuser[MAX_BUFFER];

    for (;;)
    {
        bzero(buffer, MAX_BUFFER);
        if (prompt == 0)
        {
            printf("Main-Prompt> ");
        }
        else
        {
            printf("Sub-Prompt-%s> ", subuser);
        }

        n = 0;
        while ((n < MAX_BUFFER - 1) && ((buffer[n++] = getchar()) != '\n'))
            ;
        buffer[n] = '\0';

        if (prompt == 0 && (strncmp("Listusers", buffer, 9) == 0))
        {
            network_interface(sockfd, buffer, 0);
        }
        else if (prompt == 0 && (strncmp("Adduser ", buffer, 8) == 0))
        {
            network_interface(sockfd, buffer, 1);
        }
        else if (prompt == 0 && (strncmp("SetUser ", buffer, 8) == 0))
        {
            if (network_interface(sockfd, buffer, 2) == 0)
            {
                prompt = 1;
                // SUCCESS: <subuserid>
                n = 9;
                bzero(subuser, MAX_BUFFER);
                while (n < strlen(buffer) && buffer[n] != '\0' && buffer[n] != '\n')
                {
                    subuser[n - 9] = buffer[n];
                    n++;
                }
                subuser[n] = '\0';
            }
            else
            {
                printf("Failed to set to sub prompt\n");
            }
        }
        else if (prompt == 1 && (strncmp("Read", buffer, 4) == 0))
        {
            network_interface(sockfd, buffer, 3);
        }
        else if (prompt == 1 && (strncmp("Delete", buffer, 6) == 0))
        {
            network_interface(sockfd, buffer, 4);
        }
        else if (prompt == 1 && (strncmp("Send ", buffer, 5) == 0))
        {
            network_interface(sockfd, buffer, 5);
        }
        else if (prompt == 1 && (strncmp("Done", buffer, 4) == 0))
        {
            if (network_interface(sockfd, buffer, 6) == 0)
            {
                prompt = 0;
            }
            else
            {
                // Not reachable
                printf("Failed to set to main prompt\n");
            }
        }
        else if (prompt == 0 && (strncmp("Quit", buffer, 4) == 0))
        {
            network_interface(sockfd, buffer, 7);
            break;
        }
        else
        {
            printf("ERROR: Incorrect input. Please try again\n");
            fflush(stdout);
        }
    }
    return;
}

int main()
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Socket Creation Failed...\n");
        exit(0);
    }
    else
    {
        printf("Socket Successfully Created..\n");
    }
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("Connection with the Server Failed...\n");
        exit(0);
    }
    else
    {
        printf("Connected to the Server..\n");
    }
    // function for client communication
    func(sockfd);

    // close the socket
    close(sockfd);
}