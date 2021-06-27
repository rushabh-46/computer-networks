#include "packet.h"

std::string IP_ADDRESS;
int PORTNUMBER;
int MAXPACKETS;
float PACKET_ERROR_RATE;
bool DEBUG_MODE;

high_resolution_clock::time_point timeOfStart;
int ERROR_RATE;

int sF;

bool checkErrorRate()
{
    if (rand() % ERROR_RATE == 0)
        return true;
    return false;
}

int main(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++)
    {
        std::cout << argv[i] << " ";
    }
    std::cout << std::endl;
    if (argc != 5)
    {
        std::cerr << "Insufficient arguments in srq client invocation!!\n";
        exit(0);
    }

    IP_ADDRESS = "127.0.0.1";
    PORTNUMBER = stoi(argv[1]);
    MAXPACKETS = stoi(argv[2]);
    PACKET_ERROR_RATE = stof(argv[3]);
    DEBUG_MODE = ((stoi(argv[4]) == 1) ? true : false);

    int sockfd;
    struct sockaddr_in servaddr;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed at client");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORTNUMBER);
    servaddr.sin_addr.s_addr = inet_addr(IP_ADDRESS.c_str());

    int n, len, addr_size;
    std::string hello = "Hello from client";

    sendto(sockfd, (const char *)(hello.c_str()), hello.size(), 0,
           (const struct sockaddr *)&servaddr, sizeof(servaddr));

    char message[1024];
    srand(time(0));
    ERROR_RATE = (1.0 / PACKET_ERROR_RATE);
    addr_size = sizeof(servaddr);
    sF = 0;
    timeOfStart = high_resolution_clock::now();

    while (1)
    {
        n = recvfrom(sockfd, (char *)message, 1024, 0,
                     (struct sockaddr *)&servaddr, (socklen_t *)&addr_size);

        message[n] = '\0';

        if (checkErrorRate())
            continue;

        std::stringstream stream(message);
        std::string type;

        stream >> type;

        if (type == "Stop")
            break;

        // std::cout << "Received message from seq num = " << type << std::endl;

        if (sF != stoi(type))
        {
            // packet not the one expected in sequence -> Hence ignore !!
            continue;
        }

        if (DEBUG_MODE)
        {
            high_resolution_clock::time_point timeReceived = high_resolution_clock::now();
            duration<double> time_span = duration_cast<duration<double>>(timeReceived - timeOfStart);
            double mSec = time_span.count() * 1000;
            int milli = ((int)mSec);
            int micro = 1000 * (mSec - milli);
            std::cout << "Seq #" << sF << ": Time Received: " << milli << ":" << micro << std::endl;
        }

        // send acknowledgement corresponding to expected new seq num packet
        sF++;

        std::cout << "Sending acknowledgement for next seq num = " << sF << std::endl;

        string message = to_string(sF);
        message = message + " !\0";

        sendto(sockfd, (const char *)(message.c_str()), message.size(), 0,
               (const struct sockaddr *)&servaddr, sizeof(servaddr));
    }

    // std::cout << "Done with client side!!\n";

    close(sockfd);
    return 0;
}