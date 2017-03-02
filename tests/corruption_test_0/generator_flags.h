#ifndef BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_0_GENERATOR_FLAGS_H
#define BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_0_GENERATOR_FLAGS_H

#include "bit_flags.h"

namespace bridgerrholt {
	namespace allocators {
		namespace tests {

enum class AllowedOperationsEnum
{
	REALLOCATE = 1 << 0,
	EXPAND     = 1 << 1,
	ALL        = (1 << 0) | (1 << 1)
};

using AllowedOperations = BasicBitFlags<AllowedOperationsEnum>;

		}
	}
}

#endif
