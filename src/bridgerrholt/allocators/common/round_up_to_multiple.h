#ifndef BRIDGERRHOLT_COMMON_ROUND_UP_TO_MULTIPLE_H
#define BRIDGERRHOLT_COMMON_ROUND_UP_TO_MULTIPLE_H

namespace bridgerrholt {
			namespace common {

template <class T1, class T2 = T1>
constexpr T1 roundUpToMultiple(T1 numberToRound, T2 factor) {
	T1 toReturn  {numberToRound};
	T1 remainder {toReturn % factor};

	if (remainder != 0)
		toReturn += factor - remainder;

	return toReturn;
}


	}
}

#endif
