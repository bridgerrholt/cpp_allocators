#ifndef BRH_CPP_ALLOCATORS_BRIDGERRHOLT_ALLOCATORS_TRAITS_ARRAY_INTERFACE_H
#define BRH_CPP_ALLOCATORS_BRIDGERRHOLT_ALLOCATORS_TRAITS_ARRAY_INTERFACE_H

#include <vector>

namespace bridgerrholt {
	namespace allocators {
		namespace traits {


class VectorInterface
{
	public:
		template <class T>
		using Runtime = std::vector<T>;

		template <class T, std::size_t size>
		class Templated : public Runtime<T> {
			public:
				Templated() { Runtime<T>::reserve(size); }
		};
};


		}
	}
}

#endif
