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
		constexpr MallocAllocator() {}

		RawBlock allocate(SizeType size) const {
			return {allocatePtr(size), size};
		}

		void * allocatePtr(SizeType size) const {
			return std::malloc(size);
		}

		constexpr void deallocate(NullBlock) const {}

		void deallocate(RawBlock block) const {
			deallocate(block.getPtr());
		}

		void deallocate(void * ptr) const {
			std::free(ptr);
		}
};



	}
}


#endif
