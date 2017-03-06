#ifndef BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_0_GENERATION_ARG_PACK_H
#define BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_0_GENERATION_ARG_PACK_H

#include <cstddef>
#include "allocator_policy.h"

namespace bridgerrholt {
	namespace allocators {
		namespace tests {

class GenerationArgPack
{
	public:
		GenerationArgPack(AllocatorPolicy & allocator,
		                  std::size_t       totalSize);

		GenerationArgPack(AllocatorPolicy & allocator,
		                  std::size_t       totalSize,
		                  std::size_t       minAllocationSize,
		                  std::size_t       maxAllocationSize,
		                  std::size_t       minInstructionCount);


	private:
		AllocatorPolicy & allocator_;
		std::size_t       totalSize_;
		std::size_t       minAllocationSize_;
		std::size_t       maxAllocationSize_;
		std::size_t       minimumInstructionCount;
};

		}
	}
}

#endif
