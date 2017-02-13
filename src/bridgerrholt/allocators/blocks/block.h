#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_BLOCKS_BLOCK_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_BLOCKS_BLOCK_H

#include <type_traits>
#include <utility>
#include <cstddef>

#include "../common/common_types.h"

namespace bridgerrholt {
	namespace allocators {

class BlockBase
{
	public:
		constexpr BlockBase() : BlockBase(0) {}
		constexpr BlockBase(SizeType size) : size_ {size} {}

		SizeType getSize() const { return size_; }


		friend bool operator==(BlockBase const & first,
		                       BlockBase const & second) {
			return (first.getSize() == second.getSize());
		}

		friend bool operator!=(BlockBase const & first,
		                       BlockBase const & second) {
			return !(first == second);
		}

	private:
		SizeType size_;
};



template <class T>
class BasicBlock : public BlockBase
{
	public:
		using Type           = T;
		using Pointer        = Type       *;
		using ConstPointer   = Type const *;
		using Reference      = Type       &;
		using ConstReference = Type const &;

		static constexpr BasicBlock makeNullBlock() { return {}; }

		constexpr BasicBlock() : BasicBlock(nullptr, 0) {}

		constexpr BasicBlock(Pointer ptr, SizeType size) :
			BlockBase (size),
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

		Reference      operator*()       { return *getPtr(); }
		ConstReference operator*() const { return *getPtr(); }

		Reference      operator->()       { return *getPtr(); }
		ConstReference operator->() const { return *getPtr(); }

		operator bool() const { return (getPtr() != nullptr); }

		friend bool operator==(BasicBlock const & first,
		                       BasicBlock const & second) {
			return (static_cast<BlockBase const &>(first) == second &&
			        first.getPtr() == second.getPtr());
		}

		friend bool operator!=(BasicBlock const & first,
		                       BasicBlock const & second) {
			return !(first == second);
		}


	private:
		Pointer ptr_;
};



template <>
class BasicBlock<void> : public BlockBase
{
	public:
		using Type           = void;
		using Pointer        = Type       *;
		using ConstPointer   = Type const *;

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

		operator bool() const { return (getPtr() != nullptr); }

		friend bool operator==(BasicBlock const & first,
		                       BasicBlock const & second) {
			return (static_cast<BlockBase const &>(first) == second &&
			        first.getPtr() == second.getPtr());
		}

		friend bool operator!=(BasicBlock const & first,
		                       BasicBlock const & second) {
			return !(first == second);
		}


	private:
		Pointer ptr_;
};



template <>
class BasicBlock<std::nullptr_t> : public BlockBase
{
	public:
		using Pointer      = std::nullptr_t;
		using ConstPointer = std::nullptr_t;

		static constexpr BasicBlock makeNullBlock() { return {}; }

		constexpr BasicBlock() {}
		constexpr BasicBlock(Pointer, SizeType) {}


		constexpr bool isNull() const { return true; }

		constexpr Pointer getPtr() const { return nullptr; }

		constexpr operator bool() const { return false; }

		constexpr bool operator==(BasicBlock const & other) {
			return true;
		}

		constexpr bool operator!=(BasicBlock const & other) {
			return false;
		}
};



using RawBlock  = BasicBlock<void>;
using NullBlock = BasicBlock<std::nullptr_t>;


	}
}

#endif
