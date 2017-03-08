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


template <template <class T, SizeType> class CoreArray, SizeType size>
class TemplateSizedArrayWrapper
{
	public:
		template <class T>
		using Array = CoreArray<T, size>;
};


template <template <class T> class t_ArrayType, class T>
class ArrayPolicyInterface
{
	public:
		friend void swap(ArrayPolicyInterface & first, ArrayPolicyInterface & second) {
			using std::swap;

			swap(first.array_, second.array_);
		}

		using ArrayType        = t_ArrayType<T>;
		using ArrayReturn      = ArrayType       &;
		using ArrayConstReturn = ArrayType const &;

		template <class ... ArgTypes>
		constexpr ArrayPolicyInterface(ArgTypes ... args) :
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
