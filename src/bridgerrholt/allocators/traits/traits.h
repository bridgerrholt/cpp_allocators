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

template <class ArrayType>
class ArrayPolicyBase
{

};

		}
	}
}

#endif
