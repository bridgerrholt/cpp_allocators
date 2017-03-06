#ifndef BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_0_STACK_GENERATOR_H
#define BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_0_STACK_GENERATOR_H

#include "instruction_list.h"
#include "generator_flags.h"

namespace bridgerrholt {
	namespace allocators {
		namespace tests {

class StackGenerator
{
	public:
		InstructionList generate(
			AllocatorPolicy & allocator,
			std::size_t       totalSize,
			std::size_t       minimumSize,
			std::size_t       maximumSize,
			std::size_t       minimumInstructionCount,
			AllowedOperations allowedOperations);
};

		}
	}
}

#endif
