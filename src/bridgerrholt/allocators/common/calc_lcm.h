#ifndef BRIDGERRHOLT_COMMON_CALC_LCM_H
#define BRIDGERRHOLT_COMMON_CALC_LCM_H

#include "calc_gcd.h"

namespace bridgerrholt {
	namespace common {

/// Calculates the least common multiple of two numbers.
template <class T>
constexpr T calcLcm(T first, T second) {
	return ((first / calcGcd(first, second)) * second);
}


	}
}

#endif
