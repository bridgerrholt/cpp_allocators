#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_SEGREGATOR_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_SEGREGATOR_H

#include "common/common_types.h"
#include "blocks/block.h"

namespace bridgerrholt {
	namespace allocators {

template <SizeType threshold,
		      class    SmallAllocator,
		      class    LargeAllocator>
class Segregator : private SmallAllocator,
                   private LargeAllocator
{
	public:
		Segregator() {}

		Segregator(SmallAllocator small, LargeAllocator large) :
			SmallAllocator(small),
			LargeAllocator(large) {}

		RawBlock allocate(SizeType size) {
			if (size <= threshold) {
				/*static std::size_t smallCount = 0;
				smallCount++;
				std::cout << "Small: " << smallCount << '\n';*/
				return SmallAllocator::allocate(size);
			}
			else {
				/*static std::size_t largeCount = 0;
				largeCount++;
				std::cout << "Large: " << largeCount << '\n';*/
				return LargeAllocator::allocate(size);
			}
		}

		void deallocate(RawBlock block) {
			if (block.getSize() <= threshold)
				SmallAllocator::deallocate(block);

			else
				LargeAllocator::deallocate(block);
		}

		bool owns(RawBlock block) {
			return (SmallAllocator::owns(block) || LargeAllocator::owns(block));
		}
};


template <SizeType maxSize, class Allocator>
class MaximumSizeAllocator : private Allocator
{
	public:
		MaximumSizeAllocator() {}

		MaximumSizeAllocator(Allocator allocator) : Allocator {allocator} {}

		RawBlock allocate(SizeType size) {
			if (size > maxSize)
				return {nullptr, 0};
			else
				return Allocator::allocate(size);
		}

		void deallocate(RawBlock block) {
			if (block.getSize() <= maxSize)
				Allocator::deallocate(block);
		}

		bool owns(RawBlock block) {
			return (Allocator::owns(block));
		}
};


	}
}

#endif
