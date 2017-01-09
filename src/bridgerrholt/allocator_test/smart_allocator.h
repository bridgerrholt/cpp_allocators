#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_SMART_ALLOCATOR_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_SMART_ALLOCATOR_H

#include <memory>

#include "allocator_wrapper.h"
#include "unique_block.h"

namespace bridgerrholt {
	namespace allocator_test {

template <class Allocator>
class SmartAllocator : public AllocatorWrapper<Allocator>
{
	public:
		using BaseAllocator = AllocatorWrapper<Allocator>;

		template <class ... ArgTypes>
		SmartAllocator(ArgTypes ... args) :
			AllocatorWrapper<Allocator>(std::forward(args)...) {}

		template <class T, class ... ArgTypes>
		UniqueBlock<T, BaseAllocator>
		constructUnique(ArgTypes ... args) {
			return constructAs<T, UniqueBlockHelper>(std::forward<ArgTypes>(args)...);
		}

		/*template <class T, class ... ArgTypes>
		SharedBlock<T, BaseAllocator>
		constructShared(ArgTypes ... args) {
			return constructAs<T, std::shared_ptr>(std::forward<ArgTypes>(args)...);
		}*/

		template <class T, template <class U> class Container, class ... ArgTypes>
		Container<T>
		constructAs(ArgTypes ... args) {
			return { BaseAllocator::template construct<T>(
				std::forward<ArgTypes>(args) ...
			), *this };
		};


	private:
		template <class T>
		using UniqueBlockHelper = UniqueBlock<T, BaseAllocator>;

		template <class T>
		using SharedBlockHelper = UniqueBlock<T, BaseAllocator>;
};


	}
}

#endif
