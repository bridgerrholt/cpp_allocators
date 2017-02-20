#include <array>
#include <vector>
#include <random>
#include <type_traits>
#include <iostream>

#include "../performance_test_0/get_time.h"

#include <allocators/stack_allocator.h>
#include <allocators/bitmapped_block.h>
#include <allocators/affix_allocator.h>
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

	using Type = int;
	using CoreAllocator = StackAllocator::Templated<VectorArray, 256>;
	using Affix = VarWrapper<int, 123>;
	using AllocatorBase = AffixChecker<CoreAllocator, Affix>;
	using Allocator = AllocatorWrapper<AllocatorBase>;

	Allocator allocator;

	auto a = allocator.construct<Type>(INT_MIN);
	auto b = allocator.construct<Type>(INT_MIN);
	auto c = allocator.construct<Type>(INT_MIN);

	allocator.deallocate(c);
	allocator.deallocate(b);
	allocator.deallocate(a);

}