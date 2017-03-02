#include "generate_instructions.h"


namespace bridgerrholt {
	namespace allocators {
		namespace tests {

Generator::Generator(
	AllocatorPolicy & allocator,
	std::size_t       totalSize,
	std::size_t       minimumSize,
	std::size_t       maximumSize,
	Flags             flags) : allocator_   {allocator},
													   totalSize_   {totalSize},
                             minimumSize_ {minimumSize},
                             maximumSize_ {maximumSize},
                             flags_       {flags} {}


InstructionList
Generator::generateWhole(std::ptrdiff_t seed,
                         std::size_t    minimumMainInstructions) {
	createEngine(seed);

	InstructionList list;
	generateFillSequence(list);
	generateMainSequence(list, minimumMainInstructions);


	return list;
}


InstructionList Generator::generateFillSequence(std::ptrdiff_t seed) {
	createEngine(seed);

	InstructionList list;
	generateFillSequence(list);

	return list;
}


InstructionList
Generator::generateMainSequence(std::ptrdiff_t seed,
															  std::size_t    minimumInstructions) {
	createEngine(seed);

	InstructionList list;
	generateMainSequence(list, minimumInstructions);

	return list;
}


void Generator::generateFillSequence(InstructionList & list) {
	std::size_t increment {allocator_.calcRequiredSize(minimumSize_)};
	std::size_t count = totalSize_ / increment;

	for (std::size_t i {0}; i < count; ++i) {
		list.emplace_back(
			Instruction::create<instructions::Allocate>({increment})
		);
	}


	if (flags_.isSet(FlagsEnum::IS_STACK)) {
		auto i = count;

		while (i > 0) {
			--i;
			list.emplace_back(
				Instruction::create<instructions::Deallocate>({i})
			);
		}
	}

	else {
		std::vector<std::size_t> remainingIndices;
		remainingIndices.reserve(count);

		for (std::size_t i {0}; i < count; ++i) {
			remainingIndices.push_back(i);
		}

		for (std::size_t i {0}; i < count; ++i) {
			std::uniform_int_distribution<std::size_t>
			dist {0, remainingIndices.size()};

			auto arrayPosition = remainingIndices.begin() + dist(randomEngine_);
			auto objectIndex = *arrayPosition;

			remainingIndices.erase(arrayPosition);

			list.emplace_back(
				Instruction::create<instructions::Deallocate>({objectIndex})
			);
		}
	}
}


void Generator::generateMainSequence(InstructionList & list,
                                     std::size_t       minimumInstructions) {
	// int instructionCount = 0
	// loop while instructionCount < minimumInstructions
	//   if (randomChoice(2))
	//     --index

	// A 5
	// D 0

	// A 5
	// A 10
	// D 1
	// D 0
}


void Generator::createEngine(std::ptrdiff_t seed) {
	randomEngine_ = RandomEngine(
		static_cast<typename RandomEngine::result_type>(seed)
	);
}


void Generator::fillWithRandom(char * firstByte, std::size_t byteCount) {
	using DataType = char;
	using Limits   = std::numeric_limits<DataType>;

	std::uniform_int_distribution<DataType>
		fillDistribution {Limits::min(), Limits::max()};

	auto byte    = firstByte;
	auto endByte = byte + byteCount;

	while (byte < endByte) {
		auto number = fillDistribution(randomEngine_);

		*byte = number;

		++byte;
	}
}


InstructionList generateInstructions(
	AllocatorPolicy & allocator,
	std::size_t       totalSize,
	std::size_t       minimumSize,
	std::size_t       maximumSize,
	Generator::Flags  flags)
{
	Generator generator {allocator, totalSize, minimumSize, maximumSize, flags};
	auto instructions = generator.generateWhole(0);

	return instructions;
}

		}
	}
}