#include "server.h"

int timeNow;

/**
 * Creates spaces for given number of times required. Used for output.
 * 
 * @param num - number of spaces needed.
 * @return string with given number of spaces.
 */
string spaces(int num) {
    string space = "";
    for (int i = 0; i < num; i++) {
        space = space + " ";
    }
    return space;
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
    // std::cout << "Timer start for id = " << rootNodeId << endl;
    thread([func, interval]() {
        int cc = 0;
        while (true)
        {
            this_thread::sleep_for(chrono::milliseconds(interval));
            timeNow = cc * interval / 1000;
            func();
            cc++;
        }
    }).detach();
}


/**
 * Used for sending HELLO to all the neighbours in set.
 * 1. Resets the right neighbor set of server.
 * 2. Keeps sending HELLO message to all the nodes in the set.
 */
void do_hello()
{
    // cout << "HELL/LOO from " << rootNodeId << endl;
    Server* rootServer = servers[rootNodeId];
    rootServer->fillRightSet();
    
    int size = rootServer->getRightSetSize();
    // cout << "Right HELLO set size for " << rootNodeId << " is " << size << endl;

    string helloMessage = "HELLO " + to_string(rootNodeId);

    while(!rootServer->isRightSetEmpty())
    {
        size = rootServer->getRightSetSize();
        // cout << "Right HELLO set size for " << rootNodeId << " is " << size << endl;
        unordered_set<int> currentRightSet = rootServer->getRightSetCopy();
        for (int neighborNode : currentRightSet) {
            // cout << "Sending from " << rootNodeId << " to " << neighborNode << endl;
            while(sendto(rootSocketDescriptor, helloMessage.c_str(), helloMessage.size(), 0,
                    (struct sockaddr*) &(servers[neighborNode]->serverAddress), 
                    servers[neighborNode]->getClientSockLen()) < 0) 
            {
                // repeat the send message
                std::cout << "Will send the message again in a few milliseconds\n";
                usleep(5000);
            }
        }
        usleep(size * 1000);
    }

    // rootServer->printEdges();

}

/**
 * Perform Link State Advertisement in a flood fill fashion
 * 1. Compute the LSA message.
 * 2. Send to all its neighbors.
 * 3. receive() will take care to propagate the LSA message further depending on the sequence numbers.
 */
void do_lsa() {
    // cout << "Hi from LSA id = " << rootNodeId << endl;
    sequenceNumber++;
    Server* rootServer = servers[rootNodeId];
    string lsaMessage = rootServer->makeLSAMessage(sequenceNumber);
    rootServer->sendLSA(lsaMessage, -1);
}

/**
 * Perform Dijstra's algorithm to find the shortest distance from source to every other node.
 */
void do_dijstra() {
    // std::cout << "Performing Dijstra's on id = " << rootNodeId << endl;
    priority_queue <pair<int, int>, vector <pair<int, int>>, greater<pair<int, int>>> pq;
    int parents[totalNodes];
    int weights[totalNodes];
    string paths[totalNodes];
    for (int i = 0; i < totalNodes; i++) {
        parents[i] = -1;
        weights[i] = INFINITY;
    }
    parents[rootNodeId] = rootNodeId;
    weights[rootNodeId] = 0;
    paths[rootNodeId] = to_string(rootNodeId);
    pq.push(make_pair(0, rootNodeId));

    while(!pq.empty()) {
        int temprootWeight = pq.top().first;
        int temprootId = pq.top().second;
        pq.pop();
        for (auto it : servers[temprootId]->edgesWeights) {
            int nodeId = it.first;
            int weight = it.second;
            if (weights[nodeId] > temprootWeight + weight) {
                parents[nodeId] = temprootId;
                weights[nodeId] = temprootWeight + weight;
                paths[nodeId] = paths[temprootId] + to_string(nodeId);
                pq.push(make_pair(weights[nodeId], nodeId));
            }
        }
    }

    fstream outputFile;
    outputFile.open (outputFileName, fstream::in | fstream::out | fstream::app);

    outputFile << "Routing Table for Node No. " << rootNodeId << " at Time " << timeNow << endl;
    outputFile << "Destination     Path     Cost\n";

    for (int i = 0; i < totalNodes; i++) {
        outputFile << spaces(5) << i << spaces(5);
        int sp = 11 - paths[i].size();
        outputFile << spaces(5) << paths[i] << spaces(sp) << weights[i] << endl;
    }

    outputFile << endl;
    outputFile.close();

    // std::cout << "Dijstra's algorithm runned and outfile file updated succesfully for id = " << rootNodeId << endl;;
}

