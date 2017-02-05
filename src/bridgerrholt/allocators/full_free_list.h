#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_FULL_FREE_LIST_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_FULL_FREE_LIST_H

#include <vector>
#include <type_traits>

#include "common/free_list_node.h"

#include "common/common_types.h"

namespace bridgerrholt {
	namespace allocators {

constexpr std::size_t minimumFreeListArrayElementSize {
	sizeof(std::uintptr_t)
};

// Represents a block in the allocator's memory.
template <
	SizeType minimumBlockSize,
	std::size_t alignment>
union alignas(alignment) FreeListArrayElement
{
	public:
		static constexpr SizeType getRequiredSize() {
			constexpr SizeType minimum {
				std::max(minimumBlockSize, minimumFreeListArrayElementSize)
			};

			static_assert(minimum > 0, "Array size of ArrayElement can't be 0");

			SizeType remainder {minimum % alignment};
			SizeType toReturn  {minimum};

			if (remainder != 0)
				toReturn += (alignment - minimum % alignment);

			return toReturn;
		};

		void setNextNode(FreeListArrayElement * nextNode) {
			nextNode_ = {nextNode};
		}

		FreeListArrayElement * getNextNodePtr() {
			return nextNode_;
		}

		char * getCharPtr() {
			data_ = {};
			return data_;
		}


	private:
		FreeListArrayElement * nextNode_;
		char                   data_ [getRequiredSize()];
};


template <class ArrayElement>
class FreeListIterator
{
	public:
		FreeListIterator() {}
		FreeListIterator(ArrayElement * ptr) : ptr_ {ptr} {}

		void advance() {
			ptr_ = ptr_->getNextNodePtr();
		}

		ArrayElement & get() { return *ptr_; }

	private:
		ArrayElement * ptr_;
};

template <
	SizeType    minimumBlockSize,
	std::size_t minimumAlignment>
constexpr
std::size_t
getFreeListAlignment() {
	return std::max(
		minimumAlignment,
		alignof(FreeListArrayElement<minimumBlockSize,
		                             std::max(minimumAlignment,
		                                      minimumFreeListArrayElementSize)>)
	);
}


/// This class itself does not have any undefined behaviour, working with
/// its returned values is however almost always undefined.
template <template <class, SizeType> class Array,
	SizeType    minimumBlockSize,
	SizeType    blockCount,
	std::size_t minimumAlignment = alignof(std::max_align_t)>
class alignas(getFreeListAlignment<minimumBlockSize, minimumAlignment>())
FullFreeList
{
	private:
		static constexpr std::size_t alignment {
			getFreeListAlignment<minimumBlockSize, minimumAlignment>()
		};


	public:
		using ElementType = FreeListArrayElement<minimumBlockSize, alignment>;
		using ArrayType   = Array<ElementType, blockCount>;

		static constexpr SizeType blockSize {sizeof(ElementType)};
		static_assert(sizeof(ElementType) == ElementType::getRequiredSize(),
		              "ArrayElement's size is wrong");

		friend void swap(FullFreeList & first, FullFreeList & second) {
			using std::swap;

			swap(first.array_, second.array_);
			swap(first.root_,  second.root_);
		}

		constexpr FullFreeList() : FullFreeList(ArrayType {}) { }

		FullFreeList(ArrayType array) :
			array_ {std::move(array)} {

			std::cout << "minimumBlockSize = " << minimumBlockSize << '\n'
			          << "blockCount       = " << blockCount << '\n'
			          << "minimumAlignment = " << minimumAlignment << '\n'
			          << "blockSize        = " << blockSize << '\n'
			          << "alignment        = " << alignment << '\n' << std::endl;

			ElementType       *       currentPtr {array_.data()};
			ElementType       *       nextPtr    {currentPtr + 1};
			ElementType const * const end        {array_.data() + blockCount};

			while (nextPtr < end) {
				currentPtr->setNextNode(nextPtr);

				currentPtr = nextPtr;
				++nextPtr;
			}

			currentPtr->setNextNode(nullptr);

			root_ = {array_.data()};
		}

		FullFreeList(FullFreeList && other) : FullFreeList() {
			swap(*this, other);
		}

		/// Casting and dereferencing the returned pointer is
		/// implicitly undefined behaviour, but nothing unexpected should happen
		/// as long as you follow casting guidelines of
		/// the standard and your compiler.
		///
		/// @return Guaranteed to be aligned with the alignment of
		///         the @ref FullFreeList instantiation.
		void * allocate() {
			//std::cout << "FullFreeList::allocate()\n";

			auto nextSpot = static_cast<void*>(&root_.get());

			// If root_ points to an unallocated spot,
			// it now must point to a new spot.
			if (nextSpot != nullptr) {
				//root_ = root_->node.getNextPtr();
				root_.advance();
			}

			return nextSpot;
		}

		/// @param ptr Must be a pointer returned by
		///            this same instance's @ref allocate() method.
		void deallocate(void * ptr) {
			auto blockPtr = static_cast<ElementType*>(ptr);

			// Overwrite the allocated block with a pointer to
			// the current next block to allocate.
			blockPtr->setNextNode(&root_.get());

			// The block being deallocated is now the next to be allocated.
			root_ = {blockPtr};
		}

		bool owns(void * ptr) {
			return (ptr >= array_.data() && ptr < array_.data() + blockCount);
		}

	private:
		using IteratorType = FreeListIterator<ElementType>;

		ArrayType    array_;
		IteratorType root_;
};


	}
}

#endif
