#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_FREE_LIST_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_FREE_LIST_H

#include <vector>
#include <bitset>
#include <cstddef>

#include "common/free_list_node.h"
#include "common/common_types.h"

#include "blocks/block.h"

namespace bridgerrholt {
	namespace allocators {

template <class Allocator, SizeType blockSize>
class InternalFreeList
{
	public:
		static_assert(blockSize >= sizeof(std::uintptr_t),
		              "Must be able to fit whole pointers");


		friend void swap(InternalFreeList & first, InternalFreeList & second) {
			using std::swap;

			swap(first.parent_, second.parent_);
			swap(first.root_,   second.root_);
		}

		InternalFreeList() {}

		InternalFreeList(Allocator parent) : parent_ (std::move(parent)) {}

		InternalFreeList(InternalFreeList && other) : InternalFreeList() {
			swap(*this, other);
		}

		bool isEmpty() { return !root_.hasNode(); }


		RawBlock allocate(SizeType size) {
			if (isCorrectSize(size) && !isEmpty()) {
				RawBlock block {root_.getNodePtr(), size};
				root_.advance();
				return block;
			}
			else {
				return parent_.allocate(size);
			}
		}

		constexpr void deallocate(NullBlock) {}

		void deallocate(RawBlock block) {
			if (block.isNull()) return;

			if (isCorrectSize(block)) {
				common::FreeListNodeView node {block.getPtr()};
				node.setNextPtr(root_.getNextPtr());
				root_.setNextPtr(node.getNodePtr());
			}
			else {
				parent_.deallocate(block);
			}
		}

		bool owns(RawBlock block) {
			return (isCorrectSize(block) || parent_.owns(block));
		}


	private:
		bool isCorrectSize(RawBlock block) {
			return isCorrectSize(block.getSize());
		}

		bool isCorrectSize(SizeType size) {
			return (size == blockSize);
		}

		Allocator                parent_;
		common::FreeListNodeView root_;
};


	}
}

#endif
