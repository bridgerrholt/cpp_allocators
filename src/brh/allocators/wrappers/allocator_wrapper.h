#ifndef BRH_CPP_ALLOCATORS_SRC_BRH_ALLOCATORS_WRAPPERS_ALLOCATOR_WRAPPER_H
#define BRH_CPP_ALLOCATORS_SRC_BRH_ALLOCATORS_WRAPPERS_ALLOCATOR_WRAPPER_H

#include <utility>
#include <stdexcept>

#include "../blocks/block.h"

namespace brh {
	namespace allocators {

template <class Allocator>
class AllocatorWrapper : public Allocator
{
	public:
		template <class ... ArgTypes>
		AllocatorWrapper(ArgTypes ... args) : Allocator {std::forward(args)...} {}

		/// Constructs the template type using placement new and
		/// the passed arguments.
		/// Raw arrays are not supported (use std::array instead).
		template <class T, class ... ArgTypes>
		BasicBlock<T> construct(ArgTypes ... args) {
			RawBlock block = Allocator::allocate(sizeof(T));
			T * ptr = new (block.getPtr()) T {std::forward<ArgTypes>(args)...};
			return { ptr, block.getSize() };
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
				AllocatorType::template construct<T>(std::forward<ArgTypes>(args)...), sizeof(T)
			};
		}

		template <class T>
		void destruct(BasicBlock<T> block) {
			AllocatorType::template destruct(block.getPtr());
		}

		RawBlock allocate(SizeType size) {
			return {AllocatorType::allocate(), size};
		}

		void deallocate(RawBlock block) {
			AllocatorType::deallocate(block.getPtr());
		}

		bool owns(RawBlock block) {
			return AllocatorType::owns(block.getPtr());
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
