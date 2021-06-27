/**
 * The CW emulator for the Assignment 2 of Computer Networks CS3205.
 * It involves a modified AIMD algorithm
 * @author Rushabh Lalwani CS18B046
 */

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <time.h>

using namespace std;

#define MAX_DOUBLE 1e9
#define RWS 1024
#define MSS 1

/**
 * used to generate random timer value and accordingly 
 * know whether its a timeout or not based on probability ps.
 * 
 * @param ps the probability of having a timeout
 * @return whether timeout occured or not
 */
bool getTime(double ps)
{
    int reciprocal = (int)(1.0 / ps);
    int random = rand() % reciprocal;
    return (random == 0);
}

/**
 * The main function for the CW values generator.
 * @param argc 
 * @param argv 
 */
int main(int argc, char *argv[])
{
    double ki = std::stod(argv[1]);
    double km = std::stod(argv[2]);
    double kn = std::stod(argv[3]);
    double kf = std::stod(argv[4]);
    double ps = std::stod(argv[5]);

    // <T> the number of times to folllow the algorithm.
    int tCount = 1024;

    // The output file name for the cw values which are enerated.
    int outputFileNum = stoi(argv[6]);
    string fileName = "output" + to_string(outputFileNum) + ".txt";

    cout << "ki = " << ki << "; km = " << km << "; kn = " << kn << "; kf = " << kf << "; ps = " << ps;
    cout << endl;

    // setting the srand() to get new timer values for each run
    srand(time(0));

    // output file stream.
    ofstream out_file;

    out_file.open(fileName);

    // Initialize the cw size
    double cwSize = ki;
    double threshold = (double)MAX_DOUBLE;
    bool isTimeout = false;

    // Printing the initial cw size
    out_file << ceil(cwSize) << endl;

    /// Loop for <T> times
    while (tCount--)
    {
        // check if timeout or not
        isTimeout = getTime(ps);
        if (isTimeout)
        {
            // TIMEOUT !!!
            // 1. Half the threshold to cw / 2
            threshold = cwSize / 2.0;

            // 2. Update the cw size
            cwSize = max(1.0, kf * cwSize);
        }
        else
        {
            // NOT A TIMEOUT !!!
            // Check the threshold size and compare with the cw size
            if (cwSize < threshold)
            {
                // Exponential growth
                cwSize = min(cwSize + km * MSS, (double)RWS);
            }
            else
            {
                // Linear growth
                cwSize = min(cwSize + (ki * MSS * MSS / cwSize), (double)RWS);
            }
        }

        // File write to store the CW size.
        out_file << (int)ceil(cwSize) << endl;
    }

    // close the file.
    out_file.close();
}