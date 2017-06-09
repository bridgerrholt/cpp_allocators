#ifndef BRH_CPP_ALLOCATORS_SRC_BRH_ALLOCATORS_BLOCKS_BLOCK_H
#define BRH_CPP_ALLOCATORS_SRC_BRH_ALLOCATORS_BLOCKS_BLOCK_H

#include <type_traits>
#include <utility>
#include <cstddef>

#include "../common/common_types.h"

namespace brh {
	namespace allocators {

/*/// Contains the size in bytes. Does not contain a pointer.
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
};*/


/// Contains only a pointer. Does not contain the size.
/// Works correctly with void pointers.
template <class T>
class PtrBlockBase
{
	private:
		using This = PtrBlockBase<T>;

	public:
		using Type           = T;
		using DifferenceType = std::ptrdiff_t;
		using Pointer        = Type       *;
		using ConstPointer   = Type const *;

		constexpr PtrBlockBase() : PtrBlockBase(nullptr) {}
		constexpr PtrBlockBase(Pointer ptr) : ptr_ {ptr} {}

		constexpr bool isNull()    const { return (ptr_ == nullptr); }
		constexpr bool isNotNull() const { return !isNull(); }

		constexpr Pointer getPtr() const { return ptr_; }

		constexpr void setPtr(Pointer ptr) { ptr_ = ptr; }

		constexpr Pointer operator->() const { return  ptr_; }

		constexpr operator bool() const { return isNotNull(); }

		template <class U>
		constexpr operator PtrBlockBase<U>() const {
			return {static_cast<typename PtrBlockBase<U>::Pointer>(ptr_)};
		};

		constexpr This & operator++() { ++ptr_; return *this; }
		constexpr This & operator--() { --ptr_; return *this; }

		constexpr This operator++(int) {
			This temp {*this};
			++(*this);
			return temp;
		}

		constexpr This operator--(int) {
			This temp {*this};
			--(*this);
			return temp;
		}

		constexpr This operator+(DifferenceType n) const {
			This temp {*this};
			return temp += n;
		}


		constexpr This operator-(DifferenceType n) const {
			This temp {*this};
			return temp -= n;
		}

		constexpr This & operator+=(DifferenceType n) {
			ptr_ += n;
			return *this;
		}

		constexpr This & operator-=(DifferenceType n) {
			return (*this) += -n;
		}


	private:
		Pointer ptr_;
};


template <class T1, class T2>
constexpr bool operator==(PtrBlockBase<T1> const & first,
                          PtrBlockBase<T2> const & second) {
	return (first.getPtr() ==
		static_cast<typename PtrBlockBase<T1>::Pointer>(second.getPtr()));
}

template <class T1, class T2>
constexpr bool operator!=(PtrBlockBase<T1> const & first,
                          PtrBlockBase<T2> const & second) {
	return !(first == second);
}

template <class T1, class T2>
constexpr bool operator<(PtrBlockBase<T1> const & first,
                         PtrBlockBase<T2> const & second) {
	return (first.getPtr() <
	        static_cast<typename PtrBlockBase<T1>::Pointer>(second.getPtr()));
}

template <class T1, class T2>
constexpr bool operator>(PtrBlockBase<T1> const & first,
                         PtrBlockBase<T2> const & second) {
	return (first.getPtr() >
	        static_cast<typename PtrBlockBase<T1>::Pointer>(second.getPtr()));
}

template <class T1, class T2>
constexpr bool operator<=(PtrBlockBase<T1> const & first,
                          PtrBlockBase<T2> const & second) {
	return !(first > second);
}

template <class T1, class T2>
constexpr bool operator>=(PtrBlockBase<T1> const & first,
                          PtrBlockBase<T2> const & second) {
	return !(first < second);
}

template <class T>
constexpr PtrBlockBase<T> operator+(
	typename PtrBlockBase<T>::DifferenceType n,
	PtrBlockBase<T> const & block) {
	return block + n;
}

template <class T>
constexpr typename PtrBlockBase<T>::DifferenceType
operator-(PtrBlockBase<T> const & first,
          PtrBlockBase<T> const & second) {
	return (first.getPtr() - second.getPtr());
}


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

		using Reference      = Type       &;
		using ConstReference = Type const &;

		constexpr PtrBlock()            : PtrBlock(nullptr) {}
		constexpr PtrBlock(Pointer ptr) : BaseType(ptr) {}


		constexpr Reference operator* () const { return *toBase().operator->(); }
		constexpr Pointer   operator->() const { return  toBase().operator->(); }

	private:
		BaseType       & toBase()       { return *this; }
		BaseType const & toBase() const { return *this; }
};

/*
template <class T1, class T2>
constexpr bool operator==(PtrBlock<T1> const & first,
                          PtrBlock<T2> const & second) {
	return (static_cast<PtrBlockBase<T1> const &>(first) ==
		static_cast<PtrBlockBase<T1> const &>(
			static_cast<PtrBlockBase<T2> const &>(second)));
}

template <class T, class U>
constexpr bool operator!=(PtrBlock<T> const & first,
                          PtrBlock<U> const & second) {
	return !(first == second);
};
*/



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

		constexpr char * getCharPtr() const {
			return static_cast<char*>(getPtr());
		}
};



template <class T>
class BlockBaseTemplate : public PtrBlock<T>
{
	private:
		using PtrBase = PtrBlock<T>;

	public:
		using Type           = typename PtrBase::Type;
		using Pointer        = typename PtrBase::Pointer;
		using ConstPointer   = typename PtrBase::ConstPointer;

		constexpr BlockBaseTemplate() :
			BlockBaseTemplate(nullptr, 0) {}

		constexpr BlockBaseTemplate(Pointer ptr, SizeType size) :
			PtrBase (ptr),
			size_   {size} {}


		constexpr SizeType getSize() const { return size_; }

		constexpr void setSize(SizeType size) {
			size_ = size;
		}

		friend bool operator!=(BlockBaseTemplate const & first,
		                       BlockBaseTemplate const & second) {
			return !(first == second);
		}

	private:
		SizeType size_;
};

template <class T1, class T2 = T1>
constexpr bool operator==(BlockBaseTemplate<T1> const & first,
                          BlockBaseTemplate<T2> const & second) {
	return (
		static_cast<PtrBlock<T1> const &>(first) == second &&
		first.getSize() == second.getSize()
	);
}


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


		constexpr Pointer getEnd() const {
			return (BaseType::getPtr() + getElementCount());
		}

		constexpr SizeType getElementCount() const {
			return (BaseType::getSize() / sizeof(Type));
		}

		Reference operator* () { return *BaseType::getPtr(); }
		Pointer   operator->() { return  BaseType::getPtr(); }
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

		constexpr Pointer getEnd() const {
			return getEndChar();
		}

		constexpr char * getEndChar() const {
			return (getCharPtr() + BaseType::getSize());
		}
};

/*template <class T>
constexpr typename BasicBlock<T>::DifferenceType
operator-(BasicBlock<T> const & first,
          BasicBlock<T> const & second) {
	return (static_cast<PtrBlock<T> const &>(first) - static_cast<PtrBlock<T> const &>(second));
}*/

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
