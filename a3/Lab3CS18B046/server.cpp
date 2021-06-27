#include "server.h"

/**
 * Default constructor for Server class object
 * @param server id
 */
Server::Server(int id) {
    this->nodeId = id;
    this->portNumber = id + PORT;
    this->totalNeighbors = 0;
    this->rightNeigbors = 0;
    
    this->serverAddress.sin_family = AF_INET;
    this->serverAddress.sin_port = htons(this->portNumber);
    this->serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    this->clientSocketLength = (socklen_t) sizeof(this->serverAddress);

    this->rightSetLock.unlock();

    for (int i = 0; i < totalNodes; i++) {
        this->sequenceNumbers[i] = 0;
    }
}

/**
 * Getter for the server Id
 * @return id
 */
int Server::getId() {
    return this->nodeId;
}

/**
 * Getter for the server port number
 * @return portNumber
 */
int Server::getPortNumber() {
    return this->portNumber;   
}

/**
 * Getter for total neighbors
 * @return totalNeighbors
 */
int Server::getTotalNeighbors() {
    return this->totalNeighbors;
}

/**
 * Getter for total right neighbors
 * Neighbors which have greater id's
 * @return rightNeighbors
 */
int Server::getRightNeighbors() {
    return this->rightNeigbors;
}

/**
 * Getter for socket address of type sockaddr_in
 * @return serverAddress
 */
struct sockaddr_in Server::getSocketAddress() {
    return this->serverAddress;
}

/**
 * Getter for the length of socket address
 * Used when socket behaves as client and needs the length parameter
 * @return clientSocketLength
 */
socklen_t Server::getClientSockLen() {
    return this->clientSocketLength;
}

/**
 * Puts the server id as neigbor to this server
 * Also adds the limit of edge/link between them which gets stored in a map
 * @param nId - neighbor server id
 * @param lowLimit - lower limit of edge length
 * @param highLimit - upper limit of edge length
 */
void Server::putNeighborRange(int nId, int lowLimit, int highLimit)
{
    if (neighborsRanges.find(nId) != neighborsRanges.end()) {
        return;
    }
    pair <int, pair<int, int>> p = make_pair(nId, make_pair(lowLimit, highLimit));
    this->neighborsRanges.insert(p);
    this->edgesWeights.insert(make_pair(nId, lowLimit));
    this->totalNeighbors = 1 + this->totalNeighbors;
    if (nId > this->nodeId) {
        this->rightNeigbors = 1 + this->rightNeigbors;
    }
    // cout << "Successfully put neighbor = " << nId << endl;      
}

/**
 * Helper function to print the neighbors and their corresponding range
 */
void Server::printNeighborRanges() {
    cout << "Printing neighbor ranges for server id = " << this->nodeId << endl;
    for (auto neighbor : this->neighborsRanges) {
        cout << "\t" << neighbor.first << " with limit: "
            << neighbor.second.first << "-" << neighbor.second.second << endl;
    }
    cout << "  Total neighbors = " << this->totalNeighbors << endl;
    cout << "  Total right neighbors = " << this->rightNeigbors << endl;
    cout << endl;
}

/**
 * Tells whether the right is empty or not
 * @return boolean - true if rightHellos set is empty.
 */
bool Server::isRightSetEmpty() {
    this->rightSetLock.lock();
    bool isEmpty = this->rightHellos.size() == 0;
    this->rightSetLock.unlock();
    return isEmpty;
}

/**
 * Fills the right Hello set with all the right neighbors.
 * Also falses the boolean array of weights.
 */
void Server::fillRightSet() {
    this->rightSetLock.lock();
    for (auto neighbor : this->neighborsRanges) {
        if (neighbor.first > this->nodeId) {
            this->rightHellos.insert(neighbor.first);
        }
    }
    this->rightSetLock.unlock();

    // Also initialize false edges for all the neighbors
    // No need for mutex on this as only sendHello thread will update the rights
    // and receive will update the left neighbors.
    memset(this->isWeightDone, false, sizeof(this->isWeightDone));
}

/**
 * Get the size of right set hellos.
 * @return size of rightHellos set
 */
int Server::getRightSetSize() {
    int size;
    this->rightSetLock.lock();
    size = this->rightHellos.size();
    this->rightSetLock.unlock();
    return size;
}

