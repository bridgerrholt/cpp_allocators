#ifndef BRH_CPP_ALLOCATORS_SRC_BRH_ALLOCATORS_MALLOC_ALLOCATOR_H
#define BRH_CPP_ALLOCATORS_SRC_BRH_ALLOCATORS_MALLOC_ALLOCATOR_H

#include <cstddef>
#include <cstdlib>
#include "common/common_types.h"

#include "blocks/block.h"

namespace brh {
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

		/// Allocates the region but deallocates it if it's unaligned.
		Handle allocateAligned(SizeType size, SizeType alignment) const {
			auto ptr = allocate(size);

			if (reinterpret_cast<std::uintptr_t>(ptr) % alignment == 0)
				return ptr;

			else {
				deallocate(ptr);
				return nullptr;
			}
		}

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
