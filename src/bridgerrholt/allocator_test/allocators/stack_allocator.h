#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_STACK_ALLOCATOR_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_STACK_ALLOCATOR_H

#include "../common_types.h"

#include "../block.h"

#include "../round_to_aligned.h"

namespace bridgerrholt {
	namespace allocator_test {

template <SizeType stackSize>
class StackAllocator
{
	public:
		StackAllocator() : back_ {array_} {}

		Block allocate(SizeType size) {

		}

	private:
		char   array_ [stackSize];
		char * back_;
};

	}
}

#endif
