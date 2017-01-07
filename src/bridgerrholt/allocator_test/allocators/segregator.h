#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_SEGREGATOR_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_SEGREGATOR_H

#include "../common_types.h"
#include "../block.h"

namespace bridgerrholt {
	namespace allocator_test {
		namespace allocators {

template <SizeType threshold,
		      class    SmallAllocator,
		      class    LargeAllocator>
class Segregator : private SmallAllocator,
                   private LargeAllocator
{
		Segregator(SmallAllocator small, LargeAllocator large) :
			SmallAllocator(small),
			LargeAllocator(large) {}

		RawBlock allocate(SizeType size) {
			if (size <= threshold) {
				return SmallAllocator::allocate(size);
			}
			else {
				return LargeAllocator::allocate(size);
			}
		}

		void deallocate(RawBlock block) {
			if (block.getSize() <= threshold && SmallAllocator::owns(block))
				SmallAllocator::deallocate(block);

			else if (LargeAllocator::owns(block))
				LargeAllocator::deallocate(block);
		}

		bool owns(RawBlock block) {
			return (SmallAllocator::owns(block) || LargeAllocator::owns(block));
		}
};


		}
	}
}

#endif
