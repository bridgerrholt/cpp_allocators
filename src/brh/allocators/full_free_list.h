#ifndef BRH_CPP_ALLOCATORS_SRC_BRH_ALLOCATORS_FULL_FREE_LIST_H
#define BRH_CPP_ALLOCATORS_SRC_BRH_ALLOCATORS_FULL_FREE_LIST_H

#include <vector>
#include <type_traits>

#include <supports/calc_is_aligned.h>

#include "common/common_types.h"
#include "common/free_list_node.h"

#include "multithread/thread.h"

namespace brh {
	namespace allocators {
		namespace full_free_list {

static constexpr std::size_t minElementSize {
	sizeof(std::uintptr_t)
};

template <class Element>
class Iterator
{
	public:
		constexpr Iterator() {}
		constexpr Iterator(Element * ptr) : ptr_ {ptr} {}

		void advance() {
			ptr_ = ptr_->getNextNodePtr();
		}

		Element & get() { return *ptr_; }

	private:
		Element * ptr_;
};

template <class t_Policy>
class alignas(t_Policy::alignment)
Allocator : t_Policy
{
	public:
		using Policy = t_Policy;

	private:
		using ElementType = typename Policy::ElementType;

	public:
		friend void swap(Allocator & first, Allocator & second) {
			using std::swap;

			swap(static_cast<Policy&>(first), static_cast<Policy&>(second));
			swap(first.root_,                 second.root_);
		}

		constexpr Allocator() : Allocator(Policy()) {}

		Allocator(Policy policy) : Policy(std::move(policy)) {

			ElementType       *       currentPtr {this->getArray().data()};
			ElementType       *       nextPtr    {currentPtr + 1};
			ElementType const * const end        {this->getArray().data() + this->getBlockCount()};

			while (nextPtr < end) {
				currentPtr->setNextNode(nextPtr);

				currentPtr = nextPtr;
				++nextPtr;
			}

			currentPtr->setNextNode(nullptr);

			root_ = {this->getArray().data()};
		}

		Allocator(Allocator && other) : Allocator() {
			swap(*this, other);
		}

		constexpr SizeType calcRequiredSize(SizeType desiredSize) {
			return Policy::getBlockSize();
		}

		constexpr SizeType getStorageSize() const {
			return Policy::getArray().size();
		}

		/// Casting and dereferencing the returned pointer is
		/// implicitly undefined behaviour, but nothing unexpected should happen
		/// as long as you follow casting guidelines of
		/// the standard and your compiler.
		///
		/// @return Guaranteed to be aligned with the alignment of
		///         the @ref FullFreeList instantiation.
		void * allocate() {
			std::lock_guard<std::mutex> lock {mutex_};

			auto nextSpot = static_cast<void*>(&root_.get());

			// If root_ points to an unallocated spot,
			// it now must point to a new spot.
			if (nextSpot != nullptr) {
				//root_ = root_->node.getNextPtr();
				root_.advance();
			}

			return nextSpot;
		}

		void * allocateAligned(SizeType alignment) {
			auto nextSpot = static_cast<void*>(&root_.get());

			if (brh::supports::calcIsAligned(nextSpot, alignment))
				return allocate();

			else
				return nullptr;
		}

		constexpr void deallocate(NullBlock) const {}

		/// @param ptr Must be a pointer returned by
		///            this same instance's @ref allocate() method.
		void deallocate(void * ptr) {
			if (ptr == nullptr) return;

			auto blockPtr = static_cast<ElementType*>(ptr);

			std::lock_guard<std::mutex> lock {mutex_};

			// Overwrite the allocated block with a pointer to
			// the current next block to allocate.
			blockPtr->setNextNode(&root_.get());

			// The block being deallocated is now the next to be allocated.
			root_ = {blockPtr};
		}

		bool owns(void * ptr) {
			return (ptr >= this->getArray().data() &&
				ptr < this->getArray().data() + this->getBlockCount());
		}

	private:
		using IteratorType = Iterator<ElementType>;

		IteratorType root_;

#ifdef BRH_CPP_ALLOCATORS_MULTITHREADED
		std::mutex mutex_;
#endif
};

// Represents a block in the allocator's memory.
template <
	SizeType minimumBlockSize,
	std::size_t alignment>
union alignas(alignment) ArrayElement
{
	public:
		static constexpr SizeType getRequiredSize() {
			constexpr SizeType minimum {
				std::max(minimumBlockSize, minElementSize)
			};

			static_assert(minimum > 0, "Array size of ArrayElement can't be 0");

			SizeType remainder {minimum % alignment};
			SizeType toReturn  {minimum};

			if (remainder != 0)
				toReturn += (alignment - minimum % alignment);

			return toReturn;
		};