/**
 * Receive function to receive the UDP packets
 * and according to the buffer message do the respective computation.
 * 
 * @param args - thread function argument structure
 */
void *receive(void* args) {
    // receive
    // (left)  HELLO - 1. decide the length, 2. send HELLOREPLY
    // (right) HELLOREPLY - 1. get the length 2. update the set
    // ..
    cout << "Receive from id = " << rootNodeId << endl;
    int* rootSocketDescriptor = (int *)args;
    Server* rootServer = servers[rootNodeId];
    char clientMessage[MAXMESSAGESIZE];
    struct sockaddr_in clientAddress;
    socklen_t client_struct_length = (socklen_t) sizeof(clientAddress);

    memset(clientMessage, '\0', sizeof(clientMessage));

    while(1) 
    {
        // cout << "Trying to receive from id = " << rootNodeId << endl;

        while(recvfrom(*rootSocketDescriptor, clientMessage, sizeof(clientMessage), 0,
        (struct sockaddr*)&clientAddress, &client_struct_length) < 0) {/**/}

        stringstream stream(clientMessage);
        string type;
        int srcId;

        stream >> type;
        if (type == "HELLO")
        {
            // cout << "Received HELLO\n";
            stream >> srcId;
            if (srcId < 0 || srcId >= totalNodes)
            {
                // cerr << "Incorrect Id = " << srcId << endl;
                continue;
            }

            // check if the weight is already set!!!
            if (rootServer->isWeightDone[srcId]) {
                // The weight is already set. Ignore this HELLO message.
                continue;
            }
            int weight = rootServer->getRandomWeight(srcId);
            rootServer->putWeight(srcId, weight);
            rootServer->isWeightDone[srcId] = true;
            string helloReplyMessage = "HELLOREPLY " + to_string(rootNodeId) + " "
                + to_string(srcId) + " " + to_string(weight);
            while(sendto(*rootSocketDescriptor, helloReplyMessage.c_str(),
                    helloReplyMessage.size(), 0,
                    (struct sockaddr*) &(servers[srcId]->serverAddress), 
                    servers[srcId]->getClientSockLen()) < 0) 
            {
                // repeat the send message
                // cout << "Will send the message again in a few milliseconds\n";
                usleep(5000);
            }
        }
        else if (type == "HELLOREPLY")
        {
            // cout << "Received HELLOREPLY\n";
            int rootID, weight;
            stream >> srcId;
            stream >> rootID;
            stream >> weight;
            if (srcId < 0 || srcId >= totalNodes || rootID != rootNodeId)
            {
                // cerr << "Incorrect HELLOREPLY message id = " << srcId << endl;
                continue;
            }
            // check if the weight is already set!!!
            if (rootServer->isWeightDone[srcId]) {
                // The weight is already set. Ignore this HELLO message.
                continue;
            }
            rootServer->putWeight(srcId, weight);
            rootServer->isWeightDone[srcId] = true;
            rootServer->removeRightNode(srcId);
        } 
        else if(type == "LSA") {
            // cout << "Received " << clientMessage << endl;
            int seqNum, numEntries, neigh, cost;
            stream >> srcId;
            stream >> seqNum;
            stream >> numEntries;
            if (srcId < 0 || srcId >= totalNodes)
            {
                // cerr << "Incorrect id in LSA message from id = " << srcId << endl;
                continue;
            }
            if (seqNum < sequenceNumber) {
                // Ignore this sequence number if less only. If greater then Okay
                continue;
            }
            if (seqNum <= rootServer->getSeqNum(srcId)) {
                // We have already received this sequence number. Hence reject it
                continue;
            }
            rootServer->putSeqNum(srcId, seqNum);
            while(numEntries--) {
                stream >> neigh >> cost;
                if (neigh < 0 || neigh >= totalNodes)
                {
                    // cerr << "Incorrect neighbor id from LSA message id = " << neigh << endl;
                    continue;
                }
                servers[srcId]->putWeight(neigh, cost);
            }
            rootServer->sendLSA(clientMessage, srcId);
        }
        else {
            // cout << "Received something different -> " << clientMessage << endl;
        }

        memset(clientMessage, '\0', sizeof(clientMessage));
    }
}
