#include "packet.h"

std::string IP_ADDRESS;

int PORTNUMBER;
int MAXPACKETS;
int SEQTOTALBITS;
int WINDOWSIZE;
int BUFFERSIZE;
float PACKET_ERROR_RATE;
bool DEBUG_MODE;

high_resolution_clock::time_point timeOfStart;
int ERROR_RATE;

struct BoolPacket
{
    bool ack;
    high_resolution_clock::time_point timeReceived;
    BoolPacket()
    {
        this->ack = false;
    }
};

vector<BoolPacket *> window;
int sF;
int totalSeqNum;

bool checkErrorRate()
{
    if (rand() % ERROR_RATE == 0)
        return true;
    return false;
}

/**
 * Pop seauentially and hence print in the order of sequence number.
 */
void push_pop()
{
    int count = 0;
    while (window.size() > 0 && window.front()->ack == true)
    {
        BoolPacket *rem = window.front();
        if (DEBUG_MODE)
        {
            duration<double> time_span = duration_cast<duration<double>>(rem->timeReceived - timeOfStart);
            double mSec = time_span.count() * 1000;
            int milli = ((int)mSec);
            int micro = 1000 * (mSec - milli);
            std::cout << "Seq #" << ((sF + count) % totalSeqNum)
                      << ": Time Received: " << milli << ":" << micro << std::endl;
        }
        window.erase(window.begin());
        BoolPacket *temp = new BoolPacket();
        window.push_back(temp);
        count++;
    }
    sF += count;
    sF %= totalSeqNum;
    return;
}

int main(int argc, char *argv[])
{
    // for (int i = 0; i < argc; i++) {
    //     std::cout << argv[i] << " ";
    // }
    std::cout << std::endl;
    if (argc != 8)
    {
        std::cerr << "Insufficient arguments in srq client invocation!!\n";
        exit(0);
    }

    PORTNUMBER = stoi(argv[1]);
    MAXPACKETS = stoi(argv[2]);
    SEQTOTALBITS = stoi(argv[3]);
    WINDOWSIZE = stoi(argv[4]);
    BUFFERSIZE = stoi(argv[5]);
    PACKET_ERROR_RATE = stof(argv[6]);
    DEBUG_MODE = ((stoi(argv[7]) == 1) ? true : false);

    IP_ADDRESS = "127.0.0.1";

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

    /* Confirm connection by sending hello message*/
    std::string hello = "Hello from client";
    sendto(sockfd, (const char *)(hello.c_str()), hello.size(), 0,
           (const struct sockaddr *)&servaddr, sizeof(servaddr));

    /* Initialize and declare */
    char message[1024];
    srand(time(0));
    ERROR_RATE = (1.0 / PACKET_ERROR_RATE);
    addr_size = sizeof(servaddr);
    sF = 0;
    totalSeqNum = (1 << SEQTOTALBITS);
    timeOfStart = high_resolution_clock::now();

    for (int i = 0; i < WINDOWSIZE; i++)
    {
        BoolPacket *temp = new BoolPacket();
        window.push_back(temp);
    }

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

        int index = stoi(type) - sF;
        if (index < 0)
            index += totalSeqNum;
        if (index >= WINDOWSIZE)
        {
            // std::cout << "Out of bound seq num " << type << " and sF = " << sF << std::endl;
            continue;
        }

        if (window[index]->ack == true)
        {
            // Should continue because of the assumption that ACK sent is reached !!
            // continue;
        }
        else
        {
            window[index]->ack = true;
            window[index]->timeReceived = high_resolution_clock::now();
        }

        if (index == 0)
            push_pop();

        std::cout << "Sending acknowledgement to seq num = " << type << std::endl;
        sendto(sockfd, (const char *)(type.c_str()), type.size(), 0,
               (const struct sockaddr *)&servaddr, sizeof(servaddr));
    }

    // std::cout << "Done with client side!!\n";

    close(sockfd);
    return 0;
}