#include <array>
#include <vector>
#include <random>
#include <type_traits>
#include <iostream>

#include "../performance_test_0/get_time.h"

#include <allocators/stack_allocator.h>
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

	using Template = StackAllocator::Templated<VectorArray, 16>;
	using Runtime  = StackAllocator::Runtime  <VectorWrapper>;

	Runtime t {{16}};

	auto i = t.allocate(8);
	auto j = t.allocate(8);

	std::cout << i.getPtr() << '\n';
	std::cout << j.getPtr() << '\n';

	t.deallocate(j);
	t.deallocate(i);

	int i1 {};

	BasicBlock<int> b1 {&i1, 10};
	BasicBlock<int> b2 {&i1, 10};

	std::cout << (b1 == b2) << '\n';

}