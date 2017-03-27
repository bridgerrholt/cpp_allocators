#include <iostream>
#include <array>

#include <allocators/bitmapped_block.h>
#include <allocators/stack_allocator.h>

#include "run_stack_test.h"

int main(int argc, char* argv[])
{
	using namespace bridgerrholt::allocators;
	using namespace tests;

	constexpr std::size_t size {1024};
	constexpr std::size_t minBlockSize {32};
	constexpr std::size_t blockCount {size / minBlockSize};

	using AllocatorCore = BitmappedBlock::Templated<
		std::array, minBlockSize, blockCount>;

	using Allocator = BasicAllocatorPolicy<AllocatorCore>;

	Allocator allocator {};

	std::cout << runStackTest({allocator, size, 1, 512, 1000});
}