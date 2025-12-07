# The way this memory manager works is fairly straightforward. First, the required variables and attributes were used to create the MemoryManager class.
# Two separate files were created for this program, one for the class implementation and one for the header file.
# A constructor was used to initialize the number of frames and page size, then the functions control how memory is accessed and managed.
# These functions handle process allocation, page faults, address translation, and memory replacement using a FIFO algorithm.
# Finally, a dumpState() function displays the current memory and page table information that the user would see in the simulated operating system.