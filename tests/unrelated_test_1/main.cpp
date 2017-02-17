#include <array>
#include <vector>
#include <random>
#include <type_traits>
#include <iostream>

#include "../performance_test_0/get_time.h"

#include <allocators/stack_allocator.h>
#include <allocators/bitmapped_block.h>
#include <allocators/wrappers/allocator_wrapper.h>

template <class T>
using VectorWrapper = std::vector<T>;

template <class T, std::size_t size>
class VectorArray : public std::vector<T>
{
	public:
		VectorArray() {
			this->reserve(size);
		}
};

int main(int argc, char* argv[])
{
	using namespace bridgerrholt::allocators;

	/*using Template = StackAllocator::Templated<VectorArray, 16>;
	using Runtime  = StackAllocator::Runtime  <VectorWrapper>;

	Runtime t {{16}};

	auto i = t.allocate(4);
	auto j = t.allocate(4);

	std::cout << i.getPtr() << ' ' << i.getSize() << '\n';
	std::cout << j.getPtr() << ' ' << j.getSize() << '\n';

	std::cout << t.reallocate(i, 8) << '\n';

	std::cout << i.getPtr() << ' ' << i.getSize() << '\n';

	t.deallocate(j);

	std::cout << t.reallocate(i, 8) << '\n';

	std::cout << i.getPtr() << ' ' << i.getSize() << '\n';

	t.deallocate(i);*/

	using Template = BitmappedBlock::Templated<VectorArray, 16, 16>;

	Template t;

	t.printMeta();
	std::cout << std::endl;

	auto i = t.allocate(16);
	auto j = t.allocate(16);
	auto k = t.allocate(16);

	t.deallocate(j);

	t.printMeta();
	std::cout << std::endl;
	std::cout << i.getPtr() << " " << i.getSize() << '\n';

	std::cout << t.expand(k, 16 * 13) << '\n';

	t.printMeta();
	std::cout << std::endl;
	std::cout << i.getPtr() << " " << i.getSize() << '\n';

}