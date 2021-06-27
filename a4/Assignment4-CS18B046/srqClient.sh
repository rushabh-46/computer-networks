#!/bin/bash

make srqclientmake

debug=0
portNumber=12345
maxPackets=400
seqNumBits=8
windowSize=4
bufferSize=100
packetErrorRate=0.00001

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -d) debug=1 ;;
        -p) portNumber=$2; shift ;;
        -N) maxPackets=$2; shift ;;
        -n) seqNumBits=$2; shift ;;
        -W) windowSize=$2; shift ;;
        -B) bufferSize=$2; shift ;;
        -e) packetErrorRate=$2; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

./srqclient $portNumber $maxPackets $seqNumBits $windowSize $bufferSize $packetErrorRate $debug

# ./srqclient 12345 400 8 4 100 0.00001 0

echo "End of srq client script !!"