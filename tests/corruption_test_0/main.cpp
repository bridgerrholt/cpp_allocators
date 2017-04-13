#include <allocators/stack_allocator.h>

#include "instruction_output.h"
#include "generate_instructions.h"
#include "stack_generator.h"

int main(int argc, char* argv[])
{
	using namespace brh::allocators;
	using namespace brh::allocators::tests;
	using namespace brh::allocators::tests::instructions;


	using Allocator = StackAllocator::Templated<std::array, 16>;
	using AllocatorPolicy = BasicAllocatorPolicy<Allocator>;

	AllocatorPolicy allocatorPolicy;
	BlockList       blockList;

	auto list = StackGenerator::generateFillSequence(allocatorPolicy, 16);

	std::cout << list;

	return 0;
}