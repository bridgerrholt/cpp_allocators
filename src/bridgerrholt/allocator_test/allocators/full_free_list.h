#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_SEGREGATED_STORAGE_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_SEGREGATED_STORAGE_H

#include <vector>
#include <type_traits>

#include "common/free_list_node.h"

#include "../common_types.h"

namespace bridgerrholt {
	namespace allocator_test {
		namespace allocators {

constexpr
std::size_t getFreeListAlignment(std::size_t minimumAlignment) {
	return std::max(minimumAlignment, alignof(common::FreeListNode));
}


template <template <class, SizeType> class Array,
	SizeType minimumBlockSize,
	SizeType blockCount,
	std::size_t minimumAlignment = alignof(std::max_align_t)>
class alignas(getFreeListAlignment(minimumAlignment)) FullFreeList
{
	private:
		static constexpr std::size_t alignment {
			getFreeListAlignment(minimumAlignment)
		};

		union alignas(alignment) ArrayElement
		{
			public:
				static constexpr SizeType getRequiredSize() {
					constexpr SizeType minimum {
						std::max(minimumBlockSize, sizeof(common::FreeListNode))
					};

					static_assert(minimum != 0, "Array size of ArrayElement can't be 0");

					SizeType remainder {minimum % alignment};
					SizeType toReturn  {minimum};

					if (remainder != 0)
						toReturn += (alignment - minimum % alignment);

					return toReturn;
				};

				void setNextNode(ArrayElement * nextNode) {
					node_ = {nextNode};
				}

				ArrayElement * getNextNode() {
					return node_->getNodePtr();
				}

				ArrayElement * getNodePtr() {
					return node_;
				}

				char * getCharPtr() {
					data_ = {};
					return data_;
				}


			private:
				ArrayElement * node_;
				char           data_ [getRequiredSize()];
		};


		class Iterator
		{
			public:
				Iterator() {}
				Iterator(ArrayElement * ptr) : ptr_ {ptr} {}

				void advance() {
					ptr_ = ptr_->getNodePtr();
				}

				ArrayElement & get() { return *ptr_; }

			private:
				ArrayElement * ptr_;
		};

		static_assert(sizeof(ArrayElement) == ArrayElement::getRequiredSize(),
		              "ArrayElement's size is wrong");


	public:
		static constexpr SizeType blockSize {sizeof(ArrayElement)};

		FullFreeList() {
			std::cout << "minimumBlockSize = " << minimumBlockSize << '\n'
		            << "blockCount       = " << blockCount << '\n'
		            << "minimumAlignment = " << minimumAlignment << '\n'
		            << "blockSize        = " << blockSize << '\n'
			          << "alignment        = " << alignment << '\n' << std::endl;

			ElementType * previousPtr {array_.data()};
			ElementType * ptr         {previousPtr + 1};
			ElementType * end         {array_.data() + blockCount};
			//std::cout << (void*)array_.data() << ":" << (void*)end << '\n';

			while (ptr < end) {
				//std::cout << (void*)ptr << " " << (void*)previousPtr << std::endl;
				//reinterpret_cast<common::FreeListNode*>(previousPtr)->setNextPtr(
				//	reinterpret_cast<common::FreeListNode*>(ptr)
				//);

				previousPtr->setNextNode(ptr);

				/**reinterpret_cast<std::uintptr_t*>(previousPtr) =
					reinterpret_cast<std::uintptr_t>(
						node.getNodePtr()
					);*/

				//previousPtr->node = {node.getNodePtr()};

				previousPtr = ptr;
				++ptr;
			}

			//std::cout << (void*)previousPtr << '\n';

			previousPtr->setNextNode(nullptr);

			root_ = {array_.data()};
			//std::cout << "FullFreeList() end\n";
		}

		/// @return Guaranteed to be aligned with the alignment of
		/// the FullFreeList instantiation.
		void * allocate() {
			auto toReturn = static_cast<void*>(&root_.get());

			// If root_ points to an unallocated spot,
			// it now must point to a new spot.
			if (toReturn != nullptr) {
				//root_ = root_->node.getNextPtr();
				root_.advance();
			}

			return toReturn;
		}

		/// @param ptr Must be a pointer returned by
		///            this same instance's @ref allocate() method.
		void deallocate(void * ptr) {
			auto blockPtr = static_cast<ArrayElement*>(ptr);

			//root_->node = {};
			//common::FreeListNodeView nodeView {&root_->node};

			// Overwrite the allocated block with a pointer to
			// the current next block to allocate.
			blockPtr->setNextNode(&root_.get());

			// The block being deallocated is now the next to be allocated.
			root_ = {static_cast<ElementType*>(ptr)};
		}


	private:
		using ElementType = ArrayElement;
		using ArrayType   = Array<ElementType, blockCount>;

		ArrayType array_;
		Iterator  root_;
};


		}
	}
}

#endif
