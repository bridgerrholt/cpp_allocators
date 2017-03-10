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

	protected:
		std::size_t value_;
};


template <class T>
using InstructionListContainer = std::vector<T>;

using InstructionListSizeType = InstructionListContainer<int>;

constexpr std::size_t InstructionSize =
	sizeof(InstructionBase) + sizeof(InstructionListSizeType);

class Instruction
{
	public:

};


class Allocate
{
	public:
		Allocate(std::size_t size, NodeList list);

	private:
		NodeList list;
};


	}

class NodeBase
{
	public:

};



class GeneratorInstance
{
	public:
		GeneratorInstance(
			InstructionList         & instructions,
			GenerationArgPack const & generationArgs,
			AllowedOperations         allowedOperations) :
				instructions_      (instructions),
				generationArgs_    (generationArgs),
				allowedOperations_ (allowedOperations) {

			std::size_t size  {0};
			std::size_t count {0};



		}

	private:
		InstructionList         & instructions_;
		GenerationArgPack const & generationArgs_;
		AllowedOperations         allowedOperations_;
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