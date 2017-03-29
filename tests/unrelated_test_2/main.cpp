#include <iostream>
#include <array>

#include <allocators/bitmapped_block.h>

int main(int argc, char* argv[])
{
	using namespace bridgerrholt::allocators;

	using AllocatorType = BitmappedBlock::Templated<std::array, 16, 16>;

	AllocatorType allocator;

	auto a = allocator.allocateAligned(16 * 8, 32);
	std::cout << a.getPtr() << '\n';


	return 0;
}