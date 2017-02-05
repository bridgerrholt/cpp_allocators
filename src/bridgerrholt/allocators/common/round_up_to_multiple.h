#ifndef BRIDGERRHOLT_COMMON_ROUND_UP_TO_MULTIPLE_H
#define BRIDGERRHOLT_COMMON_ROUND_UP_TO_MULTIPLE_H

namespace bridgerrholt {
			namespace common {

template <class T>
constexpr T roundUpToMultiple(T numberToRound, T factor) {
	T toReturn {numberToRound};
	T remainder {toReturn % factor};

	if (remainder != 0)
		toReturn += factor - remainder;

	return toReturn;
}


	}
}

#endif
