#ifndef BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_0_INSTRUCTIONS_H
#define BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_0_INSTRUCTIONS_H

#include <cstddef>
#include <string>

#include "block_list.h"
#include "allocator_policy.h"

namespace bridgerrholt {
	namespace allocators {
		namespace tests {
			namespace instructions {

enum Type {
	NONE = 0,
	ALLOCATE,
	DEALLOCATE,
	REALLOCATE,
	EXPAND
};



class InstructionBase {
	public:
		InstructionBase();
		InstructionBase(Type type);

		virtual ~InstructionBase() = 0;

		virtual bool execute(AllocatorPolicy & allocator,
		                     BlockList       & blockList) = 0;

		Type getType() const { return type_; }

	private:
		Type type_;
};


class Operator : public InstructionBase {
	public:
		Operator() {}
		Operator(std::size_t index);

		std::size_t getIndex() const { return index_; }

	private:
		std::size_t index_;
};


class Allocate : public InstructionBase {
	public:
		Allocate() {}
		Allocate(std::size_t size);

		bool execute(AllocatorPolicy & allocator,
		             BlockList       & blockList) override;

		std::size_t getSize() const { return size_; }

	private:
		std::size_t size_;
};


class Deallocate : public Operator {
	public:
		Deallocate();
		Deallocate(std::size_t index);

		bool execute(AllocatorPolicy & allocator,
		             BlockList       & blockList) override;
};


class Reallocate : public Operator {
	public:
		Reallocate();
		Reallocate(std::size_t index, std::size_t size);

		bool execute(AllocatorPolicy & allocator,
		             BlockList       & blockList) override;

	private:
		std::size_t size_;
};


class Expand : public Operator {
	public:
		Expand();
		Expand(std::size_t index, std::size_t amount);

		bool execute(AllocatorPolicy & allocator,
		             BlockList       & blockList) override;

	private:
		std::size_t amount_;
};


			}
		}
	}
}

#endif
