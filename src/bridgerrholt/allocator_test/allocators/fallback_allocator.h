#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_ALLOCATOR_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_ALLOCATOR_H

#include "../common_types.h"

#include "../block.h"

namespace bridgerrholt {
	namespace allocator_test {

template <class Primary, class Fallback>
class FallbackAllocator : private Primary,
                          private Fallback
{
	public:
		FallbackAllocator() {}

		Block allocate(SizeType size) {
			Block toReturn {Primary::allocate(size)};

			if (toReturn.isNull())
				toReturn = Fallback::allocate(size);

			return toReturn;
		}

		void deallocate(Block block) {
			if (Primary::owns(block))
				Primary::deallocate(block);

			else
				Fallback::deallocate(block);
		}

		bool owns(Block block) {
			return (Primary::owns(block) || Fallback::owns(block));
		}
};



	}
}

#endif
