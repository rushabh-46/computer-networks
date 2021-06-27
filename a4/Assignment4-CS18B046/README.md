The program consists of implementation of SRQ ARP and GBN transmission protocol.

Each protocol is implemented using server and client cpp code.
These codes are invoked by their own bash scripts.

I have a commmon header file <packet.h> which consists the implementation of Packet and Buffer structure.

To execute the code for SRQ protocol, open two terminals.

1. Use terminal 1 for invoking server side bash script. Just run:
   $bash srqServer.sh [inputs]
   where [inputs] are following:
   [-d] -> Turn debug mode on
   [-s] <string> -> IP address
   [-p] <int> -> Port number
   [-n] <int> -> Total sequence number bits
   [-L] <int> -> Maximum packet length
   [-R] <int> -> Packet generation rate
   [-N] <int> -> Maximum packets
   [-W] <int> -> Window size
   [-B] <int> -> Buffer size

2. Use terminal 2 for invoking client side bash script. Just run:
   $bash srqClient.sh [inputs]
   where [inputs] are following:
   [-d] -> Turn debug mode on
   [-p] <int> -> Port number
   [-N] <int> -> Maximum packets
   [-n] <int> -> Total sequence number bits
   [-W] <int> -> Window size
   [-B] <int> -> Buffer size
   [-e] <float> -> Packet error rate

To execute the GN protocol, again open two terminals - one for server and one for client.

1. Use terminal 2 for invoking server side bash script. Just run:
   $bash gbnServer.sh [inputs]
   where [inputs] are following:
   [-d] -> Turn debug mode on
   [-s] <string> -> IP address
   [-p] <int> -> Port number
   [-l] <int> -> Packet length
   [-r] <int> -> Packet generation rate
   [-n] <int> -> Maximum packets
   [-w] <int> -> Window size
   [-b] <int> -> Maximum buffer size

2. Use terminal 2 for invoking client side bash script. Just run:
   $bash gbnClient.sh [inputs]
   where [inputs] are following:
   [-d] -> Turn debug mode on
   [-p] <int> -> Port number
   [-n] <int> -> Maximum packets
   [-e] <float> -> Packet error rate

I have kept default values of all these inuts at the begging of the code.
Termination of all the codes is ensured even when connection fails.
