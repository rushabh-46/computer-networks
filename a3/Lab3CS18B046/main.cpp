#include "server.h"

int totalNodes;
int rootNodeId;
int rootSocketDescriptor;
Server* servers[MAXNODES];
int sequenceNumber;
string outputFileName;

int main(int argc, char* argv[]) {
    for (int i = 0; i < argc; i++) {
        cout << argv[i] << " ; ";
    }
    cout << endl;
    ifstream inputFile;
    inputFile.open(argv[2]);

    string read;

    if (!inputFile) {
        cerr << "Input file empty or unable to open\n";
        exit(1);
    }

    /**
     * Total number of nodes/routers in the system.
     */
    inputFile >> read;
    totalNodes = stoi(read);

    /**
     * Total number of edges/linkes among the routers.
     */
    inputFile >> read;
    int totalEdges = stoi(read);

    rootNodeId = stoi(argv[1]);
    int rootPortNumber = PORT + rootNodeId;

    /**
     * servers - array of Server objects which store
     * id and port number and address info together
     * Can be used to pick up the client side socket easily 
     * by just indexing (index == id)
     */
    // Server* servers[totalNodes];
    for (int id = 0; id < totalNodes; id++) {
        servers[id] = new Server(id);
    }

    /**
     * Setting the coket descriptor for this root socket.
     */
    rootSocketDescriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (rootSocketDescriptor < 0) {
        printf("ERROR: Socket creation failed (id = %d)\n", rootNodeId);
        close(rootSocketDescriptor);
    }

    printf("Socket created successfully with rootNodeId = %d\n", rootNodeId);
    
    struct sockaddr_in rootSocketAddress = servers[rootNodeId]->getSocketAddress();

    // Bind to the set port and IP:
    if(bind(rootSocketDescriptor, (struct sockaddr*)&rootSocketAddress, sizeof(rootSocketAddress)) < 0)
    {
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");

    while(totalEdges--) {
        int x, y, low=2, high=5;
        inputFile >> x >> y >> low >> high;
        if (x >= totalNodes || y >= totalNodes || x < 0 || y < 0) {
            cerr << x << " & " << y << "; ";
            cerr << "Incorrect node Id in input file. It should be in the range [0.."
                << totalNodes << endl;
            close(rootSocketDescriptor);
            exit(0);
        }
        servers[x]->putNeighborRange(y, low, high);
        servers[y]->putNeighborRange(x, low, high);
    }
    inputFile.close();

    outputFileName = argv[3];

    sleep(2);

    // std::cout << "Lets go for id = " << rootNodeId << endl;
    sequenceNumber = 0;

    pthread_t rid;
    pthread_create(&rid, NULL, &receive, (void *)&rootSocketDescriptor);
    timer_start(do_hello, 1000 * stoi(argv[4]));
    timer_start(do_lsa, 1000 * stoi(argv[5]));
    timer_start(do_dijstra, 1000 * stoi(argv[6]));
    
    // while(1) {
        sleep(65);
    // }

    close(rootSocketDescriptor);
    std::cout << "FINISHING main\n";
    return 0;
}