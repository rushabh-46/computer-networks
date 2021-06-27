#!/bin/bash

make gbnservermake

debug=0
ipAddress="127.0.0.1"
portNumber=12345
packetLength=512
packetGenRate=10
maxPackets=400
windowSize=3
maxBufferSize=10

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -d) debug=1 ;;
        -s) ipAddress="$2"; shift ;;
        -p) portNumber=$2; shift ;;
        -l) packetLength=$2; shift ;;
        -r) packetGenRate=$2; shift ;;
        -n) maxPackets=$2; shift ;;
        -w) windowSize=$2; shift ;;
        -b) maxBufferSize=$2; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

./gbnserver $ipAddress $portNumber $packetLength $packetGenRate $maxPackets $windowSize $maxBufferSize $debug

echo "End of gbn server script !!"