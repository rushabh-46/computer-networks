make clean

make

lastNode=7
input="input1.txt"
output="outputOSFP"
text=".txt"

X=1
Y=5
Z=20

## Run the executable on all the servers in parallel
for ((id = 0; id <= lastNode; id++))
do
    ./ospf $id $input $output$id$text $X $Y $Z &
done

echo "Done"