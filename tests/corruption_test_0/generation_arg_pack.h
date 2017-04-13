#ifndef BRH_CPP_ALLOCATORS_CORRUPTION_TEST_0_GENERATION_ARG_PACK_H
#define BRH_CPP_ALLOCATORS_CORRUPTION_TEST_0_GENERATION_ARG_PACK_H

#include <cstddef>
#include "allocator_policy.h"

namespace brh {
	namespace allocators {
		namespace tests {

struct GenerationArgPack
{
	AllocatorPolicy & allocator;
	std::size_t       totalSize;
	std::size_t       minAllocationSize;
	std::size_t       maxAllocationSize;
	std::size_t       minInstructionCount;
};

		}
	}
}

#endif
