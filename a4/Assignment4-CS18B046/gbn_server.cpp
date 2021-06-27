#include "packet.h"

int globalTimeout;
int PORT_NUMBER;
int PACKET_LENGTH;
int PACKET_GEN_RATE;
int MAX_PACKETS;
int WINDOW_SIZE;
int MAX_BUFFER_SIZE;
bool DEBUG_MODE;

int currentLastSeqNumber;
int udpSocket;
int packet_avg_length;
int total_packets;
int total_packet_length;

std::string IP_ADDRESS;

BufferVector *buffer;

/**
 * Generate at max PACKET_GEN_RATE packets.
 * Gets called each second.
 */
void generate_packets()
{
    for (int i = 0; i < PACKET_GEN_RATE; i++)
    {

        if (buffer->getSize() < MAX_BUFFER_SIZE)
        {
            Packet *newPacket = new Packet(currentLastSeqNumber, PACKET_LENGTH);
            currentLastSeqNumber++;
            // currentLastSeqNumber = currentLastSeqNumber % (1 << SEQ_TOTAL_BITS);
            buffer->push(newPacket);

            total_packet_length += PACKET_LENGTH;
            total_packets++;
        }
        else
        {
            // should break or not ??
            // std::cout << "Added " << i << " packets\n"
            //           << std::flush;
            break;
        }
    }
    // std::cout << "Added " << PACKET_GEN_RATE << " packets\n" << std::flush;
}

/**
 * This function is used to constinuously invoke a function after
 * some interval of time in a separate thread.
 * It creates a thread which does the function call and sleeps for the interval.
 * 
 * @param func - the function supposed to be repeatedly invoked.
 * @param interval - time interval (in milli-seconds)
 */
void timer_start(function<void(void)> func, unsigned int interval)
{
    thread([func, interval]()
           {
               int cc = 0;
               while (true)
               {
                   func();
                   this_thread::sleep_for(chrono::milliseconds(interval));
                   cc++;
                   if (buffer->kill)
                       break;
               }
           })
        .detach();
}

void *receive(void *)
{
    int nBytes;
    char message[1024];
    struct sockaddr_in serverStorage;
    /*Initialize size variable to be used later on*/
    int addr_size = sizeof(serverStorage);

    while (1)
    {
        nBytes = recvfrom(udpSocket, message, 1024, 0,
                          (struct sockaddr *)&serverStorage, (socklen_t *)&addr_size);

        message[nBytes] = '\0';

        std::stringstream stream(message);
        std::string type;

        stream >> type;

        // std::cout << "Acknowledgement received for " << type << std::endl;

        buffer->acknowledgeAll(stoi(type) - 1);

        if (buffer->totalACKPackets >= MAX_PACKETS)
            break;
        if (buffer->kill)
            break;
    }
    if (!buffer->kill)
        std::cout << "Receive done !! All acknowledgements done !!\n";
}

