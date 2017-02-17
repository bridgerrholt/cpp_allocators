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

	using Template = BitmappedBlock::Templated<VectorArray, 16, 8>;

	Template t;

	t.printMeta();
	std::cout << std::endl;

	auto a = t.allocate(16 * 4);

	t.printMeta();
	std::cout << std::endl << "reallocate 16*6\n";

	t.reallocate(a, 16 * 6);

	t.printMeta();
	std::cout << std::endl << "reallocate 16*4\n";

	std::cout << t.reallocate(a, 16 * 4) << '\n';

	t.printMeta();
	std::cout << std::endl << "reallocate 16*16\n";

	std::cout << t.reallocate(a, 16 * 16) << '\n';

	t.printMeta();
	std::cout << std::endl << "reallocate 16*4\n";

	std::cout << t.reallocate(a, 16 * 4) << '\n';

	t.printMeta();
	std::cout << std::endl;

	t.deallocateAll();

	t.printMeta();
	std::cout << std::endl;

	auto b = t.allocate(16 * 1);
	auto c = t.allocate(16 * 1);

	t.printMeta();
	std::cout << std::endl << "reallocate 16*2\n";

	t.reallocate(b, 16 * 2);

	t.printMeta();
	std::cout << std::endl << "reallocate 16*4\n";

	t.reallocate(c, 16 * 4);

	t.printMeta();
	std::cout << std::endl;




}