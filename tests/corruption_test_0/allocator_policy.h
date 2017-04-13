#ifndef BRH_CPP_ALLOCATORS_CORRUPTION_TEST_0_ALLOCATOR_POLICY_H
#define BRH_CPP_ALLOCATORS_CORRUPTION_TEST_0_ALLOCATOR_POLICY_H

#include <cstddef>
#include <string>

#include <allocators/blocks/block.h>

namespace brh {
	namespace allocators {
		namespace tests {

class AllocatorPolicy
{
	public:
		virtual ~AllocatorPolicy() {}

		virtual SizeType calcRequiredSize(SizeType desiredSize) const = 0;
		virtual SizeType getStorageSize() const = 0;

		virtual RawBlock allocate  (SizeType size) = 0;
		virtual void     deallocate(RawBlock block) = 0;

		virtual bool
		reallocate(RawBlock & block, SizeType size) { return false; }

		virtual bool
		expand(RawBlock & block, SizeType amount) { return false; }
};


template <class Allocator>
class BasicAllocatorPolicy : public AllocatorPolicy
{
	public:
		BasicAllocatorPolicy() {}

		template <class ... ArgTypes>
		BasicAllocatorPolicy(ArgTypes ... args) :
			allocator_(std::forward(args)...) {}

		~BasicAllocatorPolicy() {}

		SizeType calcRequiredSize(SizeType desiredSize) const override {
			return allocator_.calcRequiredSize(desiredSize);
		}

		SizeType getStorageSize() const override {
			return allocator_.getStorageSize();
		}

		RawBlock allocate(SizeType size) override {
			return allocator_.allocate(size);
		}

		void deallocate(RawBlock block) override {
			return allocator_.deallocate(block);
		}

		bool reallocate(RawBlock & block, SizeType size) override {
			return allocator_.reallocate(block, size);
		}

		bool expand(RawBlock & block, SizeType amount) override {
			return allocator_.expand(block, amount);
		}


	private:
		Allocator allocator_;
};

		}
	}
}

#endif