int main(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++)
    {
        std::cout << argv[i] << " ";
    }
    std::cout << std::endl;
    if (argc != 9)
    {
        std::cerr << "Insufficient arguments in srq server invocationn!!\n";
        exit(0);
    }

    IP_ADDRESS = argv[1];
    PORT_NUMBER = stoi(argv[2]);
    PACKET_LENGTH = stoi(argv[3]);
    PACKET_GEN_RATE = stoi(argv[4]);
    MAX_PACKETS = stoi(argv[5]);
    WINDOW_SIZE = stoi(argv[6]);
    MAX_BUFFER_SIZE = stoi(argv[7]);
    DEBUG_MODE = ((stoi(argv[8]) == 1) ? true : false);

    struct sockaddr_in serverAddr;

    /*Create UDP socket*/
    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

    // std::cout << "Using IP address = " << IP_ADDRESS << " and port = " << PORT_NUMBER << " to bind.\n";

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_NUMBER);
    serverAddr.sin_addr.s_addr = inet_addr(IP_ADDRESS.c_str());

    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    /*Bind socket with address struct*/
    if (bind(udpSocket, (struct sockaddr *)&serverAddr,
             sizeof(serverAddr)) < 0)
    {
        std::cerr << "Bind failed!!\n";
        return -1;
    }

    // std::cout << "Seq number bits = " << SEQ_TOTAL_BITS << " => " << (1 << SEQ_TOTAL_BITS) << std::endl;

    struct sockaddr_in serverStorage;
    socklen_t addr_size = sizeof(serverStorage);
    int nBytes, i;
    char message[1024];

    /* Get client connected first with some random message !! */
    nBytes = recvfrom(udpSocket, (void *)message, 1024, 0,
                      (struct sockaddr *)&serverStorage, (socklen_t *)&addr_size);

    message[nBytes] = '\0';

    // std::cout << "CONNECTED with server with message : " << message << std::endl;

    /* Initialize global buffer structure 20 seqBits -> very big hence doesn't matter */
    buffer = new BufferVector(20, WINDOW_SIZE, DEBUG_MODE, 100);

    /* generating packets independently !! */
    srand(time(0));
    currentLastSeqNumber = 0;
    total_packet_length = 0;
    total_packets = 0;
    timer_start(generate_packets, 1000);

    /* rreceive thread working independently */
    pthread_t receiveThreadID;
    pthread_create(&receiveThreadID, NULL, receive, NULL);

    /**
     * 1. Shift window so that sF does not change in the later program
     *      and hence no worries about out-of-bound vector
     * 2. Iterate through the buffer vector - only until min(size, WINDOW_SIZE)
     *      And identify whether the packet is acknowledged 
     *      -> if not -> send it !!
     *      -> if yes -> ignore !!
     */
    while (1)
    {
        buffer->shiftWindow(); // not needed

        // No change expected on buffer->sF
        //      Hence we can index without inconsistency !
        // No decrement expected on buffer->bufferSize
        //      Hence accessing bufferSize without a lock !!!!!!!!!!!

        // std::cout << "Buffer size = " << buffer->getSize() << std::endl;

        for (int i = 0; i < min(buffer->getSize(), WINDOW_SIZE); i++)
        {
            // std::cout << "in the loop iter = " << i << std::flush;
            // std::cout << "Trying to send for seq num = " << buffer->buf[i]->getSeqNum() << std::endl;
            int ss = buffer->buf[i]->shouldSend(buffer->timeout);
            if (ss == 1)
            {
                // std::string message = buffer->buf[i]->message;
                sendto(udpSocket, (void *)(buffer->buf[i]->message), buffer->buf[i]->psize, 0,
                       (struct sockaddr *)&serverStorage, addr_size);
            }
            else if (ss == -1)
            {
                buffer->kill = true;
                break;
            }
        }

        // std::cout << "Main while loop iteration done. Sleeping for a sec !!\n";

        if (buffer->totalACKPackets >= MAX_PACKETS)
            break;

        if (buffer->kill)
            break;
    }

    std::cout << "Finishing SRQ \n";

    // pthread_join(receiveThreadID, NULL);

    std::string stopMessage = "Stop !";
    sendto(udpSocket, (void *)(stopMessage.c_str()), stopMessage.size(), 0,
           (struct sockaddr *)&serverStorage, addr_size);

    close(udpSocket);

    std::cout << "PACKET_GEN_RATE: " << PACKET_GEN_RATE << std::endl;
    std::cout << "PACKET_AVG_LENGTH: " << (double)((double)total_packet_length / (double)total_packets) << std::endl;
    std::cout << "ReTransmission Ratio: " << (double)((double)buffer->totalRetransmissions / (double)buffer->totalACKPackets) << std::endl;
    std::cout << "Average RTT: " << buffer->RTT << std::endl;

    std::cout << "Total packets acknowledged = " << buffer->totalACKPackets << std::endl;

    return 0;
}
