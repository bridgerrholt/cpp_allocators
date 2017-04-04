#ifndef BRIDGERRHOLT_COMMON_CALC_IS_ALIGNED_H
#define BRIDGERRHOLT_COMMON_CALC_IS_ALIGNED_H

#include <cstdint>

namespace bridgerrholt {
	namespace common {

/// Calculates whether the value is aligned or not.
template <class P, class T = std::size_t>
constexpr bool calcIsAligned(P * ptr, T alignment)
{
	return (reinterpret_cast<std::uintptr_t>(ptr) % alignment == 0);
}


	}
}

#endif
