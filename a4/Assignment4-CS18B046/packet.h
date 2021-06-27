#ifndef PACKET_H
#define PACKET_H

#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <chrono>
#include <thread>
#include <functional>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <iterator>
#include <sstream>
#include <mutex>
#include <time.h>
#include <ctime>
using namespace std;
using namespace std::chrono;

#define RTT_TIMEOUT_RATIO 20

class Packet
{
    int seqNum;
    bool ack;
    bool firstSent;
    high_resolution_clock::time_point timeOfCreation;
    high_resolution_clock::time_point latestSent;

public:
    int psize;
    char *message;
    int totalTransmission;

    Packet(int seqNumber, int size)
    {
        // std::cout << "Creating a packet with arg: " << seqNumber << " " << size << std::endl;
        this->seqNum = seqNumber;
        this->psize = size;
        this->createMessage();
        this->ack = false;
        this->timeOfCreation = high_resolution_clock::now();
        this->firstSent = false;
        this->totalTransmission = 0;
    }

    ~Packet()
    {
        delete (message);
    }

    int getSeqNum()
    {
        return this->seqNum;
    }

    int getSize()
    {
        return this->psize;
    }

    void createMessage()
    {
        // std::cout << "Creating message\n" << std::flush ;
        std::string seqString = to_string(this->seqNum);
        this->message = (char *)malloc(this->psize * sizeof(char));
        int itr = 0;
        for (char c : seqString)
        {
            this->message[itr++] = c;
        }
        this->message[itr++] = ' ';

        while (itr < this->psize)
        {
            this->message[itr++] = '*';
        }

        this->message[this->psize - 1] = '\0';

        // std::cout << "Message created \n" << std::flush;

        // int remBytes = this->psize - message.size();
        // for (int i= 0 ; i < remBytes - 1; i++) message = message + "A";
        // message = message + '\0';
        // std::cout << "Message size = " << message.size()
        //     << " and packet size = " << this->psize << std::endl;
        // this->message = message;
    }

    bool isAck()
    {
        return this->ack;
    }

    /**
     * Return whether already acknowledged -> -1
     * Or not -> Then acknowledge
     * @return time difference between now and time of creation to modify RTT
     */
    double pAcknowledge()
    {
        if (this->ack)
            return -1;

        high_resolution_clock::time_point now = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(now - this->timeOfCreation);
        this->ack = true;
        return time_span.count() * 1000;
    }

    /**
     * Identifies whether the packet is not acknowledged 
     * Checks with the latest sent time
     * If the difference is more than $timeout -> updates the latest sent time
     * Increment the transmission count
     * 
     * @return 1 -> send the packet; 
     *          0 -> don't send the packet
     *          -1 -> exit !
     */
    int shouldSend(double timeoutt)
    {
        if (this->ack == true)
            return 0;

        high_resolution_clock::time_point now = high_resolution_clock::now();
        if (this->firstSent)
        {
            // check the latestTime and now -> if timeout is exceeded
            duration<double> time_span = duration_cast<duration<double>>(now - this->latestSent);
            if (timeoutt > (time_span.count() * 1000))
                return 0;
            // std::cout << "Transmission num : " << this->totalTransmission << " with timegap = " << (time_span.count()) << std::endl;
        }
        else
        {
            this->firstSent = true;
        }
        this->totalTransmission++;

        if (this->totalTransmission > 10)
        {
            std::cout << "Total transmission exceeded 10 for seq number " << this->seqNum << std::endl;
            return -1;
        }

        // update latestSent
        this->latestSent = high_resolution_clock::now();
        return 1;
    }

    void printDebug(double rtt)
    {
        using Clock = std::chrono::high_resolution_clock;
        constexpr auto num = Clock::period::num;

        high_resolution_clock::time_point now = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(now - this->timeOfCreation);

        double mSec = time_span.count() * 1000;
        int milli = (int)mSec;
        int micro = 1000 * (mSec - milli);

        std::cout << "Seq #" << this->seqNum << ": Time Generated "
                  // << this->timeOfCreation.time_since_epoch().count() << num
                  << milli << ":" << micro
                  << " RTT: " << rtt << " Number of Attempts: " << this->totalTransmission << std::endl;
    }
};

class BufferVector
{
    std::mutex mtx;
    int sF;
    int totalSeqBits;
    int wSize;
    bool debugMode;

public:
    std::vector<Packet *> buf;
    int bufferSize;
    int totalACKPackets;
    int totalRetransmissions;
    double totalTimeSpan;
    double RTT;
    double timeout;
    bool kill;

    BufferVector(int seqTotalBits, int wSiz, bool debug, int timeO)
    {
        this->bufferSize = 0;
        this->sF = 0;
        this->totalSeqBits = seqTotalBits;
        this->totalACKPackets = 0;
        this->totalTimeSpan = 0;
        this->timeout = timeO;
        this->wSize = wSiz;
        this->kill = false;
        this->totalRetransmissions = 0;
        this->debugMode = debug;
    }

