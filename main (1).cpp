#include <iostream>
#include "auth.h"
#include "process.h"
#include <vector>

using namespace std; 

int main()
{
    cout << "The Operating System is booting please autheticate.........." << endl;

    authentication();

    

    vector<Process> processes; 

    processes.push_back(Process(4,2, 5));
    processes.push_back(Process(6,1, 9));
    processes.push_back(Process(8,0, 3));
    processes.push_back(Process(3,2, 4));

    cout << "Process Exeution: " << endl;

    for (auto &process :processes)
    {
        process.start();
        process.finish(process.arrival_time + process.burst_time);
        process.display();
    }

    cout << "The System will shutdown" << endl;

    return 0;

        

}