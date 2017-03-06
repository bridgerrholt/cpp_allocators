#ifndef BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_0_INSTRUCTIONS_H
#define BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_0_INSTRUCTIONS_H

#include <iostream>

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
		InstructionBase(Type type);

		virtual bool execute(AllocatorPolicy & allocator,
		                     BlockList       & blockList) = 0;

		Type getType() const { return type_; }

		virtual std::string makeAttributeString() const = 0;

	private:
		Type type_;
};


class Operator : public InstructionBase {
	public:
		Operator(Type type, std::size_t index);

		std::size_t getIndex() const { return index_; }

	private:
		std::size_t index_;
};


class Allocate : public InstructionBase {
	public:
		Allocate(std::size_t size);

		bool execute(AllocatorPolicy & allocator,
		             BlockList       & blockList) override;

		std::string makeAttributeString() const override;

		std::size_t getSize() const { return size_; }

	private:
		std::size_t size_;
};


class Deallocate : public Operator {
	public:
		Deallocate(std::size_t index);

		bool execute(AllocatorPolicy & allocator,
		             BlockList       & blockList) override;

		std::string makeAttributeString() const override;
};


class Reallocate : public Operator {
	public:
		Reallocate(std::size_t index, std::size_t size);

		bool execute(AllocatorPolicy & allocator,
		             BlockList       & blockList) override;

		std::string makeAttributeString() const override;

		std::size_t getSize() const { return size_; }

	private:
		std::size_t size_;
};


class Expand : public Operator {
	public:
		Expand(std::size_t index, std::size_t amount);

		bool execute(AllocatorPolicy & allocator,
		             BlockList       & blockList) override;

		std::string makeAttributeString() const override;

		std::size_t getAmount() const { return amount_; }

	private:
		std::size_t amount_;
};


			}
		}
	}
}

#endif
