#ifndef BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_0_GENERATE_INSTRUCTIONS_H
#define BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_0_GENERATE_INSTRUCTIONS_H

#include <iostream>

#include <cstddef>
#include <string>

#include "allocator_policy.h"
#include "instructions.h"

namespace bridgerrholt {
	namespace allocators {
		namespace tests {


union InstructionUnion
{
	public:
		InstructionUnion() {}
		~InstructionUnion() {}

		instructions::Allocate   allocate;
		instructions::Deallocate deallocate;
		instructions::Reallocate reallocate;
		instructions::Expand     expand;
};


class Instruction
{
	public:

	private:

};


enum class GeneratorFlags {
	ALLOW_REALLOCATE = 1 << 0,
	ALLOW_EXPAND     = 1 << 1,
	IS_STACK         = 1 << 2
};

GeneratorFlags operator|(GeneratorFlags a, GeneratorFlags b);


std::string generateInstructions(
	std::size_t    totalSize,
	std::size_t    minimumSize,
	std::size_t    maximumSize,
	GeneratorFlags flags =
		GeneratorFlags::ALLOW_REALLOCATE | GeneratorFlags::ALLOW_EXPAND);





		}
	}
}

#endif
