#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_BLOCKS_UNIQUE_BLOCK_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_BLOCKS_UNIQUE_BLOCK_H

#include <type_traits>

#include "../common/common_types.h"
#include "block.h"

namespace bridgerrholt {
	namespace allocators {

template <class T, class Allocator>
class UniqueBlock
{
	public:
		using Type         = T;
		using Pointer      = Type       *;
		using ConstPointer = Type const *;
		using BlockType    = BasicBlock<T>;

		friend void swap(UniqueBlock & first, UniqueBlock & second) {
			using std::swap;

			swap(first.block_,     second.block_);
			swap(first.allocator_, second.allocator_);
		}

		UniqueBlock(void * ptr, SizeType size, Allocator & allocator) :
			block_     {ptr, size},
			allocator_ {&allocator} { }

		UniqueBlock(Pointer ptr, SizeType size, Allocator & allocator) :
			UniqueBlock {static_cast<void *>(ptr), size, allocator} {}

		template <class U>
		UniqueBlock(BasicBlock<U> block, Allocator & allocator) :
			block_     (block),
			allocator_ {&allocator} {}

		~UniqueBlock() {
			if (!block_.isNull())
				allocator_->destruct(block_);
		}

		UniqueBlock(UniqueBlock const &) = delete;
		UniqueBlock(UniqueBlock && other) : UniqueBlock() {
			swap(*this, other);
		}

		UniqueBlock & operator=(UniqueBlock) = delete;
		UniqueBlock & operator=(UniqueBlock && other) {
			swap(*this, other);
		}


	private:
		UniqueBlock() : allocator_ {nullptr} {}

		BlockType   block_;
		Allocator * allocator_;
};

	}
}

#endif
