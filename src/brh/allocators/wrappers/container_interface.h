#ifndef BRH_CPP_ALLOCATORS_SRC_BRH_ALLOCATORS_WRAPPERS_CONTAINER_INTERFACE_H
#define BRH_CPP_ALLOCATORS_SRC_BRH_ALLOCATORS_WRAPPERS_CONTAINER_INTERFACE_H

#include <cstddef>
#include <iterator>

#include "../blocks/block.h"

namespace brh {
	namespace allocators {

/// Allows usage of a BasicBlock as a standard pointer.
template <class T>
class AllocatorPointerInterface
{
	public:
		using This      = AllocatorPointerInterface<T>;
		using BlockType = BasicBlock<T>;

		using difference_type   = std::ptrdiff_t;
		using size_type         = std::size_t;
		using value_type        = T;
		using pointer           = value_type *;
		using reference         = value_type &;
		using iterator_category = std::random_access_iterator_tag;

		constexpr AllocatorPointerInterface() :
			AllocatorPointerInterface(nullptr) {}

		constexpr AllocatorPointerInterface(std::nullptr_t) :
			AllocatorPointerInterface(BlockType()) {}

		constexpr AllocatorPointerInterface(BlockType block) :
			block_ (block) {}

		constexpr friend void swap(This & first, This & second) {
			using std::swap;

			swap(first.block_, second.block_);
		}

		BlockType       & getBlock()       { return block_; }
		BlockType const & getBlock() const { return block_; }

		reference operator*() const {
			return *block_.getPtr();
		}

		pointer operator->() const {
			return block_.getPtr();
		}

		reference operator[](difference_type index) {
			return *(block_.getPtr() + index);
		}

		operator bool() { return !block_.isNull(); }

		template <class U>
		explicit operator AllocatorPointerInterface<U>() {
			return static_cast<
				typename AllocatorPointerInterface<U>::BlockType
			>(block_);
		}

		This & operator++() {
			return *this += 1;
		}

		This & operator--() {
			return *this -= 1;
		}

		This operator++(int) {
			This temp {*this};
			++(*this);
			return temp;
		}

		This operator--(int) {
			This temp {*this};
			--(*this);
			return temp;
		}

		/*template <class N>
		This operator+(N value) const {
			This toReturn {*this};
			return (toReturn += value);
		}

		template <class N>
		This operator-(N value) const {
			This toReturn {*this};
			return (toReturn -= value);
		}*/


		template <class N>
		This & operator+=(N amount) {
			block_ += amount;
			return *this;
		}

		template <class N>
		This & operator-=(N amount) {
			return (*this += -amount);
		}

		/*template <class U>
		This & operator+=(AllocatorPointerInterface<U> const & other) {
			block_ += other.getBlock();
			return *this;
		}*/



	private:
		BlockType block_;
};




template <class T1, class T2 = T1>
constexpr bool operator==(AllocatorPointerInterface<T1> const & first,
                          AllocatorPointerInterface<T2> const & second) {
	return (first.getBlock() == second.getBlock());
}

template <class T1, class T2 = T1>
constexpr bool operator==(AllocatorPointerInterface<T1> const & first,
                          typename AllocatorPointerInterface<T2>::pointer ptr) {
	return (first.getBlock().getPtr() == ptr);
}


template <class T1, class T2 = T1>
constexpr bool operator!=(AllocatorPointerInterface<T1> const & first,
                          AllocatorPointerInterface<T2> const & second) {
	return !(first == second);
}

template <class T1, class T2 = T1>
constexpr bool operator!=(AllocatorPointerInterface<T1> const & first,
                          typename AllocatorPointerInterface<T2>::pointer ptr) {
	return !(first == ptr);
}


template <class T1, class T2>
constexpr bool operator<(AllocatorPointerInterface<T1> const & first,
                         AllocatorPointerInterface<T2> const & second) {
	return (first.getBlock() < second.getBlock());
}

/*friend bool operator!=(This const & first,
                       This const & second) {
	return !(first == second);
}

friend bool operator>(This const & first,
                      This const & second) {
	return (first.block_.getPtr() > second.block_.getPtr());
}

friend bool operator<(This const & first,
                      This const & second) {
	return (first.block_.getPtr() < second.block_.getPtr());
}

friend bool operator>=(This const & first,
                       This const & second) {
	return (first.block_.getPtr() >= second.block_.getPtr());
}

friend bool operator<=(This const & first,
                       This const & second) {
	return (first.block_.getPtr() <= second.block_.getPtr());
}*/


/*template <class T>
AllocatorPointerInterface<T>
constexpr operator+(AllocatorPointerInterface<T> const & first,
                    AllocatorPointerInterface<T> const & second) {
	AllocatorPointerInterface<T> toReturn {first};
	return (toReturn += second);
}*/

template <class N, class T>
AllocatorPointerInterface<T>
constexpr operator+(N first, AllocatorPointerInterface<T> const & second) {
	return second + first;
}

template <class T, class N>
AllocatorPointerInterface<T>
constexpr operator+(AllocatorPointerInterface<T> const & first, N second) {
	AllocatorPointerInterface<T> toReturn {first};
	return (toReturn += second);
}


template <class T>
constexpr auto
operator-(AllocatorPointerInterface<T> const & first,
          AllocatorPointerInterface<T> const & second
) -> decltype(first.getBlock() - second.getBlock()) {
	return first.getBlock() - second.getBlock();
}


template <class T, class N>
AllocatorPointerInterface<T>
constexpr operator-(AllocatorPointerInterface<T> const & first, N second) {
	AllocatorPointerInterface<T> toReturn {first};
	return (toReturn -= second);
}



template <class A, class T>
class ContainerInterface
{
	public:
		using pointer            = AllocatorPointerInterface<T>;
		using const_pointer      = AllocatorPointerInterface<T const>;
		using void_pointer       = AllocatorPointerInterface<void>;
		using const_void_pointer = AllocatorPointerInterface<void const>;
		using value_type         = T;
		using size_type          = std::size_t;
		using difference_type    = std::ptrdiff_t;

		template <class U>
		using rebind = ContainerInterface<A, U>;

		ContainerInterface(A & allocator) : allocator_ {allocator} {}

		pointer allocate(size_type size) {
			return pointer(allocator_.allocate(size * sizeof(T)));
		}

		void deallocate(pointer ptr, size_type) {
			allocator_.deallocate(ptr.getBlock());
		}

	private:
		A & allocator_;
};

	}
}

#endif
