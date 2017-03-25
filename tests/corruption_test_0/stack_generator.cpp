#include "stack_generator.h"

#include <cassert>
#include <vector>
#include <memory>
#include <tuple>

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

using InstructionList = std::vector<std::unique_ptr<InstructionBase> >;



class Allocate : public InstructionBase
{
	public:
		Allocate(std::size_t size) :
			Allocate (size, {}) {}

		Allocate(std::size_t size, InstructionList list) :
			InstructionBase (size),
      list_           (std::move(list)) {}

		std::size_t getSize()   const { return getValue(); }

		InstructionList       & getList()       { return list_; }
		InstructionList const & getList() const { return list_; }

	private:
		InstructionList list_;
};


class Block : public Allocate
{
	public:
		template <class ... ArgTypes>
		Block(Block * parent, ArgTypes ... args) :
			Allocate(std::forward<ArgTypes>(args)...), parent_ {parent} {}

		Block       * getParent()       { return parent_; }
		Block const * getParent() const { return parent_; }

	private:
		Block * parent_;
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
		Reallocate(std::size_t size) : InstructionBase(size) {}

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
				allowedOperations_ (allowedOperations),
				currentBlock_ {nullptr},
				size_  {0},
				count_ {0} {

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

		bool generate() {
			stack_instructions::InstructionList list;

			bool success {true};

			while (count_ < generationArgs_.minInstructionCount) {
				if (currentBlock_ == nullptr) {
					stack_instructions::Block * nextBlock;
					if (makeBlock(nextBlock, list, nullptr))
						currentBlock_ = nextBlock;
					else {
						success = false;
						break;
					}
				}

				else {
					Action action;
					if (size_ == generationArgs_.maxAllocationSize) {
						action = Action::END_BLOCK;
					}
					else {
						action = randomAction();
					}

					switch (action) {
						case Action::END_BLOCK:
							currentBlock_ = currentBlock_->getParent();
							break;

						case Action::ALLOCATE:
							currentBlock_ = makeBlock();
							break;

						case Action::REALLOCATE:
							makeReallocate();
							break;

						case Action::EXPAND:
							makeExpand();
							break;
					}
				}
			}

			if (!success)
				return false;

			return true;
		}

		bool generateSize(std::size_t & outSize) const {
			auto max = std::min(generationArgs_.maxAllocationSize,
			                    generationArgs_.totalSize - size_);

			std::uniform_int_distribution<std::size_t> distribution {
				generationArgs_.minAllocationSize, max
			};

			auto value =
				generationArgs_.allocator.calcRequiredSize(
					distribution(randomEngine_)
				);

			if (value <= generationArgs_.maxAllocationSize) {
				outSize = value;
				return true;
			}

			else return false;
		}

		Action randomAction() const {
			std::uniform_int_distribution<std::size_t> distribution {
				static_cast<std::size_t>(Action::_START),
				static_cast<std::size_t>(Action::_END)
			};

			return static_cast<Action>(distribution(randomEngine_));
		}

		bool makeBlock(stack_instructions::Block * & outBlock) {
			return makeBlock(outBlock, currentBlock_->getList(), currentBlock_);
		}

		// False if generateSize() failed.
		bool
		makeBlock(stack_instructions::Block         * & outBlock,
							stack_instructions::InstructionList & list,
		          stack_instructions::Block           * parentBlock) {
			std::size_t nextSize;
			if (generateSize(nextSize)) {
				size_  += nextSize;
				count_ += 2;

				list.emplace_back(
					new stack_instructions::Block(parentBlock, nextSize)
				);

				outBlock = &static_cast<stack_instructions::Block&>(*list.back());

				return true;
			}

			else {
				return false;
			}
		}

		bool makeReallocate() {
			auto oldSize = currentBlock_->getSize();
			size_ -= oldSize;

			std::size_t newSize;
			if (!generateSize(newSize)) {
				size_ += oldSize;
				return false;
			}

			size_ += newSize;

			++count_;

			currentBlock_->getList().emplace_back(
				new stack_instructions::Reallocate(newSize)
			);

			return true;
		}

		void makeExpand() {
			auto amount = generateSize();
			size_ += amount;
		}

		bool isComplete() const {
			auto maxSize = generationArgs_.maxAllocationSize;

			assert(maxSize <= size_);

			return (
				size_  == maxSize &&
				count_ >= generationArgs_.minInstructionCount
			);
		}



		InstructionList                  & instructions_;
		StackGenerator::RandomEngineType & randomEngine_;
		GenerationArgPack          const & generationArgs_;
		AllowedOperations                  allowedOperations_;
		stack_instructions::Block        * currentBlock_;

		std::size_t size_;
		std::size_t count_;
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