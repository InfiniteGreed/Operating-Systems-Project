#include <iostream>
#include "auth.h"
#include "process.h"
#include "scheduler.h"
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

    int Scheduler;
    cout << "\nDo you want to run CPU Scheduling?";
    cout << "\n 1.Yes";
    cout << "\n 1.No";

    cin >> Scheduler;

    if(Scheduler == 1)
    {
        int n = processes.size();
        

                vector<Process> sched(processes.begin(), processes.end());
        for (int i=0;i<n;i++) {
            sched[i].completion_time = -1;
            sched[i].turnaround_time = 0;
            sched[i].waiting_time = 0;
        }
int algo;
        cout << "\nPick algorithm: ";
        cout <<"\n1.FCFS";
        cout << "\n2.SJF";
        cout << "\n3.Round Robin\n";

        cin >> algo;

        if(algo==1) fcfs(sched,n);
        else if(algo==2) sjf(sched,n);
        else if(algo==3){
            int q;
            cout << "Time quantum? ";
            cin >> q;
            roundRobin(sched,n,q);
        }else cout << "Invalid choice\n";
    }

    cout << "\nThe System will shutdown" << endl;
    return 0;
}



        


