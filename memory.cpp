#include "memory.h"

MemoryManager::MemoryManager(int numFrames_, int pageSize_)
    : numFrames(numFrames_), pageSize(pageSize_), frames(numFrames_)
{

}

MemoryManager::~MemoryManager()
{
}

void MemoryManager::allocateProcess(PID pid, int numVirtualPages)
{
    std::vector<int> pt(numVirtualPages, -1);
    pageTables[pid] = pt;

    for (int v = 0; v < numVirtualPages; ++v) {
        uint64_t key = mkKey(pid, v);
        backingStore[key] = std::vector<char>(pageSize, char((v & 0xFF)));
    }

    std::cout << "Allocated process " << pid << " with " << numVirtualPages << " pages\n";
}

void MemoryManager::freeProcess(PID pid)
{
    for (int i = 0; i < numFrames; ++i) {
        if (frames[i].used && frames[i].pid == pid) {
            frames[i].used = false;
            frames[i].pid = -1;
            frames[i].vpn = -1;
        }
    }

    pageTables.erase(pid);

    std::vector<uint64_t> keys;
    for (auto &kv : backingStore) {
        uint64_t k = kv.first;
        PID p = int(k >> 32);
        if (p == pid) keys.push_back(k);
    }
    for (auto k : keys) backingStore.erase(k);
}

int MemoryManager::handlePageFault(PID pid, VPN vpn)
{
    std::cout << "Page fault: pid=" << pid << " vpn=" << vpn << "\n";

    for (int i = 0; i < numFrames; ++i) {
        if (!frames[i].used) {
            frames[i].used = true;
            frames[i].pid = pid;
            frames[i].vpn = vpn;
            pageTables[pid][vpn] = i;
            fifoQueue.push(i);
            return i;
        }
    }

    if (fifoQueue.empty()) return -1;
    int victim = fifoQueue.front(); fifoQueue.pop();
    PID oldPid = frames[victim].pid;
    VPN oldVpn = frames[victim].vpn;

    if (pageTables.count(oldPid)) {
        pageTables[oldPid][oldVpn] = -1;
    }

    frames[victim].pid = pid;
    frames[victim].vpn = vpn;
    pageTables[pid][vpn] = victim;
    fifoQueue.push(victim);

    std::cout << "Evicted pid=" << oldPid << " vpn=" << oldVpn << " from frame " << victim << "\n";
    return victim;
}

bool MemoryManager::access(PID pid, int virtualAddress)
{
    if (!pageTables.count(pid)) {
        std::cerr << "access: unknown pid " << pid << "\n";
        return false;
    }
    int vpn = virtualAddress / pageSize;
    int offset = virtualAddress % pageSize;
    if (vpn < 0 || vpn >= (int)pageTables[pid].size()) {
        std::cerr << "access: virtual address out of range for pid " << pid << "\n";
        return false;
    }

    int frame = pageTables[pid][vpn];
    if (frame == -1) {
        frame = handlePageFault(pid, vpn);
        if (frame == -1) return false;
    }

    int physical = frame * pageSize + offset;
    std::cout << "pid=" << pid << " vaddr=" << virtualAddress << " -> paddr=" << physical << " (frame=" << frame << ",vpn=" << vpn << ")\n";
    return true;
}

int MemoryManager::translate(PID pid, int virtualAddress)
{
    if (!pageTables.count(pid)) return -1;
    int vpn = virtualAddress / pageSize;
    int offset = virtualAddress % pageSize;
    if (vpn < 0 || vpn >= (int)pageTables[pid].size()) return -1;
    int frame = pageTables[pid][vpn];
    if (frame == -1) return -1;
    return frame * pageSize + offset;
}

void MemoryManager::dumpState()
{
    std::cout << "\nMemory State:\n";
    for (int i = 0; i < numFrames; ++i) {
        if (frames[i].used)
            std::cout << "Frame " << i << ": pid=" << frames[i].pid << " vpn=" << frames[i].vpn << "\n";
        else
            std::cout << "Frame " << i << ": free\n";
    }

    std::cout << "Page tables:\n";
    for (auto &kv : pageTables) {
        PID pid = kv.first;
        auto &pt = kv.second;
        std::cout << " pid=" << pid << ": ";
        for (size_t i = 0; i < pt.size(); ++i) std::cout << pt[i] << " ";
        std::cout << "\n";
    }
}

int MemoryManager::framesInUse()
{
    int count = 0;
    for (int i = 0; i < numFrames; ++i) if (frames[i].used) ++count;
    return count;
}
