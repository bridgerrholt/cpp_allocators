#include "instructions.h"

namespace bridgerrholt {
	namespace allocators {
		namespace tests {
			namespace instructions {

// InstructionBase
InstructionBase::InstructionBase() : type_ {NONE} {}

InstructionBase::InstructionBase(Type type) : type_ {type} {}

InstructionBase::~InstructionBase() {}


// Operator
Operator::Operator(std::size_t index) : index_ {index} {}



// Allocator
Allocate::Allocate(std::size_t size) : size_ {size} {}

bool Allocate::execute(AllocatorPolicy & allocator,
                       BlockList       & blockList)
{
	auto block = allocator.allocate(getSize());

	if (block.isNull())
		return false;

	blockList.push_back(block);
	return true;
}


// Deallocate
Deallocate::Deallocate(std::size_t index) : Operator(index) {}

bool Deallocate::execute(AllocatorPolicy & allocator,
                         BlockList       & blockList)
{
	allocator.deallocate(blockList.at(getIndex()));
	return true;
}


// Reallocate
Reallocate::Reallocate(std::size_t index,
                       std::size_t size) : Operator(index),
                                           size_ {size} {}

bool Reallocate::execute(AllocatorPolicy & allocator,
             BlockList       & blockList) override;



class Expand : public Operator {
	public:
		Expand::Expand();
		Expand::Expand(std::size_t index, std::size_t amount);

		bool Expand::execute(AllocatorPolicy & allocator,
		             BlockList       & blockList) override;

	private:
		std::size_t amount_;
};


			}
		}
	}
}