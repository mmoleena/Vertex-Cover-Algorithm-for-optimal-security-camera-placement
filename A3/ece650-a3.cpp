#include<vector>
#include<iostream>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<signal.h>
#include<cstdlib>

using namespace std;

int main(int argc, char** argv) 
{
    int StNo = 10, SegNo = 5, delay = 5, CordRange = 20;
    int opt;
    vector<string> errorMessages;
    vector<pid_t> processes;

    while ((opt = getopt(argc, argv, "s:n:l:c:")) != -1) 
    {
        if (opt == 's') 
        {
            StNo = atoi(optarg);
            if (StNo < 2) 
            {
                errorMessages.push_back("Number of streets cannot be less than 2.");
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
            delay = atoi(optarg);
            if (delay < 5) 
            {
                errorMessages.push_back("Number of seconds cannot be less than 5.");
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

    int rgenPipe[2];
    int cppPipe[2];
    pipe(rgenPipe);
    pipe(cppPipe);
    if (pipe(rgenPipe) == -1) 
    {
        cerr << "Error: Pipe creation failed" << endl;
        return 1;
    }

    pid_t rgenPid, pythonPid, cppPid;
    rgenPid = fork();
    if (rgenPid == 0) 
    {
        close(rgenPipe[0]);
        dup2(rgenPipe[1], STDOUT_FILENO);
        execv("./rgen", argv);
    }
    processes.push_back(rgenPid);

    pythonPid = fork();
    if (pythonPid == 0) 
    {
        close(rgenPipe[1]);
        dup2(rgenPipe[0], STDIN_FILENO);
        close(cppPipe[0]);
        dup2(cppPipe[1], STDOUT_FILENO);

        execlp("python", "python", "ece650-a1.py", nullptr);
    } 
    processes.push_back(pythonPid);
    

    cppPid = fork();
    if (cppPid == 0) 
    {
        close(cppPipe[1]);
        dup2(cppPipe[0],STDIN_FILENO);
        execlp("./ece650-a2", "./ece650-a2", nullptr);
    }
    processes.push_back(cppPid);

    dup2(cppPipe[1], STDOUT_FILENO);

    while (!std::cin.eof())
    {
        std::string line;
        std::getline(std::cin, line);
        std::cout << line << std::endl;
    }

    close(rgenPipe[0]);
    close(rgenPipe[1]);
    close(cppPipe[0]);
    close(cppPipe[1]);

    int status;

    for(auto process : processes)
    {
      kill(process, SIGTERM);
      waitpid(process, &status, 0);
    }

    return 0;
}
