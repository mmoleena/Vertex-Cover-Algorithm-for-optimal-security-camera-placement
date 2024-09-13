#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>
#include <fstream>
#include <random>
#include <cstdlib>
#include <unistd.h>

using namespace std;

//Function to read random data from /dev/urandom
//Reference taken from online resources
unsigned int read_dev_urandom() 
{
    unsigned int random_value = 0;
    ifstream urandom("/dev/urandom", ios::in | ios::binary);
    if (urandom) 
    {
        urandom.read(reinterpret_cast<char*>(&random_value), sizeof(random_value));
        if (!urandom) 
        {
            cerr << "Failed reading from /dev/urandom" << endl;
            exit(1);
        }
        urandom.close();
    } 
    else 
    {
        cerr << "Failed opening /dev/urandom" << endl;
        exit(1);
    }
    return random_value;
}

// Returns random character
char shuffle_characters() 
{
    static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    return alphabet[read_dev_urandom() % (sizeof(alphabet) - 1)];
}

// This function generates random street names by choosing random letters and putting them into a string of arbitrary lengths
string StreetNameRandom() 
{
    string stName;
    int size;
    static mt19937 gen(read_dev_urandom());
    uniform_int_distribution<int> sizeDist(5, 14);
    size = sizeDist(gen);

    for (int i = 0; i < size; i++) 
    {
        stName += shuffle_characters();
    }
    return stName;
}

// This function is creating random coordinates within a specific range
int RandomCoordinates(int k) 
{
    static mt19937 gen(read_dev_urandom());
    uniform_int_distribution<int> Dist(-k, k);
    return Dist(gen);
}

// The below function is checking if two line segments are overlapping or not
bool OverlappingLineSeg(const pair<int, int>& p1, const pair<int, int>& p2) 
{
    return p1 == p2;
}

//This function adds and removes streets, assigns random names and coordinates to line segments and check the overlapping condition as well. 
void RandomInputGenerator(int MaxS, int MaxL, int SegNo, int CordRange, int NoOfTry) 
{
    srand(static_cast<unsigned int>(time(nullptr)));
    vector<string> recordOfStreets;
    bool addMode = true;
    int attempts = 0;

    while (attempts < NoOfTry) 
    {
        if (addMode) 
        {
            for (string toRemStreets : recordOfStreets) 
            {
                cout << "rm \"" << toRemStreets << "\"" << endl;
            }
            recordOfStreets.clear();
        }

        // Generate random StNo between 5 and MaxS
        int StNo = read_dev_urandom() % (MaxS - 5 + 1) + 5;

        for (int i = 0; i < StNo; i++) 
        {
            string command = addMode ? "add" : "rm";
            string stName = StreetNameRandom();
            bool validStreet = true;

            if (addMode && command == "add") 
            {
                vector<pair<int, int>> coordinates;

                int numOfcoord;
                do 
                {
                    numOfcoord = read_dev_urandom() % SegNo + 1;
                } 
                while (numOfcoord == 1);

                cout << "add \"" << stName << "\"";

                for (int j = 0; j < numOfcoord; j++) 
                {
                    int x, y;
                    bool overlap;

                    do {
                        x = RandomCoordinates(CordRange);
                        y = RandomCoordinates(CordRange);

                        overlap = false;
                        for (const pair<int, int>& coord : coordinates) 
                        {
                            if (OverlappingLineSeg(make_pair(x, y), coord)) 
                            {
                                overlap = true;
                                break;
                            }
                        }
                    } 
                    while (overlap);

                    coordinates.push_back(make_pair(x, y));
                    cout << " (" << x << "," << y << ")";
                }

                if (validStreet) 
                {
                    recordOfStreets.push_back(stName);
                    cout << endl;
                }
            }
        }

        if (addMode) 
        {
            // Generate random delay between 5 and MaxL
            int delay = read_dev_urandom() % (MaxL - 5 + 1) + 5;
            cout << "gg" << endl;
            sleep(delay);
        }

        addMode = !addMode;
        attempts++;

        // Error condition when the program fails to generate a valid specification for a continuous number of attempts
        if (attempts >= NoOfTry) 
        {
            cerr << "Error: failed to generate valid input for " << NoOfTry << " simultaneous attempts." << endl;
            exit(1);
        }
    }
}

//Below is the main function which parses command-line arguments
int main(int argc, char** argv) 
{
    int MaxS = 10, MaxL = 5, SegNo = 5, CordRange = 20, NoOfTry = 25;
    int opt;
    vector<string> errorMessages;

    while ((opt = getopt(argc, argv, "s:n:l:c:")) != -1) 
    {
        if (opt == 's') 
        {
            MaxS = atoi(optarg);
            if (MaxS < 2) 
            {
                errorMessages.push_back("Maximum number of streets cannot be less than 2.");
            }
        } 
        else if (opt == 'n') 
        {
            SegNo = atoi(optarg);
            if (SegNo < 1) 
            {
                errorMessages.push_back("Number of line segments in a street cannot be less than 1.");
            }
        } 
        else if (opt == 'l') 
        {
            MaxL = atoi(optarg);
            if (MaxL < 5) 
            {
                errorMessages.push_back("Maximum delay (l) cannot be less than 5.");
            }
        } 
        else if (opt == 'c') 
        {
            CordRange = atoi(optarg);
            if (CordRange < 1) 
            {
                errorMessages.push_back("K value for coordinate [-k,k] must be at least 1.");
            }
        } 
        else if (opt == '?') 
        {
            errorMessages.push_back("Unknown option or missing argument.");
        }
    }

    if (!errorMessages.empty()) 
    {
        for (const string& message : errorMessages) 
        {
            cerr << "Error: " << message << endl;
        }
        exit(1);
    }
    
    RandomInputGenerator(MaxS, MaxL, SegNo, CordRange, NoOfTry);

    return 0;
}

