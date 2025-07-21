#ifndef CACHE_CPP
#define CACHE_CPP

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <vector>
#include "random.cpp"

// Memory operation types
enum class MemOp
{
    LOAD,
    STORE
};

// Cache statistics structure
// Contains counters for accesses, hits, misses, and write-backs
struct CacheStats
{
    uint64_t accesses = 0, hits = 0, misses = 0, writeBacks = 0;
};

// Generic cache class
// Supports multiple levels of cache with configurable parameters
// Each cache can have a next level (e.g., L2 cache)
// Implements Write Back algorithm
// Random replacement policy for cache lines
// Accesses return the number of cycles taken
class Cache
{
public:
    // Constructor initializes cache parameters
    // cacheSizeBytes: total size of the cache in bytes
    // blockSizeBytes: size of each cache block in bytes
    // associativity: number of lines per set (1 for direct-mapped)
    // hitTimeCycles: cycles taken for a hit in this cache
    // memTimeCycles: cycles taken to access memory or next level cache
    // nextLevel: pointer to the next level cache (nullptr if none)
    Cache(size_t cacheSizeBytes, size_t blockSizeBytes, size_t associativity,
          unsigned hitTimeCycles, unsigned memTimeCycles, Cache *nextLevel = nullptr)
        : cacheSize(cacheSizeBytes), blockSize(blockSizeBytes), assoc(associativity), hitTime(hitTimeCycles),
          memTime(memTimeCycles), next(nextLevel)
    {
        numSets = cacheSizeBytes / (blockSizeBytes * associativity);
        sets.resize(numSets, std::vector<Line>(assoc));
    }

    // Access the cache with a given address and memory operation
    // Returns the number of cycles taken for the access
    unsigned access(uint64_t addr, MemOp op)
    {
        stats.accesses++;
        size_t setIdx = (addr / blockSize) % numSets;
        uint64_t tag = (addr / blockSize) / numSets;
        // hit?
        for (auto &line : sets[setIdx])
        {
            if (line.valid && line.tag == tag)
            {
                stats.hits++;
                if (op == MemOp::STORE) // mark as dirty if store operation
                    line.dirty = true;
                return hitTime;
            }
        }
        // miss? fetch from next level
        stats.misses++;
        unsigned cycles = 0;
        if (next)
            cycles += next->access(addr, MemOp::LOAD);
        else
            cycles += memTime;

        // find line to replace (invalid if possible, else random)
        Line *toReplace = nullptr;
        for (auto &line : sets[setIdx])
            if (!line.valid)
            {
                toReplace = &line;
                break;
            }
        // no invalid line found, pick a random one
        if (!toReplace)
            toReplace = &sets[setIdx][rand_() % assoc];

        // if the line to replace (victim) is dirty, do a write-back to the next level/memory
        if (toReplace->valid && toReplace->dirty)
        {
            stats.writeBacks++;
            uint64_t evAddr = (toReplace->tag * numSets + setIdx) * blockSize;
            if (next)
                cycles += next->access(evAddr, MemOp::STORE);
            else
                cycles += memTime;
        }
        // update line with new data
        toReplace->valid = true;
        toReplace->tag = tag;
        toReplace->dirty = (op == MemOp::STORE); // mark as dirty if store operation
        return cycles + hitTime;
    }

    // Reset the cache statistics and contents
    void reset()
    {
        stats = CacheStats();
        for (auto &set : sets)
            for (auto &l : set)
                l = {};
    }

    const CacheStats &getStats() const
    {
        return stats;
    }

private:
    // Represents a single cache line
    struct Line
    {
        bool valid = false;
        bool dirty = false;
        uint64_t tag = 0;
    };
    size_t cacheSize, blockSize, assoc, numSets;
    unsigned hitTime, memTime;
    Cache *next;
    std::vector<std::vector<Line>> sets;
    CacheStats stats;
};

#endif // CACHE_CPP