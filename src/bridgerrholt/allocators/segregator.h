#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_SEGREGATOR_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_SEGREGATOR_H

#include "common/common_types.h"
#include "blocks/block.h"

namespace bridgerrholt {
	namespace allocators {

template <class t_Policy,
	        class SmallAllocator,
		      class LargeAllocator>
class BasicSegregator : private t_Policy,
                        private SmallAllocator,
                        private LargeAllocator
{
	public:
		using Policy = t_Policy;

		BasicSegregator() {}

		BasicSegregator(SmallAllocator small, LargeAllocator large) :
			SmallAllocator(small),
			LargeAllocator(large) {}

		RawBlock allocate(SizeType size) {
			if (size <= Policy::getThreshold()) {
				return SmallAllocator::allocate(size);
			}
			else {
				return LargeAllocator::allocate(size);
			}
		}

		constexpr void deallocate(NullBlock) {}

		void deallocate(RawBlock block) {
			if (block.getSize() <= Policy::getThreshold()) {
				SmallAllocator::deallocate(block);
			}

			else {
				LargeAllocator::deallocate(block);
			}
		}

		bool owns(RawBlock block) {
			return (SmallAllocator::owns(block) || LargeAllocator::owns(block));
		}
};



class Segregator
{
	public:
		class RuntimePolicy {
			public:
				RuntimePolicy(SizeType threshold) : threshold_ {threshold} {}

			private:
				SizeType threshold_;
		};

		template <SizeType threshold>
		class TemplatedPolicy {
			public:
				static constexpr SizeType getThreshold() { return threshold; }
		};

		template <class SmallAllocator, class LargeAllocator>
		using Runtime =
			BasicSegregator<RuntimePolicy, SmallAllocator, LargeAllocator>;

		template <class SmallAllocator, class LargeAllocator, SizeType threshold>
		using Templated =
			BasicSegregator<
				TemplatedPolicy<threshold>, SmallAllocator, LargeAllocator>;

};



template <SizeType maxSize, class Allocator>
class MaximumSizeAllocator : private Allocator
{
	public:
		MaximumSizeAllocator() {}

		MaximumSizeAllocator(Allocator allocator) : Allocator {allocator} {}

		RawBlock allocate(SizeType size) {
			if (size > maxSize)
				return {nullptr, 0};
			else
				return Allocator::allocate(size);
		}

		void deallocate(RawBlock block) {
			if (block.getSize() <= maxSize)
				Allocator::deallocate(block);
		}

		bool owns(RawBlock block) {
			return (Allocator::owns(block));
		}
};


	}
}

#endif
