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

template <class T>
std::string formatArray(void * start, std::size_t elements,
                        std::string separator = " ") {
	auto begin = static_cast<T*>(start);
	auto end   = begin + elements;
	std::string toReturn;
	for (auto i = begin; i < end; ++i) {
		toReturn += std::to_string(*i) + separator;
	}

	return toReturn;
}

template <class T>
void setArrayElements(void * start, std::size_t elements, T value) {
	auto begin = static_cast<T*>(start);
	auto end   = begin + elements;

	for (auto i = begin; i < end; ++i) {
		*i = value;
	}
}

template <class T>
void printMeta(T & t) {
	t.printMeta();
	std::cout << std::endl;
}

template <class T>
bridgerrholt::allocators::RawBlock
allocatePrint(T & t, std::size_t blockCount) {
	auto toReturn = t.allocateBlocks(blockCount);
	printMeta(t);
	return toReturn;
}

template <class T>
void deallocatePrint(T & t, bridgerrholt::allocators::RawBlock block) {
	t.deallocate(block);
	printMeta(t);
}

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

	using Template = BitmappedBlock::Templated<VectorArray, 8, 16, 8>;

	Template t;

	printMeta(t);

	auto a = allocatePrint(t, 3);
	auto b = allocatePrint(t, 7);
	deallocatePrint(t, a);
	auto d = allocatePrint(t, 4);
	auto e = allocatePrint(t, 3);

	/*Template t;

	printMeta(t);

	auto a = t.allocate(8 * 4);
	auto b = t.allocate(8 * 4);
	auto const start = a.getPtr();

	printMeta(t);

	using Type = uint64_t;

	setArrayElements<Type>(a.getPtr(), 4, 7);

	std::cout << a.getPtr() << "\n";
	std::cout << formatArray<Type>(start, 16) << '\n';

	std::cout << "Reallocate: " << t.reallocate(a, 8 * 8) << '\n';

	printMeta(t);

	std::cout << a.getPtr() << " (start+" << ((a.getCharPtr() - (char*)start) / 8) << ")\n";
	std::cout << formatArray<Type>(start, 16) << '\n';

	using TemplatedAlign = BitmappedBlock::Templated<VectorArray, 16, 8, 16>;
	std::cout << TemplatedAlign::Policy::getBlockSize() << ' ' <<
	             TemplatedAlign::Policy::getBlockCount() << ' ' <<
	             TemplatedAlign::Policy::alignment << '\n';

	TemplatedAlign tA;
	auto i = tA.allocate(1);
	std::cout << reinterpret_cast<uintptr_t>(i.getPtr()) % TemplatedAlign::Policy::alignment << '\n';*/

}