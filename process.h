#ifndef PROCESS_H 
#define PROCESS_H 

#include <string> 

using namespace std; 

class Process 
{
    public: 
    int pid, arrival_time, burst_time, completion_time, remaining_time, waiting_time, turnaround_time; 
    string process_occurance; 

    Process (int id, int arrival, int burst); 

    void start();
    void display();
    void wait();
    void finish(int time); 
}; 

#endif
