I recorded the testing session with 'script' command in python.
I used this command to screen record:
    $ script --timing=file_time shell_record.md

To replay the recording, use the following command:
    $ scriptreplay file_time shell_record.md

To execute the program:
1. First give execute permissions to bashscript bash file.
2. Now simply run the command: $./bashscript

OR

1. Just run the command: $ bash bashscript

OR

1. Just use the Makefile by running: $ make cw

This bash script iterates over all the 32 unique sets of tuples and does the following:
1. It calls cw.cpp C++ file using this 5 arguments along with a counter and creates a output file
2. It calls plot.py python file to use this output file generated and plot it in image file (.png).

To run the the cpp file manually and give test cases manually, use the following command:
1. g++ cw.cpp -o cw
2. ./cw <ki> <km> <kn> <kf> <ps> <num> where num is some number such that outfile name becomes "output${num}.txt"

For plotting output3.txt which contains integral cw size in each line, use the command: 
$ python plot.py <num> <ki> <km> <kn> <kf> <ps> where num is some number (here = 3) and finally plots in the image file image3.png
