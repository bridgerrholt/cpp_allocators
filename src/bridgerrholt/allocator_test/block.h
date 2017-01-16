#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_BLOCK_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_BLOCK_H

#include <type_traits>
#include <utility>

#include "common_types.h"

namespace bridgerrholt {
	namespace allocator_test {

class BlockBase
{
	public:
		BlockBase() : BlockBase {0} {}
		BlockBase(SizeType size) : size_ {size} {}

		SizeType getSize() const { return size_; }

	private:
		SizeType size_;
};


template <class T>
class BasicBlock : public BlockBase
{
	public:
		using Type         = T;
		using Pointer      = Type       *;
		using ConstPointer = Type const *;

		static constexpr BasicBlock makeNullBlock() { return {}; }

		constexpr BasicBlock() : BasicBlock {nullptr, 0} {}

		constexpr BasicBlock(Pointer ptr, SizeType size) :
			BlockBase {size},
			ptr_      {ptr} {}

		template <class C>
		BasicBlock(BasicBlock<C> block) :
			BasicBlock {static_cast<Pointer>(block.getPtr()), block.getSize()} {
			// Can only convert from a child to a base or from/to void.
			static_assert(
				(std::is_base_of<Type, C>() ||
				 std::is_same<void, C>()    ||
				 std::is_same<void, Type>()),
				"Cannot convert to non-base"
			);
		}

		bool isNull() const { return (ptr_ == nullptr); }

		ConstPointer getPtr() const { return ptr_; }
		Pointer      getPtr()       { return ptr_; }


	private:
		Pointer ptr_;
};


using RawBlock = BasicBlock<void>;


	}
}

#endif
