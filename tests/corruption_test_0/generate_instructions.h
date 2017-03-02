#ifndef BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_0_GENERATE_INSTRUCTIONS_H
#define BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_0_GENERATE_INSTRUCTIONS_H

#include <iostream>

#include <array>
#include <cstddef>
#include <string>
#include <random>
#include <limits>

#include "instruction_list.h"
#include "allocator_policy.h"
#include "bit_flags.h"

namespace bridgerrholt {
	namespace allocators {
		namespace tests {


class Generator
{
	public:
		enum class FlagsEnum {
			ALLOW_REALLOCATE = 1 << 0,
			ALLOW_EXPAND     = 1 << 1,
			IS_STACK         = 1 << 2,
			ALL              = (1 << 0) | (1 << 1) | (1 << 2)
		};

		using Flags        = BasicBitFlags<FlagsEnum>;
		using FlagsWrapper = typename Flags::EnumWrapperType;

		Generator(
			AllocatorPolicy & allocator,
			std::size_t      totalSize,
			std::size_t      minimumSize,
			std::size_t      maximumSize,
			Flags            flags);

		InstructionList generateWhole(std::ptrdiff_t seed,
		                              std::size_t    minimumMainInstructions);

		InstructionList generateFillSequence(std::ptrdiff_t seed);
		InstructionList generateMainSequence(std::ptrdiff_t seed,
		                                     std::size_t    minimumInstructions);


	private:
		void generateFillSequence(InstructionList & list);
		void generateMainSequence(InstructionList & list,
		                          std::size_t       minimumInstructions);

		void createEngine(std::ptrdiff_t seed);

		void fillWithRandom(char * firstByte, std::size_t byteCount);

		using RandomEngine = std::mt19937;

		AllocatorPolicy & allocator_;
		std::size_t       totalSize_;
		std::size_t       minimumSize_;
		std::size_t       maximumSize_;
		Flags             flags_;

		RandomEngine randomEngine_;
};



InstructionList generateInstructions(
	AllocatorPolicy & allocator,
	std::size_t       totalSize,
	std::size_t       minimumSize,
	std::size_t       maximumSize,
	Generator::Flags flags = {
		Generator::FlagsWrapper(Generator::FlagsEnum::ALLOW_REALLOCATE) |
		Generator::FlagsWrapper(Generator::FlagsEnum::ALLOW_EXPAND)}
);




		}
	}
}

#endif
