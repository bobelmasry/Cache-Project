#include <iostream>
#include <iomanip>
#include <cstdint>
#include <vector>
#include "random.cpp"
#include "cache.cpp"

// Tests line sizes 16,32,64,128 for gens 1-5, dump CSV
int main()
{
	const int NUM_ITERS = 1'000'000;
	const double MEM_PROB = 0.35, WRITE_PROB = 0.5;

	std::vector<int> blockSizes = {16, 32, 64, 128};

	// CSV header
	std::cout << "gen,lineSize,CPI,"
			  << "L1_acc,L1_hit,L1_miss,L1_wb,"
			  << "L2_acc,L2_hit,L2_miss,L2_wb\n";

	// Loop through each block size and memory generation
	for (int blockSize : blockSizes)
	{
		for (int gen = 1; gen <= 5; gen++)
		{
			// Reset random number generator state
			m_w = 0xABABAB55;
			m_z = 0x05080902;

			Cache L2(128 * 1024, blockSize, 8, 10, 50, nullptr);
			Cache L1(16 * 1024, blockSize, 4, 1, 0, &L2);

			int totalCycles = 0;

			L1.reset();
			L2.reset();
			for (int i = 0; i < NUM_ITERS; i++)
			{
				double p = getRandomDouble();
				// 35% chance of memory access, 65% no access
				if (p <= MEM_PROB)
				{
					unsigned addr = getAddress(gen);
					double r = getRandomDouble();
					// 50% chance of store, 50% load
					if (r < WRITE_PROB)
						totalCycles += L1.access(addr, MemOp::STORE);
					else
						totalCycles += L1.access(addr, MemOp::LOAD);
				}
				else
				{
					totalCycles += 1;
				}
			}
			// Print results
			double cpi = totalCycles / (double)NUM_ITERS;
			auto &s1 = L1.getStats();
			auto &s2 = L2.getStats();
			std::cout << gen << "," << blockSize << "," << std::fixed << std::setprecision(4)
					  << cpi << ","
					  << s1.accesses << "," << s1.hits << "," << s1.misses << "," << s1.writeBacks << ","
					  << s2.accesses << "," << s2.hits << "," << s2.misses << "," << s2.writeBacks << "\n";
		}
	}
	return 0;
}
