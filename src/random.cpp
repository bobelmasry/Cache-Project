#ifndef RANDOM_CPP
#define RANDOM_CPP

// File that provides random number generation and memory address generation functions

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <vector>

static unsigned int m_w = 0xABABAB55;
static unsigned int m_z = 0x05080902;

unsigned int rand_()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;
}

#define DRAM_SIZE (64 * 1024 * 1024)

unsigned int memGen1()
{
    static unsigned int addr = 0;
    return (addr++) % DRAM_SIZE;
}
unsigned int memGen2()
{
    static unsigned int addr = 0;
    return rand_() % (24 * 1024);
}
unsigned int memGen3() { return rand_() % DRAM_SIZE; }
unsigned int memGen4()
{
    static unsigned int addr = 0;
    return (addr++) % (4 * 1024);
}
unsigned int memGen5()
{
    static unsigned int addr = 0;
    return (addr += 32) % (64 * 16 * 1024);
}

unsigned int getAddress(int gen)
{
    switch (gen)
    {
    case 1:
        return memGen1();
    case 2:
        return memGen2();
    case 3:
        return memGen3();
    case 4:
        return memGen4();
    default:
        return memGen5();
    }
}

double getRandomDouble()
{
    return rand_() / (double)UINT32_MAX;
}

#endif // RANDOM_CPP