#ifndef BRH_CPP_ALLOCATORS_CORRUPTION_TEST_0_STACK_GENERATOR_H
#define BRH_CPP_ALLOCATORS_CORRUPTION_TEST_0_STACK_GENERATOR_H

#include <random>

#include "instruction_list.h"
#include "generator_flags.h"
#include "generation_arg_pack.h"

namespace brh {
	namespace allocators {
		namespace tests {

class StackGenerator
{
	public:
		using RandomEngineType = std::mt19937;

		static InstructionList generate(
			GenerationArgPack generationArgs,
			AllowedOperations allowedOperations);

		static InstructionList generateFillSequence(
			AllocatorPolicy & allocator,
		  SizeType          totalSize
		);

		static InstructionList generateMainSequence(
			GenerationArgPack generationArgs,
			AllowedOperations allowedOperations
		);


	private:
		static void generateFillSequence(
			InstructionList & list,
		  AllocatorPolicy & allocator,
		  SizeType          totalSize
		);

		static void generateMainSequence(
			InstructionList         & list,
			GenerationArgPack const & generationArgs,
			AllowedOperations         allowedOperations
		);

		RandomEngineType randomEngine_;
};

		}
	}
}

#endif
