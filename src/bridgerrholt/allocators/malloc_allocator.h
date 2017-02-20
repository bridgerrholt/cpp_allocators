#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_MALLOC_ALLOCATOR_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_MALLOC_ALLOCATOR_H

#include <cstdlib>
#include "common/common_types.h"

#include "blocks/block.h"

namespace bridgerrholt {
	namespace allocators {

/// A simple wrapper of the malloc family of functions.
class MallocAllocator
{
	public:
		using Handle = void *;

		constexpr MallocAllocator() {}

		Handle allocate(SizeType size) const {
			return std::malloc(size);
		}

		/// TODO: allocateAligned(SizeType size, SizeType alignment)

		bool reallocate(Handle & ptr, SizeType newSize) const {
			Handle newPtr {std::realloc(ptr, newSize)};

			if (newPtr != nullptr) {
				ptr = newPtr;
				return true;
			}

			else return false;
		}

		constexpr void deallocate(NullBlock) const {}

		void deallocate(Handle ptr) const {
			std::free(ptr);
		}
};



	}
}


#endif
