#ifndef BRH_CPP_ALLOCATORS_SRC_BRH_ALLOCATORS_BLOCKS_BLOCK_H
#define BRH_CPP_ALLOCATORS_SRC_BRH_ALLOCATORS_BLOCKS_BLOCK_H

#include <type_traits>
#include <utility>
#include <cstddef>

#include "../common/common_types.h"

namespace brh {
	namespace allocators {

/// Contains the size in bytes. Does not contain a pointer.
class BlockBase
{
	public:
		constexpr BlockBase() : BlockBase(0) {}
		constexpr BlockBase(SizeType size) : size_ {size} {}

		constexpr SizeType getSize() const { return size_; }

		constexpr void setSize(SizeType size) {
			size_ = size;
		}


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


/// Contains only a pointer. Does not contain the size.
/// Works correctly with void pointers.
template <class T>
class PtrBlockBase
{
	public:
		using Type           = T;
		using Pointer        = Type       *;
		using ConstPointer   = Type const *;

		constexpr PtrBlockBase() : PtrBlockBase(nullptr) {}

		constexpr PtrBlockBase(Pointer ptr) : ptr_ {ptr} {}


		constexpr bool isNull() const { return (ptr_ == nullptr); }

		constexpr ConstPointer getPtr() const { return ptr_; }
		constexpr Pointer      getPtr()       { return ptr_; }

		constexpr void setPtr(Pointer ptr) { ptr_ = ptr; }


		constexpr operator bool() const { return (getPtr() != nullptr); }

		constexpr friend bool operator==(PtrBlockBase const & first,
		                                 PtrBlockBase const & second) {
			return (first.getPtr() == second.getPtr());
		}

		constexpr friend bool operator!=(PtrBlockBase const & first,
		                                 PtrBlockBase const & second) {
			return !(first == second);
		}


	private:
		Pointer ptr_;
};


/// Has features that do not work with void pointers,
/// specialization works with void pointers.
template <class T>
class PtrBlock : public PtrBlockBase<T>
{
	private:
		using BaseType = PtrBlockBase<T>;

	public:
		using Type           = typename BaseType::Type;
		using Pointer        = typename BaseType::Pointer;
		using ConstPointer   = typename BaseType::ConstPointer;

		using Reference      = Type &;
		using ConstReference = Type const &;

		constexpr PtrBlock() : PtrBlock(nullptr) {}

		constexpr PtrBlock(Pointer ptr) : BaseType(ptr) {}


		Reference      operator*()       { return *BaseType::getPtr(); }
		ConstReference operator*() const { return *BaseType::getPtr(); }

		Reference      operator->()       { return *BaseType::getPtr(); }
		ConstReference operator->() const { return *BaseType::getPtr(); }

		constexpr friend bool operator==(PtrBlock const & first,
		                                 PtrBlock const & second) {
			return (static_cast<BaseType const &>(first) == second);
		}

		constexpr friend bool operator!=(PtrBlock const & first,
		                                 PtrBlock const & second) {
			return !(first == second);
		}
};


/// Specialization for void pointers, doesn't have all the features.
template <>
class PtrBlock<void> : public PtrBlockBase<void>
{
	private:
		using BaseType = PtrBlockBase<void>;

	public:
		using Type           = typename BaseType::Type;
		using Pointer        = typename BaseType::Pointer;
		using ConstPointer   = typename BaseType::ConstPointer;

		constexpr PtrBlock() : PtrBlock(nullptr) {}

		constexpr PtrBlock(Pointer ptr) : BaseType(ptr) {}

		constexpr char * getCharPtr() {
			return static_cast<char*>(getPtr());
		}
		constexpr char const * getCharPtr() const {
			return static_cast<char const *>(getPtr());
		}
};



template <class T>
class BlockBaseTemplate : public BlockBase, public PtrBlock<T>
{
	private:
		using PtrBase = PtrBlock<T>;

	public:
		using Type           = typename PtrBase::Type;
		using Pointer        = typename PtrBase::Pointer;
		using ConstPointer   = typename PtrBase::ConstPointer;

		constexpr BlockBaseTemplate() : BlockBaseTemplate(nullptr, 0) {}

		constexpr BlockBaseTemplate(Pointer ptr, SizeType size) :
			BlockBase (size),
			PtrBase   (ptr) {}

		friend bool operator==(BlockBaseTemplate const & first,
		                       BlockBaseTemplate const & second) {
			return (
				static_cast<BlockBase const &>(first) == second &&
				static_cast<PtrBase   const &>(first) == second
			);
		}

		friend bool operator!=(BlockBaseTemplate const & first,
		                       BlockBaseTemplate const & second) {
			return !(first == second);
		}
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


		constexpr Pointer getEnd() {
			return (BaseType::getPtr() + getElementCount());
		}

		constexpr ConstPointer getEnd() const {
			return (BaseType::getPtr() + getElementCount());
		}

		constexpr SizeType getElementCount() const {
			return (BaseType::getSize() / sizeof(Type));
		}

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

		constexpr Pointer getEnd() {
			return getEndChar();
		}

		constexpr ConstPointer getEnd() const {
			return getEndChar();
		}

		constexpr char * getEndChar() {
			return (getCharPtr() + BaseType::getSize());
		}

		constexpr char const * getEndChar() const {
			return (getCharPtr() + BaseType::getSize());
		}
};


class NullBlock
{
	public:
		using Pointer      = std::nullptr_t;
		using ConstPointer = std::nullptr_t;

		static constexpr NullBlock makeNullBlock() { return {}; }

		constexpr NullBlock() {}


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


using RawPtr   = PtrBlock  <void>;
using RawBlock = BasicBlock<void>;


	}
}

#endif
