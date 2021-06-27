#!/bin/bash

make gbnclientmake

debug=0
portNumber=12345
maxPackets=400
packetErrorRate=0.00001

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -d) debug=1 ;;
        -p) portNumber=$2; shift ;;
        -n) maxPackets=$2; shift ;;
        -e) packetErrorRate=$2; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

./gbnclient $portNumber $maxPackets $packetErrorRate $debug

# ./gbnclient 12345 400 8 4 100 0.00001 0

echo "End of gbn client script !!"