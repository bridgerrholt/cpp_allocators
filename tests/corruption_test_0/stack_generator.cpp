#include "stack_generator.h"

#include <vector>

namespace {

using namespace bridgerrholt::allocators;
using namespace bridgerrholt::allocators::tests;

	namespace stack_instructions {

class InstructionBase
{
	public:
		InstructionBase(std::size_t value) : value_ {value} {}

		std::size_t getValue() const { return value_; }

	private:
		std::size_t value_;
};


template <class T>
using InstructionListContainer = std::vector<T>;

using InstructionListSizeType = InstructionListContainer<int>;

constexpr std::size_t instructionSize =
	sizeof(InstructionBase) + sizeof(InstructionListSizeType);

class Instruction
{
	private:
		using BasePtr = InstructionBase *;

	public:
		template <class T>
		Instruction(T coreInstruction) {
			new (rawArray_.data()) T {std::move(coreInstruction)};
		}

		BasePtr get() { return reinterpret_cast<BasePtr>(rawArray_.data()); }

	private:
		std::array<char, instructionSize> rawArray_;
};

using InstructionList = std::vector<Instruction>;


class Allocate : public InstructionBase
{
	public:
		Allocate(std::size_t size) : InstructionBase (size) {}

		Allocate(std::size_t size, InstructionList list) :
			Allocate (size),
      list_    (list) {}

		std::size_t getSize() const { return getValue(); }

		InstructionList const & getList() const { return list_; }

	private:
		InstructionList list_;
};


class Expand : public InstructionBase
{
	public:
		Expand(std::size_t amount) : InstructionBase(amount) {}

		std::size_t getAmount() const { return getValue(); }
};


class Reallocate : public InstructionBase
{
	public:
		Reallocate(std::size_t size) : Reallocate(size) {}

		std::size_t getSize() const { return getValue(); }
};


	}


class GeneratorInstance
{
	public:
		GeneratorInstance(
			InstructionList                  & instructions,
			StackGenerator::RandomEngineType & randomEngine,
			GenerationArgPack          const & generationArgs,
			AllowedOperations                  allowedOperations) :
				instructions_      {instructions},
				randomEngine_      {randomEngine},
				generationArgs_    (generationArgs),
				allowedOperations_ (allowedOperations) {

			generate();
		}


	private:
		enum class Action {
			END_BLOCK = 1,
			ALLOCATE,
			REALLOCATE,
			EXPAND,

			_START = 1,
			_END   = 4
		};

		void generate() {
			std::size_t size  {0};
			std::size_t count {0};

			stack_instructions::InstructionList list;

			stack_instructions::InstructionList * currentList {&list};

			while (true) {
				auto nextSize = generateSize(generationArgs_.totalSize - count);

				size += nextSize;
				++count;

				currentList->emplace_back(stack_instructions::Allocate(nextSize));

				auto action = randomAction();

				switch (action) {
					case END_BLOCK:
				}
			}
		}

		std::size_t generateSize(std::size_t remainingSpace) const {
			auto max = std::min(generationArgs_.maxAllocationSize,
			                    remainingSpace);

			std::uniform_int_distribution<std::size_t> distribution {
				generationArgs_.minAllocationSize, max
			};

			return distribution(randomEngine_);
		}

		Action randomAction() const {
			std::uniform_int_distribution<std::size_t> distribution {
				static_cast<std::size_t>(Action::_START),
				static_cast<std::size_t>(Action::_END)
			};

			return static_cast<Action>(distribution(randomEngine_));
		}



		InstructionList                  & instructions_;
		StackGenerator::RandomEngineType & randomEngine_;
		GenerationArgPack          const & generationArgs_;
		AllowedOperations                  allowedOperations_;
};

}


namespace bridgerrholt {
	namespace allocators {
		namespace tests {

InstructionList StackGenerator::generate(
	GenerationArgPack generationArgs,
	AllowedOperations allowedOperations)
{

}



InstructionList StackGenerator::generateFillSequence(
	AllocatorPolicy & allocator,
	SizeType          totalSize)
{
	InstructionList toReturn;
	generateFillSequence(toReturn, allocator, totalSize);
	return toReturn;
}


InstructionList StackGenerator::generateMainSequence(
	GenerationArgPack generationArgs,
	AllowedOperations allowedOperations)
{
	InstructionList toReturn;
	generateMainSequence(toReturn, generationArgs, allowedOperations);
	return toReturn;
}



void StackGenerator::generateFillSequence(
	InstructionList & list,
	AllocatorPolicy & allocator,
	SizeType          totalSize)
{
	auto firstIndex = list.size();
	auto lastIndex  = firstIndex;

	SizeType size {0};

	while (size < totalSize) {
		auto toAdd = allocator.calcRequiredSize(1);

		if (totalSize - size < toAdd)
			break;

		list.emplace_back(instructions::Allocate(toAdd));

		++lastIndex;
		size += toAdd;
	}

	for (auto i = firstIndex; i < lastIndex; ++i) {
		list.emplace_back(instructions::Deallocate(i));
	}
}



void StackGenerator::generateMainSequence(
	InstructionList         & list,
	GenerationArgPack const & generationArgs,
	AllowedOperations         allowedOperations)
{

}



		}
	}
}