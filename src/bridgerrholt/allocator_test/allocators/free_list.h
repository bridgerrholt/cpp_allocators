#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_FREE_LIST_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_FREE_LIST_H

#include <vector>
#include <bitset>
#include <cstddef>

#include "common/free_list_node.h"

#include "../common_types.h"
#include "../block.h"

namespace bridgerrholt {
	namespace allocator_test {
		namespace allocators {

template <class Allocator, SizeType blockSize>
class FreeList
{
	public:
		static_assert(blockSize >= sizeof(std::uintptr_t),
		              "Must be able to fit whole pointers");

		using ByteType  = unsigned char;

		FreeList() {

		}

		FreeList(Allocator parent) : parent_ {parent} {

		}

		bool isEmpty() {
			return !root_.hasNode();
		}


		RawBlock allocate(SizeType size) {
			if (isCorrectSize(size) && !isEmpty()) {
				//std::cout << "FreeList::allocate()\n";
				RawBlock block {root_.getNodePtr(), size};
				root_.advance();
				return block;
			}
			else {
				//std::cout << "parent_.allocate()\n";
				return parent_.allocate(size);
			}
		}

		void deallocate(RawBlock block) {
			if (isCorrectSize(block)) {
				common::FreeListNodeView node {block.getPtr()};
				node.setNextPtr(root_.getNextPtr());
				root_.setNextPtr(node.getNodePtr());

				//std::cout << "FreeList::deallocate()\n";
			}
			else {
				//std::cout << "parent_.deallocate()\n";
				parent_.deallocate(block);
			}
		}

		bool owns(RawBlock block) {
			return (isCorrectSize(block) || parent_.owns(block));
		}


	private:
		using Pointer = ByteType *;

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
}

#endif
