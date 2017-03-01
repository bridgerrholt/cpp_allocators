#ifndef BRH_CPP_ALLOCATORS_BRIDGERRHOLT_ALLOCATORS_TRAITS_H
#define BRH_CPP_ALLOCATORS_BRIDGERRHOLT_ALLOCATORS_TRAITS_H

#include <utility>

namespace bridgerrholt {
	namespace allocators {
		namespace traits {

template <template <class T> class ArrayType, class T>
class RuntimeSizedArray : public ArrayType<T>
{
	public:
		RuntimeSizedArray(std::size_t size) : ArrayType<T> (size) {}
};

template <class t_ArrayType>
class ArrayPolicyBase
{
	public:
		friend void swap(ArrayPolicyBase & first, ArrayPolicyBase & second) {
			using std::swap;

			swap(first.array_, second.array_);
		}

		using ArrayType        = t_ArrayType;
		using ArrayReturn      = ArrayType       &;
		using ArrayConstReturn = ArrayType const &;

		template <class ... ArgTypes>
		constexpr ArrayPolicyBase(ArgTypes ... args) :
			array_ (std::forward<ArgTypes>(args)...) {}

		constexpr ArrayReturn      getArray()       { return array_; }
		constexpr ArrayConstReturn getArray() const { return array_; }

	private:
		ArrayType array_;
};

		}
	}
}

#endif
