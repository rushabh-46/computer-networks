#ifndef SERVER_H
#define SERVER_H

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

using namespace std;

#define PORT                10000
#define MAXNODES            100
#define MAXMESSAGESIZE      256
#define INFINITY            1e9

#define ff                  first
#define ss                  second

class Server {
private:
    int nodeId;
    int portNumber;
    int rightNeigbors;
    int totalNeighbors;
    std::unordered_map <int, pair<int, int>> neighborsRanges;
    std::unordered_set <int> rightHellos;
    std::mutex rightSetLock;
    int sequenceNumbers[MAXNODES];

public:
    struct sockaddr_in serverAddress;
    socklen_t  clientSocketLength;
    bool isWeightDone[MAXNODES];
    std::unordered_map <int, int> edgesWeights;

    Server(int id);
    int getId();
    int getPortNumber();
    int getTotalNeighbors();
    int getRightNeighbors();
    struct sockaddr_in getSocketAddress();
    socklen_t getClientSockLen();
    void putNeighborRange(int nId, int lowLimit, int highLimit);
    void printNeighborRanges();
    bool isRightSetEmpty();
    void fillRightSet();
    int getRightSetSize();
    void removeRightNode(int id);
    unordered_set <int> getRightSetCopy();
    int getRandomWeight(int neighborId);
    void putWeight(int id, int weight);
    void printEdges();
    string makeLSAMessage(int seqNumber);
    void sendLSA(string message, int id);
    int getSeqNum(int srcId);
    void putSeqNum(int srcId, int seqNum);
};

extern int totalNodes;
extern int rootNodeId;
extern int rootSocketDescriptor;
extern Server* servers[MAXNODES];
extern int sequenceNumber;
extern string outputFileName;
extern int timeNow;

void timer_start(std::function<void(void)> func, unsigned int interval);
void do_hello();
void *receive(void* args);
void do_lsa();
void do_dijstra();

#endif