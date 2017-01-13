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

			common::FreeListNode node;
			char                 data [getRequiredSize()];
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
			ElementType * ptr         {previousPtr + blockSize};
			ElementType * end         {array_.data() + blockCount};
			//std::cout << (void*)array_.data() << ":" << (void*)end << '\n';

			while (ptr < end) {
				//std::cout << (void*)ptr << " " << (void*)previousPtr << std::endl;
				//reinterpret_cast<common::FreeListNode*>(previousPtr)->setNextPtr(
				//	reinterpret_cast<common::FreeListNode*>(ptr)
				//);
				ptr->node = {};
				common::FreeListNodeView node {&ptr->node};

				/**reinterpret_cast<std::uintptr_t*>(previousPtr) =
					reinterpret_cast<std::uintptr_t>(
						node.getNodePtr()
					);*/

				previousPtr->node = {node.getNodePtr()};

				previousPtr = ptr;
				ptr += blockSize;
			}

			//std::cout << (void*)previousPtr << '\n';
			common::FreeListNodeView node {previousPtr};
			node.setNextPtr(nullptr);

			root_ = {static_cast<void*>(array_.data())};
			//std::cout << "FullFreeList() end\n";
		}

		/// @return Guaranteed to be aligned with the alignment of
		/// the FullFreeList instantiation.
		void * allocate() {
			auto toReturn = static_cast<void*>(root_.getNodePtr());

			// If root_ points to an unallocated spot,
			// it now must point to a new spot.
			if (toReturn != nullptr) {
				root_ = root_->node.getNextPtr();
			}

			return toReturn;
		}

		/// @param ptr Must be a pointer returned by
		///            this same instance's @ref allocate() method.
		void deallocate(void * ptr) {
			auto blockPtr = static_cast<common::FreeListNode*>(ptr);

			root_->node = {};
			common::FreeListNodeView nodeView {&root_->node};

			// Overwrite the allocated block with a pointer to
			// the current next block to allocate.
			blockPtr->setNextPtr(nodeView.getNodePtr());

			// The block being deallocated is now the next to be allocated.
			root_ = static_cast<ElementType*>(ptr);
		}


	private:
		using ElementType = ArrayElement;
		using ArrayType   = Array<ElementType, blockCount>;

		ArrayType     array_;
		ElementType * root_;
};


		}
	}
}

#endif
