#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_MALLOC_ALLOCATOR_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_MALLOC_ALLOCATOR_H

#include <cstdlib>
#include "common/common_types.h"

#include "blocks/block.h"

namespace bridgerrholt {
	namespace allocators {

class MallocAllocator
{
	public:
		MallocAllocator() {}

		RawBlock allocate(SizeType size) {
			RawBlock toReturn {malloc(size), size};

			return toReturn;
		}

		void deallocate(RawBlock block) {
			free(block.getPtr());
		}
};



	}
}


#endif
