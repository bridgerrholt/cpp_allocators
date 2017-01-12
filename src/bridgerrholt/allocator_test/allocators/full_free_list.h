#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_SEGREGATED_STORAGE_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_SEGREGATED_STORAGE_H

#include <vector>
#include <type_traits>

#include "common/free_list_node.h"

#include "../common_types.h"

namespace bridgerrholt {
	namespace allocator_test {
		namespace allocators {

template <template <class, SizeType> class Array,
	SizeType blockSizeT,
	SizeType blockCount,
	class AlignmentType = unsigned long long>
class alignas(alignof(AlignmentType)) FullFreeList
{
	private:
		static constexpr SizeType totalSize() { return blockSize * blockCount; }

	public:
		static constexpr SizeType blockSize {blockSizeT};

		static_assert(blockSize >= sizeof(common::FreeListNode),
		              "Blocks must be large enough for node data");

		static_assert(blockSize % alignof(AlignmentType) == 0,
		              "Block size must be divisible by the alignment");

		using ElementType = char;
		using ArrayType   = Array<ElementType, totalSize()>;


		FullFreeList() {
			void * previousPtr {array_.data()};
			void * ptr         {previousPtr + blockSize};
			void * end         {array_.data() + totalSize()};
			//std::cout << (void*)array_.data() << ":" << (void*)end << '\n';

			while (ptr < end) {
				//std::cout << (void*)ptr << " " << (void*)previousPtr << std::endl;
				//reinterpret_cast<common::FreeListNode*>(previousPtr)->setNextPtr(
				//	reinterpret_cast<common::FreeListNode*>(ptr)
				//);

				common::FreeListNodeView node {ptr};

				*reinterpret_cast<std::uintptr_t*>(previousPtr) =
					reinterpret_cast<std::uintptr_t>(
						node.getNodePtr()
					);

				previousPtr = ptr;
				ptr += blockSize;
			}

			//std::cout << (void*)previousPtr << '\n';
			common::FreeListNodeView node {previousPtr};
			node.setNextPtr(nullptr);

			root_ = {static_cast<void*>(array_.data())};
			//std::cout << "FullFreeList() end\n";
		}

		void * allocate() {
			auto toReturn = static_cast<void*>(root_.getNextPtr());

			if (toReturn != nullptr)
				root_.advance();

			return toReturn;
		}

		void deallocate(void * ptr) {
			auto blockPtr = static_cast<common::FreeListNode *>(ptr);
			blockPtr->setNextPtr(root_.getNextPtr());
			root_.setNextPtr(blockPtr);
		}


	private:
		ArrayType                array_;
		common::FreeListNodeView root_;
};

		}
	}
}

#endif