/**
 * Remove the node from hello set as we have received the message
 * @param id - neighbor id
 */
void Server::removeRightNode(int id) {
    this->rightSetLock.lock();
    this->rightHellos.erase(id);
    this->rightSetLock.unlock();
}

/**
 * This function creates a copy of the right neighbor set.
 * This is done so that at the same time we can maniuplate the original right set
 * and we can also iterate through this copy set.
 * 
 * @return copy set of rightHellos
 */
unordered_set <int> Server::getRightSetCopy() {
    unordered_set<int> copySet;
    this->rightSetLock.lock();
    for (int it : this->rightHellos) {
        copySet.insert(it);
    }
    this->rightSetLock.unlock();
    return copySet;
}

/**
 * This function computes random edge length and returns it
 * It does not set the edge length; it just computes the weight.
 * Later to avoid multiple writes, we use the mutex in right set 
 * 
 * @param neighborId - the neighbor id for current server
 * @return edge weight between the two servers
 */
int Server::getRandomWeight(int neighborId) {
    auto neighbor = this->neighborsRanges.find(neighborId);
    if (neighbor == this->neighborsRanges.end()) {
        // cout << "Neighbor Range Map incorrect. Couldn't find id = " << neighborId
        //     << " in the server id = " << this->nodeId << endl;
        return 5;
    }
    int left = neighbor->second.first;
    int right = neighbor->second.second;
    int edgeW = left + rand() % (right - left);
    return edgeW;
}

/**
 * Put the edge between the servers
 * 
 * @param id - neighbor server id
 * @param weight - link weight
 */
void Server::putWeight(int id, int weight) {
    // cout << "Putting weight: " << this->nodeId << "-" << id << " = " << weight << endl;
    if (this->edgesWeights.find(id) == this->edgesWeights.end()) {
        // cerr << "Cannot update weight as wrong neighbor is asked neighbor = " << id 
        //     << " and root server id = " << this->nodeId << endl;
        return;
    }
    this->edgesWeights[id] = weight;
    // if (rootNodeId >= 0) {
    //     cout << "\tFROM " << rootNodeId << ": UPDATING edge " << this->nodeId << "-" << id << endl;
    // }
}

/**
 * Print the edge weights
 * Helper debugger function
 */
void Server::printEdges() {
    cout << "Printing weights for id = " << this->nodeId << endl;
    for (auto it : this->edgesWeights) {
        cout << "\t" << this->nodeId << "-" << it.first << " = " << it.second << endl;
    }
}

/**
 * This function computes LSA message.
 * @param seqNumber - the current sequence number
 * @return LSA message string
 */
string Server::makeLSAMessage(int seqNumber) {
    string message = "LSA " + to_string(this->nodeId) + " " + to_string(seqNumber)
        + " " + to_string(this->getTotalNeighbors());
    for (auto it : this->edgesWeights) {
        message = message + " " + to_string(it.first) + " " + to_string(it.second);
    }
    return message;
}

/**
 * Send LSA message to all the neighbors except ignoreId
 * @param message - the message to be send
 * @param ignoreId - the id to which the message to not send
 */
void Server::sendLSA(string message, int ignoreId) {
    for (auto it : this->edgesWeights) {
        if (it.second != ignoreId) {
            while(sendto(rootSocketDescriptor, message.c_str(),
                        message.size(), 0,
                        (struct sockaddr*) &(servers[it.first]->serverAddress), 
                        servers[it.first]->getClientSockLen()) < 0) 
            {
                // repeat the send message
                cout << "Will send LSA again in a few milliseconds\n";
                usleep(5000);
            }
        }
    }
}

/**
 * To get the sequence number corresponding to the src id give.
 * Just check the boolean array.
 * 
 * @param srcId - src id whose seq number is needed.
 * @return sequence number for srcId
 */
int Server::getSeqNum(int srcId) {
    return this->sequenceNumbers[srcId];
}

/**
 * Update the sequence number as per the parameters
 * 
 * @param srcId - src id whose seq number is to be updated
 * @param seqNum - new sequence number 
 */
void Server::putSeqNum(int srcId, int seqNum) {
    this->sequenceNumbers[srcId] = seqNum;
}