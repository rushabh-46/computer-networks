#!/bin/bash

make srqservermake

debug=0
ipAddress="127.0.0.1"
portNumber=12345
seqNumBits=8
maxPacketLength=512
packetGenRate=10
maxPackets=400
windowSize=4
bufferSize=100

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -d) debug=1 ;;
        -s) ipAddress="$2"; shift ;;
        -p) portNumber=$2; shift ;;
        -n) seqNumBits=$2; shift ;;
        -L) maxPacketLength=$2; shift ;;
        -R) packetGenRate=$2; shift ;;
        -N) maxPackets=$2; shift ;;
        -W) windowSize=$2; shift ;;
        -B) bufferSize=$2; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

./srqserver $ipAddress $portNumber $seqNumBits $maxPacketLength $packetGenRate $maxPackets $windowSize $bufferSize $debug

echo "End of srq server script !!"