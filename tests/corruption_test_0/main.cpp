#include <allocators/stack_allocator.h>

#include "instruction_output.h"
#include "generate_instructions.h"

int main(int argc, char* argv[])
{
	using namespace bridgerrholt::allocators;
	using namespace bridgerrholt::allocators::tests;
	using namespace bridgerrholt::allocators::tests::instructions;


	using Allocator = StackAllocator::Templated<std::array, 16>;
	using AllocatorPolicy = BasicAllocatorPolicy<Allocator>;

	AllocatorPolicy allocatorPolicy;
	BlockList       blockList;

	Generator::FlagsWrapper flagsWrapper {Generator::FlagsEnum::ALL};
	std::cout << flagsWrapper.getNumeric() << '\n';

	auto list = generateInstructions(allocatorPolicy, 16, 1, 16,
	                                 {Generator::FlagsEnum::ALL});

	std::cout << list;

	return 0;
}