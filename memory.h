#ifndef MEMORY_H
#define MEMORY_H

#include <vector>
#include <unordered_map>
#include <queue>
#include <cstdint>
#include <iostream>

using PID = int;
using VPN = int;

struct Frame {
    PID pid = -1;
    VPN vpn = -1;
    bool used = false;
};

class MemoryManager {
public:
    MemoryManager(int numFrames = 4, int pageSize = 256);
    ~MemoryManager();

    void allocateProcess(PID pid, int numVirtualPages);
    void freeProcess(PID pid);

    
    bool access(PID pid, int virtualAddress);

   
    int translate(PID pid, int virtualAddress);

    void dumpState();

    int framesInUse();
    int getNumFrames() { return numFrames; }
private:
    int numFrames;
    int pageSize;
    std::vector<Frame> frames;
    std::queue<int> fifoQueue; 

    std::unordered_map<PID, std::vector<int>> pageTables;

    std::unordered_map<uint64_t, std::vector<char>> backingStore;

    int handlePageFault(PID pid, VPN vpn);
    uint64_t mkKey(PID pid, VPN vpn) { return (uint64_t(pid) << 32) | uint32_t(vpn); }
    
};

#endif
