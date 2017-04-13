#ifndef BRH_CPP_ALLOCATORS_CORRUPTION_TEST_0_INSTRUCTION_H
#define BRH_CPP_ALLOCATORS_CORRUPTION_TEST_0_INSTRUCTION_H

#include <array>
#include <cstddef>
#include <string>
#include <random>
#include <limits>

#include "instructions.h"

namespace brh {
	namespace allocators {
		namespace tests {

class Instruction
{
	private:
		using BasePtr      = instructions::InstructionBase       *;
		using ConstBasePtr = instructions::InstructionBase const *;

	public:
		template <class T>
		Instruction(T && coreInstruction) {
			initialize<T>(std::move(coreInstruction));
		}

		template <class T>
		static Instruction create(T && coreInstruction) {
			return {std::move(coreInstruction)};
		}

		template <class T>
		void initialize(T && coreInstruction) {
			new (get()) T {std::move(coreInstruction)};
		}

		BasePtr get() {
			return reinterpret_cast<BasePtr>(rawArray_.data());
		}

		ConstBasePtr get() const {
			return reinterpret_cast<ConstBasePtr>(rawArray_.data());
		}

	private:
		Instruction() {}

		union InstructionUnion {
			public:
				InstructionUnion() {}

				instructions::Allocate   allocate;
				instructions::Deallocate deallocate;
				instructions::Reallocate reallocate;
				instructions::Expand     expand;
		};

		std::array<char, sizeof(InstructionUnion)> rawArray_;

};

		}
	}
}

#endif
