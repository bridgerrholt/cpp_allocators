#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_FREE_LIST_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_FREE_LIST_H

#include <vector>
#include <bitset>
#include <cstddef>

#include "../common_types.h"
#include "../block.h"

namespace bridgerrholt {
	namespace allocator_test {
		namespace allocators {

template <class A, SizeType blockSize>
class FreeList
{
	public:
		static_assert(blockSize >= sizeof(std::uintptr_t),
		              "Must be able to fit whole pointers");

		using ByteType  = unsigned char;

		FreeList() {

		}

		FreeList(A parent) : parent_ {parent} {

		}

		bool isEmpty() {
			return !root_.hasNext();
		}


		RawBlock allocate(SizeType size) {
			if (isCorrectSize(size) && root_.hasNext()) {
				//std::cout << "FreeList::allocate()\n";
				RawBlock block {static_cast<void *>(root_.getNextPtr()), size};
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
				auto node = static_cast<Node*>(block.getPtr());
				node->setNextPtr(root_.getNextPtr());
				root_.setNextPtr(node);

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

		class Node {
			public:
				Node() : nextPtr_ {nullptr} {}

				Node * getNextPtr() const { return nextPtr_; }
				void setNextPtr(Node * next) { nextPtr_ = next; }

				Node & getNext() const { return *nextPtr_; }

				void advance() {
					setNextPtr(getNext().getNextPtr());
				}

				bool hasNext() const { return nextPtr_ != nullptr; }

			private:
				Node * nextPtr_;
		};

		A    parent_;
		Node root_;
};


		}
	}
}

#endif
