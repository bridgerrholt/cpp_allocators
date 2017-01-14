#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_CONSTRUCT_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_CONSTRUCT_H

#include <utility>
#include <stdexcept>

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
			Allocator::deallocate(static_cast<void *>(ptr));
		}
};


template <class Allocator>
class BlockAllocatorRegularInterface :
	private BlockAllocatorWrapper<Allocator>
{
	public:
		using AllocatorType = BlockAllocatorWrapper<Allocator>;

		template <class ... ArgTypes>
		BlockAllocatorRegularInterface(ArgTypes ... args) :
			AllocatorType {std::forward(args)...} {}

		template <class T, class ... ArgTypes>
		BasicBlock<T> construct(ArgTypes ... args) {
			return {
				AllocatorType::template construct<T>(std::forward<ArgTypes>(args)...), 1
			};
		}

		template <class T>
		void destruct(BasicBlock<T> block) {
			AllocatorType::template destruct(block.getPtr());
		}
};


template <class Allocator>
class SimpleBlockAllocatorRegularInterface :
	public Allocator
{
	public:
		RawBlock allocate(SizeType size) {
			if (size > Allocator::blockSize)
				return {nullptr, 0};

			return {Allocator::allocate(), size};
		}


		void deallocate(RawBlock block) {
			Allocator::deallocate(block.getPtr());
		}
};


	}
}

#endif
