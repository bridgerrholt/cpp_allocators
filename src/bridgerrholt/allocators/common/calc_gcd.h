#ifndef BRIDGERRHOLT_COMMON_CALC_GCD_H
#define BRIDGERRHOLT_COMMON_CALC_GCD_H

namespace bridgerrholt {
	namespace common {

/// Calculates the greatest common divisor of
/// two numbers using the Euclidean algorithm.
template <class T>
constexpr T calcGcd(T first, T second)
{
	while (second != 0) {
		auto temp = second;
		second = first % second;
		first = temp;
	}

	return first;
}


	}
}

#endif
