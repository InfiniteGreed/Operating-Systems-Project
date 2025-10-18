#include "process.h"
#include <iostream>

using namespace std;

Process::Process(int process_ID, int process_arrival, int process_burst)
{
    pid = process_ID;
    arrival_time = process_arrival;
    burst_time = process_burst;
    remaining_time = process_burst; 
    completion_time = 0;
    waiting_time = 0;
    turnaround_time = 0;
    process_occurance = "New process created.";


}

void Process::start()
{
    process_occurance = "System process is running.";
}

void Process::wait()
{
    process_occurance = "System process is waiting.";
}

void Process::finish(int time)
{
    completion_time = time;
    turnaround_time = completion_time - arrival_time;
    waiting_time = turnaround_time - burst_time;
    process_occurance = "System process has finished execution.";
}

void Process::display()
{
    cout << "Process ID: " << pid << endl
         << "Arrival Time: " << arrival_time << endl
         << "Burst Time: " << burst_time << endl
         << "Completion Time: " << completion_time << endl
         << "Waiting Time: " << waiting_time << endl
         << "Turnaround Time: " << turnaround_time << endl;
}



