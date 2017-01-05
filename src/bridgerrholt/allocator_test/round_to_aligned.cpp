#include "round_to_aligned.h"

namespace {

using namespace bridgerrholt::allocator_test;

SizeType alignDown(SizeType size) {
	return ~(size - 1);
}

}

namespace bridgerrholt {
	namespace allocator_test {

SizeType roundToAligned(SizeType size)
{
	if (size < 8)
}

	}
}