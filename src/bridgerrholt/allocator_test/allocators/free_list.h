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
		using ByteType  = unsigned char;

		FreeList() {

		}

		FreeList(A parent) : parent_ {parent} {

		}

		bool isEmpty() {
			return !root_.hasNext();
		}


		RawBlock allocate(SizeType size) {
			if (size == blockSize && root_.hasNext()) {
				RawBlock block { root_.getNext(), size};
				root_.setNext(root_.getNext());
				return block;
			}
			else
				return parent_.allocate(size);
		}

		void deallocate(RawBlock block) {
			auto ptr = static_cast<Pointer>(block.getPtr());

			// The amount of blocks it takes up.
			std::size_t blocks     {block.getSize() / blockSize};
			std::size_t blockIndex {getBlockIndex(ptr)};
			std::size_t objectEnd  {blockIndex + blocks};

			while (blockIndex < objectEnd) {
				unsetMetaBit(blockIndex);

				++blockIndex;
			}
		}

		bool owns(RawBlock block) {
			auto ptr = static_cast<Pointer>(block.getPtr());

			return (
				ptr >= array_.data() + metaDataSize() &&
			  ptr <  array_.data() + totalSize()
			);
		}


	private:
		using Pointer = ByteType *;

		SizeType getBlockIndex(Pointer blockPtr) {
			return (blockPtr - array_.data() - metaDataSize()) / blockSize;
		}

		SizeType getMetaIndex(Pointer blockPtr) {
			return getMetaIndex(getBlockIndex(blockPtr));
		}

		SizeType getMetaBitIndex(Pointer blockPtr) {
			return getMetaIndex(getBlockIndex(blockPtr));
		}


		constexpr static SizeType getBlockIndex(
			SizeType metaIndex, SizeType metaBitIndex) {
			return (CHAR_BIT * metaIndex) + metaBitIndex;
		}

		constexpr static SizeType getMetaIndex(SizeType blockIndex) {
			return blockIndex / CHAR_BIT;
		}

		constexpr static SizeType getMetaBitIndex(SizeType blockIndex) {
			return blockIndex % CHAR_BIT;
		}


		Pointer getMetaPtr(Pointer blockPtr) {
			return array_[getMetaBitIndex(blockPtr)];
		}


		int getMetaBit(Pointer blockPtr) {
			return getMetaBit(getMetaIndex(blockPtr), getMetaBitIndex(blockPtr));
		}

		int getMetaBit(SizeType blockIndex) {
			return getMetaBit(getMetaIndex(blockIndex), getMetaBitIndex(blockIndex));
		}

		int getMetaBit(SizeType metaIndex, SizeType metaBitIndex) {
			ByteType & metaPtr {array_[metaIndex]};
			int temp = (metaPtr >> metaBitIndex) & 1;

			//std::cout << "(" << metaIndex << ", " << metaBitIndex << ") = " << temp << '\n';

			return temp;
		}


		void setMetaBit(Pointer blockPtr) {
			setMetaBit(getMetaIndex(blockPtr), getMetaBitIndex(blockPtr));
		}

		void setMetaBit(SizeType blockIndex) {
			setMetaBit(getMetaIndex(blockIndex), getMetaBitIndex(blockIndex));
		}

		void setMetaBit(SizeType metaIndex, SizeType metaBitIndex) {
			ByteType & metaRef {array_[metaIndex]};
			metaRef |= 1 << metaBitIndex;

			//std::cout << "set (" << metaIndex << ", " << metaBitIndex << ")\n";
		}


		void unsetMetaBit(Pointer blockPtr) {
			unsetMetaBit(getMetaIndex(blockPtr), getMetaBitIndex(blockPtr));
		}

		void unsetMetaBit(SizeType blockIndex) {
			unsetMetaBit(getMetaIndex(blockIndex), getMetaBitIndex(blockIndex));

			//std::cout << "Unset block " << blockIndex << '\n';
		}

		void unsetMetaBit(SizeType metaIndex, SizeType metaBitIndex) {
			ByteType & metaRef {array_[metaIndex]};
			metaRef &= ~(1 << metaBitIndex);
		}


		constexpr static double efficiency() {
			return static_cast<double>(blockCount) / (metaDataSize() * CHAR_BIT);
		}

		A    parent_;
		Node root_;

		class Node {
			public:
				Node() : next_ {nullptr} {}

				Node * getNext() const { return next_; }
				void setNext(Node * next) { next_ = next; }

				bool hasNext() const { return next_ != nullptr; }

			private:
				Node * next_;
		};
};


template <
	template <class T, SizeType size> class Allocator,
	SizeType blockCoefficient,
	SizeType blockCountCoefficient,
	SizeType alignment = alignof(std::max_align_t)>
using MemoryEfficientBitmappedBlock =
BitmappedBlock<
	Allocator,
	blockCoefficient * alignment,
  blockCountCoefficient * alignment * CHAR_BIT,
  alignment>;


		}
	}
}

#endif
