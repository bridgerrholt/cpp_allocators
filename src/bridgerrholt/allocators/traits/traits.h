#ifndef BRH_CPP_ALLOCATORS_BRIDGERRHOLT_ALLOCATORS_TRAITS_H
#define BRH_CPP_ALLOCATORS_BRIDGERRHOLT_ALLOCATORS_TRAITS_H

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
		ArrayPolicyBase(ArgTypes ... args) :
			array_ (std::forward<ArgTypes>(args)...) {
			std::cout << ' ' << array_.data() << ' ' << array_.data() + array_.size() << '\n';
		}

		ArrayReturn      getArray()       { return array_; }
		ArrayConstReturn getArray() const { return array_; }

	private:
		ArrayType array_;
};

		}
	}
}

#endif
