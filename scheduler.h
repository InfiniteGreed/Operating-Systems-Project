#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include <vector>

void fcfs(std::vector<Process> &p, int n);
void sjf(std::vector<Process> &p, int n);
void roundRobin(std::vector<Process> &p, int n, int quantum);

#endif
