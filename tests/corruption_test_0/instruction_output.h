#ifndef BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_0_INSTRUCTION_OUTPUT_H
#define BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_0_INSTRUCTION_OUTPUT_H

#include <iostream>

#include "instruction_list.h"

namespace bridgerrholt {
	namespace allocators {
		namespace tests {

std::ostream & operator<<(std::ostream                        & stream,
                          instructions::InstructionBase const * ptr) {
	stream << ptr->makeAttributeString();

	return stream;
}

std::ostream & operator<<(std::ostream          & stream,
                          InstructionList const & list) {
	for (auto const & i : list) {
		stream << i.get() << '\n';
	}

	return stream;
}

		}
	}
}

#endif
