#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATOR_SINGLETON_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATOR_SINGLETON_H

#include "allocator_wrapper.h"
#include "block.h"

namespace bridgerrholt {
	namespace allocator_test {

template <class Allocator>
class AllocatorSingleton : public AllocatorWrapper<Allocator>
{
	public:
		static AllocatorSingleton & get() {
			static AllocatorSingleton instance;
			return instance;
		}

	private:
		AllocatorSingleton() {}
};


template <class Allocator, class T>
class SmartBlockSingleton : public BasicBlock<T>
{
	public:
		SmartBlockSingleton(GenericPtr ptr, SizeType size) : BasicBlock {ptr, size} {}

		~SmartBlockSingleton() {
			AllocatorSingleton<Allocator>::get().destruct({getPtr(), getSize()});
		}
};


	}
}


#endif