		void setNextNode(ArrayElement * nextNode) {
			nextNode_ = {nextNode};
		}

		ArrayElement * getNextNodePtr() {
			return nextNode_;
		}

		char * getCharPtr() {
			data_ = {};
			return data_;
		}


	private:
		ArrayElement * nextNode_;
		char           data_ [getRequiredSize()];
};


template <
	SizeType    minimumBlockSize,
	std::size_t minimumAlignment>
static constexpr std::size_t getAlignment() {
	using Element =
		ArrayElement<
			minimumBlockSize,
			std::max(minimumAlignment,
			         minElementSize)>;

	return std::max(minimumAlignment, alignof(Element));
}

template <template <class T> class CoreArray,
	SizeType    minimumBlockSize,
	std::size_t minimumAlignment>
class RuntimePolicy {
	public:
		static constexpr std::size_t alignment {
			getAlignment<minimumBlockSize, minimumAlignment>()
		};

		using ElementType = ArrayElement<minimumBlockSize, alignment>;
		using ArrayType   = CoreArray<ElementType>;

		static_assert(sizeof(ElementType) == ElementType::getRequiredSize(),
		              "ArrayElement's size is wrong");

		using ArrayReturn      = ArrayType       &;
		using ArrayConstReturn = ArrayType const &;

		RuntimePolicy(SizeType blockCount) : array_ (blockCount) {}

		SizeType calcRequiredSize(SizeType desiredSize) const {
			return getBlockSize();
		}

		ArrayReturn      getArray()       { return array_; }
		ArrayConstReturn getArray() const { return array_; }

		SizeType getBlockCount() { return array_.size(); }
		static constexpr SizeType getBlockSize()  { return sizeof(ElementType); }

	private:
		ArrayType array_;
};

template <template <class T> class CoreArray,
	SizeType    minimumBlockSize,
	std::size_t minimumAlignment = alignof(std::max_align_t)>
using Runtime = Allocator<RuntimePolicy<
	CoreArray, minimumBlockSize, minimumAlignment> >;


template <template <class T, SizeType size> class CoreArray,
	SizeType    minimumBlockSize,
	SizeType    blockCount,
	std::size_t minimumAlignment>
class TemplatedPolicy {
	public:
		static constexpr std::size_t alignment {
			getAlignment<minimumBlockSize, minimumAlignment>()
		};

		using ElementType = ArrayElement<minimumBlockSize, alignment>;
		using ArrayType   = CoreArray<ElementType, blockCount>;

		static_assert(sizeof(ElementType) == ElementType::getRequiredSize(),
		              "ArrayElement's size is wrong");

		using ArrayReturn      = ArrayType       &;
		using ArrayConstReturn = ArrayType const &;

		static constexpr SizeType calcRequiredSize(SizeType desiredSize) {
			return getBlockSize();
		}

		ArrayReturn      getArray()       { return array_; }
		ArrayConstReturn getArray() const { return array_; }

		static constexpr SizeType getBlockCount() { return blockCount; }
		static constexpr SizeType getBlockSize()  { return sizeof(ElementType); }

	private:
		ArrayType array_;
};

template <template <class, SizeType> class Array,
	SizeType    minimumBlockSize,
	SizeType    blockCount,
	std::size_t minimumAlignment = alignof(std::max_align_t)>
using Templated = Allocator<TemplatedPolicy<
		Array, minimumBlockSize, blockCount, minimumAlignment> >;

		} // full_free_list



class FullFreeList {
	public:
		template <class Policy>
		using Allocator = full_free_list::Allocator<Policy>;


		template <template <class T> class CoreArray,
			SizeType    minimumBlockSize,
			std::size_t minimumAlignment>
		using RuntimePolicy = full_free_list::RuntimePolicy<
			CoreArray, minimumBlockSize, minimumAlignment>;

		template <template <class T> class CoreArray,
			SizeType    minimumBlockSize,
			std::size_t minimumAlignment = alignof(std::max_align_t)>
		using Runtime = full_free_list::Runtime<
			CoreArray, minimumBlockSize, minimumAlignment>;


		template <template <class T, SizeType size> class CoreArray,
			SizeType    minimumBlockSize,
			SizeType    blockCount,
			std::size_t minimumAlignment>
		using TemplatedPolicy = full_free_list::TemplatedPolicy<
			CoreArray, minimumBlockSize, blockCount, minimumAlignment>;

		template <template <class, SizeType> class Array,
			SizeType    minimumBlockSize,
			SizeType    blockCount,
			std::size_t minimumAlignment = alignof(std::max_align_t)>
		using Templated = full_free_list::Templated<
			Array, minimumBlockSize, blockCount, minimumAlignment>;
};



	}
}

#endif
