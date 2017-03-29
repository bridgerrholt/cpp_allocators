#include <iostream>
#include <array>

#include <allocators/common/calc_lcm.h>
#include <allocators/bitmapped_block.h>

int main(int argc, char* argv[])
{
	using namespace bridgerrholt;
	using namespace bridgerrholt::allocators;

	using AllocatorType = BitmappedBlock::Templated<std::array, 16, 16>;

	AllocatorType allocator;

	std::cout << "Allocate\n";
	auto a = allocator.allocateAligned(16 * 8, 32);
	std::cout << a.getPtr() << '\n';


	return 0;
}