#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_CONSTRUCT_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_CONSTRUCT_H

#include <utility>

#include "block.h"

namespace bridgerrholt {
	namespace allocator_test {

template <class T>
class BlockWrapper : public Block
{
	public:
		using Pointer      = T       *;
		using ConstPointer = T const *;

		BlockWrapper(GenericPtr ptr, SizeType size) : Block {ptr, size} {}

		ConstPointer getPtr() const {
			return reinterpret_cast<ConstPointer>(this->Block::getPtr());
		}

		Pointer getPtr() { return reinterpret_cast<Pointer>(Block::getPtr()); }

};



template <class Allocator>
class AllocatorWrapper : private Allocator
{
	public:
		template <class ... ArgTypes>
		AllocatorWrapper(ArgTypes ... args) : Allocator {std::forward(args)...} {}

		template <class T, class ... ArgTypes>
		BlockWrapper<T> construct(ArgTypes ... args) {
			Block block = Allocator::allocate(sizeof(T));
			T * ptr = new (block.getPtr()) T { std::forward<ArgTypes>(args)... };
			return { block.getPtr(), block.getSize() };
		}

		template <class T>
		void destruct(BlockWrapper<T> block) {
			block.getPtr()->~T();
			Allocator::deallocate(block);
		}
};





	}
}

#endif
