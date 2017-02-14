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
class BlockBaseTemplate : public BlockBase
{
	public:
		using Type           = T;
		using Pointer        = Type       *;
		using ConstPointer   = Type const *;

		constexpr BlockBaseTemplate() : BlockBaseTemplate(nullptr, 0) {}

		constexpr BlockBaseTemplate(Pointer ptr, SizeType size) :
			BlockBase (size),
			ptr_      {ptr} {}


		bool isNull() const { return (ptr_ == nullptr); }

		ConstPointer getPtr() const { return ptr_; }
		Pointer      getPtr()       { return ptr_; }

		operator bool() const { return (getPtr() != nullptr); }

		friend bool operator==(BlockBaseTemplate const & first,
		                       BlockBaseTemplate const & second) {
			return (
				(static_cast<BlockBase const &>(first) == second) &&
				first.getPtr() == second.getPtr()
			);
		}

		friend bool operator!=(BlockBaseTemplate const & first,
		                       BlockBaseTemplate const & second) {
			return !(first == second);
		}


	private:
		Pointer ptr_;
};



template <class T>
class BasicBlock : public BlockBaseTemplate<T>
{
	private:
		using BaseType = BlockBaseTemplate<T>;

	public:
		using Type           = typename BaseType::Type;
		using Pointer        = typename BaseType::Pointer;
		using ConstPointer   = typename BaseType::ConstPointer;
		using Reference      = Type       &;
		using ConstReference = Type const &;

		static constexpr BasicBlock makeNullBlock() { return {}; }

		constexpr BasicBlock() : BasicBlock(nullptr, 0) {}

		constexpr BasicBlock(Pointer ptr, SizeType size) :
			BaseType(ptr, size) {}

		template <class Type>
		BasicBlock(BasicBlock<Type> block) :
			BasicBlock(static_cast<Pointer>(block.getPtr()), block.getSize()) {}


		Reference      operator*()       { return *BaseType::getPtr(); }
		ConstReference operator*() const { return *BaseType::getPtr(); }

		Reference      operator->()       { return *BaseType::getPtr(); }
		ConstReference operator->() const { return *BaseType::getPtr(); }
};



template <>
class BasicBlock<void> : public BlockBaseTemplate<void>
{
	private:
		using BaseType = BlockBaseTemplate<void>;

	public:
		using Type           = void;
		using Pointer        = Type       *;
		using ConstPointer   = Type const *;

		static constexpr BasicBlock makeNullBlock() { return {}; }

		constexpr BasicBlock() : BasicBlock {nullptr, 0} {}

		constexpr BasicBlock(Pointer ptr, SizeType size) :
			BaseType(ptr, size) {}

		template <class C>
		BasicBlock(BasicBlock<C> block) :
			BasicBlock {static_cast<Pointer>(block.getPtr()), block.getSize()} {}
};



class NullBlock : public BlockBase
{
	public:
		using Pointer      = std::nullptr_t;
		using ConstPointer = std::nullptr_t;

		static constexpr NullBlock makeNullBlock() { return {}; }

		constexpr NullBlock() {}
		constexpr NullBlock(Pointer, SizeType) {}


		constexpr bool isNull() const { return true; }

		constexpr Pointer getPtr() const { return nullptr; }

		constexpr operator bool() const { return false; }

		constexpr bool operator==(NullBlock const & other) {
			return true;
		}

		constexpr bool operator!=(NullBlock const & other) {
			return false;
		}
};


using RawBlock = BasicBlock<void>;


	}
}

#endif
