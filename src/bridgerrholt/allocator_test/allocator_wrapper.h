#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_CONSTRUCT_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_CONSTRUCT_H

#include <utility>

#include "block.h"

namespace bridgerrholt {
	namespace allocator_test {

template <class Allocator>
class AllocatorWrapper : public Allocator
{
	public:
		template <class ... ArgTypes>
		AllocatorWrapper(ArgTypes ... args) : Allocator {std::forward(args)...} {}

		template <class T, class ... ArgTypes>
		BasicBlock<T> construct(ArgTypes ... args) {
			RawBlock block = Allocator::allocate(sizeof(T));
			T * ptr = new (block.getPtr()) T {std::forward<ArgTypes>(args)...};
			return { block.getPtr(), block.getSize() };
		}

		template <class T>
		void destruct(BasicBlock<T> block) {
			block.getPtr()->~T();
			Allocator::deallocate(block);
		}
};



template <class Allocator>
class BlockAllocatorWrapper : public Allocator
{
	public:
		template <class ... ArgTypes>
		BlockAllocatorWrapper(ArgTypes ... args) : Allocator {std::forward(args)...} {}

		template <class T, class ... ArgTypes>
		T * construct(ArgTypes ... args) {
			static_assert(sizeof(T) <= Allocator::blockSize,
			              "Type size too large for allocator");

			T * ptr = reinterpret_cast<T*>(Allocator::allocate());
			ptr = new (ptr) T {std::forward<ArgTypes>(args)...};
			return ptr;
		}

		template <class T>
		void destruct(T * ptr) {
			ptr->~T();
			Allocator::deallocate(reinterpret_cast<void *>(ptr));
		}
};


	}
}

#endif
