#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_SEGREGATED_STORAGE_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_SEGREGATED_STORAGE_H

#include <vector>

#include "common/free_list_node.h"

#include "../common_types.h"

namespace bridgerrholt {
	namespace allocator_test {
		namespace allocators {

template <template <class, SizeType> class Array,
	SizeType blockSizeT,
	SizeType blockCount,
	SizeType alignment = alignof(std::max_align_t)>
class alignas(alignment) FullFreeList
{
	private:
		static constexpr SizeType totalSize() { return blockSize * blockCount; }

	public:
		static constexpr SizeType blockSize {blockSizeT};

		static_assert(blockSize >= sizeof(common::FreeListNode),
		              "Blocks must be large enough for node data");

		static_assert(blockSize % alignment == 0,
		              "Block size must be divisible by the alignment");

		using ByteType = char;
		using ArrayType = Array<ByteType, totalSize()>;


		FullFreeList() {
			ByteType * previousPtr {array_.data()};
			ByteType * ptr         {previousPtr + blockSize};
			ByteType * end         {array_.data() + totalSize()};
			//std::cout << (void*)array_.data() << ":" << (void*)end << '\n';

			while (ptr < end) {
				//std::cout << (void*)ptr << " " << (void*)previousPtr << std::endl;
				reinterpret_cast<common::FreeListNode*>(previousPtr)->setNextPtr(
					reinterpret_cast<common::FreeListNode*>(ptr)
				);

				previousPtr = ptr;
				ptr += blockSize;
			}

			//std::cout << (void*)previousPtr << '\n';
			reinterpret_cast<common::FreeListNode*>(previousPtr)->setNextPtr(nullptr);

			root_ = {reinterpret_cast<common::FreeListNode*>(array_.data())};
			//std::cout << "FullFreeList() end\n";
		}

		void * allocate() {
			void * toReturn {root_.getNextPtr()};

			if (toReturn != nullptr)
				root_.advance();

			return toReturn;
		}

		void deallocate(void * ptr) {
			auto * blockPtr = static_cast<common::FreeListNode *>(ptr);
			blockPtr->setNextPtr(root_.getNextPtr());
			root_.setNextPtr(blockPtr);
		}


	private:
		ArrayType            array_;
		common::FreeListNode root_;
};

		}
	}
}

#endif
