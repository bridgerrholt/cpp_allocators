#include "instructions.h"

namespace brh {
	namespace allocators {
		namespace tests {
			namespace instructions {

// InstructionBase
//InstructionBase::InstructionBase() : type_ {NONE} {}

InstructionBase::InstructionBase(Type type) : type_ {type} {}



// Operator
Operator::Operator(Type        type,
                   std::size_t index) : InstructionBase (type),
                                        index_          {index} {}



// Allocator
Allocate::Allocate(std::size_t size) : InstructionBase (ALLOCATE),
                                       size_           {size} {}


bool Allocate::execute(AllocatorPolicy & allocator,
                       BlockList       & blockList)
{
	auto block = allocator.allocate(getSize());

	if (block.isNull())
		return false;

	blockList.push_back(block);
	return true;
}

std::string Allocate::makeAttributeString() const
{
	return "A " + std::to_string(getSize());
}



// Deallocate
Deallocate::Deallocate(std::size_t index) : Operator(DEALLOCATE, index) {}


bool Deallocate::execute(AllocatorPolicy & allocator,
                         BlockList       & blockList)
{
	allocator.deallocate(blockList.at(getIndex()));
	return true;
}


std::string Deallocate::makeAttributeString() const
{
	return "D " + std::to_string(getIndex());
}


// Reallocate
Reallocate::Reallocate(std::size_t index,
                       std::size_t size) : Operator (REALLOCATE, index),
                                           size_    {size} {}


bool Reallocate::execute(AllocatorPolicy & allocator,
                         BlockList       & blockList)
{
	return allocator.reallocate(blockList.at(getIndex()), getSize());
}


std::string Reallocate::makeAttributeString() const
{
	return "R " + std::to_string(getIndex()) + " " + std::to_string(getSize());
}



// Expand
Expand::Expand(std::size_t index,
               std::size_t amount) : Operator (EXPAND, index),
                                     amount_  {amount} {}


bool Expand::execute(AllocatorPolicy & allocator,
                     BlockList       & blockList)
{
	return allocator.expand(blockList.at(getIndex()), getAmount());
}


std::string Expand::makeAttributeString() const
{
	return "E " + std::to_string(getIndex()) + " " +
		            std::to_string(getAmount());
}


			}
		}
	}
}