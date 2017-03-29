#include <array>

#include <allocators/bitmapped_block.h>

int main(int argc, char* argv[])
{
	using namespace bridgerrholt::allocators;

	using AllocatorType = BitmappedBlock::Templated<std::array, 16, 16>;

	AllocatorType allocator;

	auto a = allocator.allocate(16 * 8);
	auto b = allocator.allocate(16 * 8);

	allocator.deallocate(a);
	allocator.deallocate(b);

	auto c = allocator.allocate(16 * 16);

	return 0;
}