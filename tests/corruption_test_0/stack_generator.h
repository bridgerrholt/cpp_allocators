#ifndef BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_0_STACK_GENERATOR_H
#define BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_0_STACK_GENERATOR_H

#include "instruction_list.h"
#include "generator_flags.h"
#include "generation_arg_pack.h"

namespace bridgerrholt {
	namespace allocators {
		namespace tests {

class StackGenerator
{
	public:
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
};

		}
	}
}

#endif
