#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_FULL_FREE_LIST_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_FULL_FREE_LIST_H

#include <vector>
#include <type_traits>

#include "common/free_list_node.h"

#include "common/common_types.h"

namespace bridgerrholt {
	namespace allocators {

template <class> class BasicFullFreeList;

class FullFreeList
{
	private:
		static constexpr std::size_t minElementSize {
			sizeof(std::uintptr_t)
		};

	public:
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
			std::size_t t_alignment>
		class RuntimePolicy {
			public:
				static constexpr std::size_t alignment {t_alignment};

				using ElementType = ArrayElement<minimumBlockSize, alignment>;
				using ArrayType   = CoreArray<ElementType>;

				static_assert(sizeof(ElementType) == ElementType::getRequiredSize(),
				              "ArrayElement's size is wrong");

				using ArrayReturn      = ArrayType       &;
				using ArrayConstReturn = ArrayType const &;

				RuntimePolicy(SizeType blockCount) : array_ (blockCount) {}

				ArrayReturn      getArray()       { return array_; }
				ArrayConstReturn getArray() const { return array_; }

				SizeType getBlockCount() { return array_.size(); }
				static constexpr SizeType getBlockSize()  { return sizeof(ElementType); }

			private:
				ArrayType array_;
		};

		template <template <class T, SizeType size> class CoreArray,
			SizeType    minimumBlockSize,
			SizeType    blockCount,
			std::size_t t_alignment>
		class TemplatedPolicy {
			public:
				static constexpr std::size_t alignment {t_alignment};

				using ElementType = ArrayElement<minimumBlockSize, alignment>;
				using ArrayType   = CoreArray<ElementType, blockCount>;

				static_assert(sizeof(ElementType) == ElementType::getRequiredSize(),
				              "ArrayElement's size is wrong");

				using ArrayReturn      = ArrayType       &;
				using ArrayConstReturn = ArrayType const &;

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
		using Templated = BasicFullFreeList<TemplatedPolicy<
			Array, minimumBlockSize, blockCount,
			getAlignment<minimumBlockSize, minimumAlignment>()
		>>;
};


/// This class itself does not have any undefined behaviour, working with
/// its returned values is however almost always undefined.
template <class t_Policy>
class alignas(t_Policy::alignment)
BasicFullFreeList : t_Policy
{
	public:
		using Policy = t_Policy;

	private:
		using ElementType = typename Policy::ElementType;

	public:
		friend void swap(BasicFullFreeList & first, BasicFullFreeList & second) {
			using std::swap;

			swap(static_cast<Policy&>(first), static_cast<Policy&>(second));
			swap(first.root_,                 second.root_);
		}

		constexpr BasicFullFreeList() : BasicFullFreeList(Policy()) {}

		BasicFullFreeList(Policy policy) : Policy(std::move(policy)) {

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

		BasicFullFreeList(BasicFullFreeList && other) : BasicFullFreeList() {
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

		constexpr void deallocate(NullBlock) {}

		/// @param ptr Must be a pointer returned by
		///            this same instance's @ref allocate() method.
		void deallocate(void * ptr) {
			if (ptr == nullptr) return;

			auto blockPtr = static_cast<ElementType*>(ptr);

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
		using IteratorType = FullFreeList::Iterator<ElementType>;

		IteratorType root_;
};



	}
}

#endif
