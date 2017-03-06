#include <iostream>
#include <array>

#define BRH_CPP_ALLOCATORS_MULTITHREADED

#include <allocators/bitmapped_block.h>

using namespace bridgerrholt::allocators;

using Type = long;

using AllocatorCore = BitmappedBlock::Templated<
	std::array, sizeof(Type), 16, sizeof(Type)
>;

using Allocator = AllocatorWrapper<AllocatorCore>;
using BlockType = BasicBlock<Type>;

Allocator g_allocator;

void allocate(Type value, BlockType & outBlock) {
	outBlock = g_allocator.construct<Type>(value);
}

int main(int argc, char* argv[])
{
	BlockType block1;
	allocate(99, block1);

	std::cout << block1.getPtr() << '\n';


	return 0;
}