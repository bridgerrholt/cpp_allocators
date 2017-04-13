#include <iostream>
#include <array>

#include <supports/calc_is_aligned.h>
#include <supports/calc_lcm.h>
#include <allocators/traits/array_interface.h>
#include <allocators/traits/traits.h>
#include <allocators/bitmapped_block.h>

int main(int argc, char* argv[])
{
	using namespace brh;
	using namespace brh::allocators;

	using AllocatorType = BitmappedBlock::Templated<traits::VectorInterface::Templated, 16, 1024 * 16>;

	AllocatorType allocator;

	SizeType size      {16 * 8};

	SizeType alignment  {16};
	SizeType factor     {2};
	SizeType iterations {20};

	for (std::size_t i {0}; i < iterations; ++i) {
		std::cout << "allocateAligned(" << size << ", " << alignment << ")\n";
		auto a = allocator.allocateAligned(size, alignment);
		std::cout << a.getPtr() << '\n';
		std::cout << "Aligned:   " << supports::calcIsAligned(a.getPtr(), alignment) << '\n';
		std::cout << "Division:  " << reinterpret_cast<std::uintptr_t>(a.getPtr()) / alignment << '\n';
		allocator.deallocate(a);
		alignment *= factor;
		std::cout << std::endl;
	}


	return 0;
}