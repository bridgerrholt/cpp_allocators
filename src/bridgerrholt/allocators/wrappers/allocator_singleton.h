#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_WRAPPERS_ALLOCATOR_SINGLETON_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_WRAPPERS_ALLOCATOR_SINGLETON_H

#include "allocator_wrapper.h"
#include "../blocks/block.h"

namespace bridgerrholt {
	namespace allocators {

template <class Allocator>
class AllocatorSingleton : public AllocatorWrapper<Allocator>
{
	public:
		static AllocatorSingleton & get() {
			static AllocatorSingleton instance;
			return instance;
		}

		AllocatorSingleton(AllocatorSingleton const &) = delete;
		AllocatorSingleton(AllocatorSingleton      &&) = delete;

		AllocatorSingleton operator=(AllocatorSingleton) = delete;

	private:
		AllocatorSingleton() {}
};


template <class Allocator, class T>
class SmartBlockSingleton : public BasicBlock<T>
{
	public:
		SmartBlockSingleton(void * ptr, SizeType size) : BasicBlock<T>(ptr, size) {}
		SmartBlockSingleton(RawBlock block) :
			BasicBlock<T>(block.getPtr(), block.getSize()) {}

		~SmartBlockSingleton() {
			AllocatorSingleton<Allocator>::get().destruct(
				BasicBlock<T>{this->getPtr(), this->getSize()}
			);
		}
};


template <class AllocatorWrapperType, class BlockType, class T, class ... ArgTypes>
BlockType makeBlock(AllocatorWrapperType allocator, ArgTypes ... args) {
	return {allocator.template construct<T>(std::forward(args)...)};
};


	}
}


#endif
