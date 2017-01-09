#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_SEGREGATED_STORAGE_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_SEGREGATED_STORAGE_H

#include <vector>

#include "../common_types.h"

namespace bridgerrholt {
	namespace allocator_test {
		namespace allocators {

template <SizeType size>
class SegregatedStorage
{
	public:
		SegregatedStorage() {}

	private:
		char stackArray_[size];
};

		}
	}
}

#endif