    ~BufferVector()
    {
        for (int i = 0; i < this->bufferSize; i++)
        {
            delete (buf[i]);
        }
    }

    /**
     * Getter for buffer size
     * @return bufferSize
     */
    int getSize()
    {
        int s;
        this->mtx.lock();
        s = this->bufferSize;
        this->mtx.unlock();
        return s;
    }

    /**
     * push back the pacet in the buffer
     * @param packet - to insert in the buffer
     */
    void push(Packet *packet)
    {
        this->mtx.lock();
        this->buf.push_back(packet);
        this->bufferSize = this->bufferSize + 1;
        this->mtx.unlock();
    }

    /**
     * Checks whether the first packet in the buffer is acknowledged or not.
     * Deletes a packet from buffer if acknowledged and returns its size.
     * @return size of popped packet.
     */
    int pop()
    {
        // reading buffer size doesn't need lock here !!
        if (this->bufferSize < 1)
        {
            std::cerr << "Empty buffer asked to pop !!\n";
            return -1;
        }
        Packet *pack;

        this->mtx.lock();
        pack = this->buf.front();
        if (!pack->isAck())
        {
            this->mtx.unlock();
            return -1;
        }
        this->buf.erase(this->buf.begin());
        // this->sF ++;
        // this->sF %= (1 << this->totalSeqBits);
        // this->bufferSize --;
        this->mtx.unlock();

        // no need to invoke here ! Just invoke while sending.
        // this->shiftWindow();

        int siz = pack->getSize();
        delete (pack);
        return siz;
    }

    /**
     * acknowledge if not already
     * get the timespan
     * update the RTT and timeout
     * @param packetSeqNum - sequence number of packet !
     */
    void acknowledge(int packetSeqNum)
    {
        // sF -> 0,1,2,...,2^n - 1
        // packetSeqNum -> 0,1,2,..(1 << totalSeqBits - 1) - same as sF
        int totalSeqNumber = (1 << this->totalSeqBits);

        double timeSpan;

        this->mtx.lock();
        int index = packetSeqNum - this->sF;
        if (index < 0)
            index += totalSeqNumber;
        if (index >= this->wSize)
        {
            this->mtx.unlock();
            return;
        }
        timeSpan = this->buf[index]->pAcknowledge();
        this->mtx.unlock();

        if (timeSpan != -1)
        {
            // std::cout << "Succesfull acknowledgement for packet snum = " << packetSeqNum << std::endl;
            this->totalRetransmissions += this->buf[index]->totalTransmission;
            this->totalACKPackets++;
            this->totalTimeSpan += timeSpan;
            this->RTT = this->totalTimeSpan / (double)this->totalACKPackets;
            if (this->totalACKPackets >= 10)
            {
                this->timeout = RTT_TIMEOUT_RATIO * this->RTT;
            }
            // Seq #: Time Generated: xx:yy RTT: zz Number of Attempts: aa
            if (this->debugMode)
            {
                this->buf[index]->printDebug(this->RTT);
            }
        }
    }

    void acknowledgeAll(int packetSeqNum)
    {
        int totalSeqNumber = (1 << this->totalSeqBits);
        double timeSpan;

        this->mtx.lock();
        int index = packetSeqNum - this->sF;
        if (index < 0)
            index += totalSeqNumber;
        if (index >= this->wSize)
        {
            this->mtx.unlock();
            return;
        }
        for (int i = 0; i <= index; i++)
        {
            timeSpan = this->buf[i]->pAcknowledge();

            // don't worry -> this condition will go only once per packet - 1 ACK
            if (timeSpan != -1)
            {
                // std::cout << "Succesfull acknowledgement for packet snum = " << packetSeqNum << std::endl;
                this->totalRetransmissions += this->buf[i]->totalTransmission;
                this->totalACKPackets++;
                this->totalTimeSpan += timeSpan;
                this->RTT = this->totalTimeSpan / (double)this->totalACKPackets;
                if (this->totalACKPackets >= 10)
                {
                    this->timeout = 10 * this->RTT;
                }
                // Seq #: Time Generated: xx:yy RTT: zz Number of Attempts: aa
                if (this->debugMode)
                {
                    this->buf[i]->printDebug(this->RTT);
                }
            }
        }
        this->mtx.unlock();
    }

    /**
     * Shift the window by popping the first acknowledged packets !!
     * This is very efficient shift method as it only takes the lock when required !!
     */
    void shiftWindow()
    {
        Packet *pack;
        int numPops = 0;
        // reading buffer size doesn't need lock here !!
        while (this->getSize() > 0)
        {
            int didPop = this->pop();
            if (didPop < 0)
            {
                break;
            }
            numPops++;
        }

        this->mtx.lock();
        this->sF += numPops;
        this->bufferSize -= numPops;
        this->sF %= (1 << totalSeqBits);
        this->mtx.unlock();

        if (numPops > 0)
        {
            // std::cout << "Window shifted by " << numPops << " packets\n" << std::flush;
        }
    }
};

#endif // PACKET_H
