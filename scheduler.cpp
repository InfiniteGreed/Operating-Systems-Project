#include <iostream>
#include "process.h"
#include <vector>
#include "scheduler.h"
using namespace std;

void fcfs(std::vector<Process> &p, int n) 
{
    cout << "\n==== FCFS Scheduling ====\n";
    int totalWT = 0, totalTAT = 0;

    for(int i=0;i<n-1;i++)
    {
        for(int j=0;j<n-i-1;j++)
        {
            if(p[j].arrival_time > p[j+1].arrival_time)
            {
                Process temp = p[j];
                p[j] = p[j+1];
                p[j+1] = temp;
            }
        }
    }

    int current = 0;
    for(int i=0;i<n;i++)
    {
        if(current < p[i].arrival_time)
            current = p[i].arrival_time;

        current += p[i].burst_time;
        p[i].completion_time = current;
        p[i].turnaround_time = p[i].completion_time - p[i].arrival_time;
        p[i].waiting_time = p[i].turnaround_time - p[i].burst_time;

        totalWT += p[i].waiting_time;
        totalTAT += p[i].turnaround_time;
    }

    cout << "PID AT BT CT TAT WT\n";
    for(int i=0;i<n;i++)
    {
        cout << p[i].pid << " " << p[i].arrival_time << " " << p[i].burst_time
             << " " << p[i].completion_time << " " << p[i].turnaround_time
             << " " << p[i].waiting_time << endl;
    }

    cout << "\nAverage Turnaround Time: " << (float)totalTAT/n;
    cout << "\nAverage Waiting Time: " << (float)totalWT/n << endl;
}


void sjf(std::vector<Process> &p, int n) 
{
    cout << "\n==== SJF (Non-Preemptive) ====\n";
    int completed = 0, time = 0;
    float totalWT = 0, totalTAT = 0;

    for(int i=0;i<n;i++)
    {
        p[i].completion_time = -1;
    }

    while(completed != n)
    {
        int idx = -1, minBT = 9999;
        for(int i=0;i<n;i++)
        {
            if(p[i].arrival_time <= time && p[i].completion_time == -1)
            {
                if(p[i].burst_time < minBT)
                {
                    minBT = p[i].burst_time;
                    idx = i;
                }
            }
        }

        if(idx != -1)
        {
            time += p[idx].burst_time;
            p[idx].completion_time = time;
            p[idx].turnaround_time = p[idx].completion_time - p[idx].arrival_time;
            p[idx].waiting_time = p[idx].turnaround_time - p[idx].burst_time;
            totalWT += p[idx].waiting_time;
            totalTAT += p[idx].turnaround_time;
            completed++;
        }else
        {
            time++;
        }
    }

    cout << "PID AT BT CT TAT WT\n";
    for(int i=0;i<n;i++)
    {
        cout << p[i].pid << " " << p[i].arrival_time << " " << p[i].burst_time
             << " " << p[i].completion_time << " " << p[i].turnaround_time
             << " " << p[i].waiting_time << endl;
    }

    cout << "\nAverage Turnaround Time: " << totalTAT/n;
    cout << "\nAverage Waiting Time: " << totalWT/n << endl;
}


void roundRobin(std::vector<Process> &p, int n, int quantum) 
{
    cout << "\n==== Round Robin ====\n";
    std::vector<int> remBT(n);
    int time = 0, done = 0;
    float totalWT = 0, totalTAT = 0;

    for(int i=0;i<n;i++)
    {
        remBT[i] = p[i].burst_time;
        p[i].completion_time = -1;
    }

    while(done < n)
    {
        bool allDone = true;
        for(int i=0;i<n;i++)
        {
            if(remBT[i] > 0)
            {
                allDone = false;
                if(remBT[i] > quantum)
                {
                    time += quantum;
                    remBT[i] -= quantum;
                }else
                {
                    time += remBT[i];
                    remBT[i] = 0;
                    p[i].completion_time = time;
                    p[i].turnaround_time = p[i].completion_time - p[i].arrival_time;
                    p[i].waiting_time = p[i].turnaround_time - p[i].burst_time;
                    totalWT += p[i].waiting_time;
                    totalTAT += p[i].turnaround_time;
                    done++;
                }
            }
        }
        if(allDone) break;
    }

    cout << "PID AT BT CT TAT WT\n";
    for(int i=0;i<n;i++)
    {
        cout << p[i].pid << " " << p[i].arrival_time << " " << p[i].burst_time
             << " " << p[i].completion_time << " " << p[i].turnaround_time
             << " " << p[i].waiting_time << endl;
    }

    cout << "\nAverage Turnaround Time: " << totalTAT/n;
    cout << "\nAverage Waiting Time: " << totalWT/n << endl;
}
