#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_STACK_ALLOCATOR_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_STACK_ALLOCATOR_H

#include "../common_types.h"

#include "../block.h"

namespace bridgerrholt {
	namespace allocators {

template <SizeType stackSize>
class StackAllocator
{
	public:
		StackAllocator() : back_ {array_} {}

		RawBlock allocate(SizeType size) {

		}

	private:
		char   array_ [stackSize];
		char * back_;
};

	}
}

#endif
