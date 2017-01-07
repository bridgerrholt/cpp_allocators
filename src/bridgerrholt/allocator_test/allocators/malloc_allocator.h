#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_MALLOC_ALLOCATOR_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_MALLOC_ALLOCATOR_H

#include <cstdlib>
#include "../common_types.h"

#include "../block.h"

namespace bridgerrholt {
	namespace allocator_test {

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
