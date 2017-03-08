#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_SIZE_FAIL_ALLOCATOR_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_SIZE_FAIL_ALLOCATOR_H

#include <functional>

#include "common/common_types.h"
#include "blocks/block.h"

namespace bridgerrholt {
	namespace allocators {
		namespace size_fail_allocator {

// bool Comparator(SizeType threshold, SizeType size)
using Comparator = std::function<bool(SizeType, SizeType)>;


template <class t_Policy>
class Allocator : private t_Policy
{
	public:
		using Policy = t_Policy;

		Allocator() {}

		Allocator(Policy policy) : Policy (std::move(policy)) {}

		RawBlock allocate(SizeType size) {
			if (Policy::passes(size))
				return Policy::Allocator::allocate(size);
			else
				return RawBlock::makeNullBlock();
		}

		void deallocate(RawBlock block) {
			Policy::Allocator::deallocate(block);
		}

		bool owns(RawBlock block) {
			return Policy::Allocator::owns(block);
		}
};


template <class t_Allocator, Comparator comparator, SizeType threshold>
class TemplatedPolicy : private t_Allocator
{
	public:
		using Allocator = t_Allocator;

		TemplatedPolicy() {}

		TemplatedPolicy(Allocator allocator) : Allocator (std::move(allocator)) {}

		constexpr bool passes(SizeType size) const {
			return comparator(threshold, size);
		}
};


template <class t_Allocator>
class RuntimePolicy : private t_Allocator
{
	public:
		using Allocator = t_Allocator;

		RuntimePolicy(Comparator comparator, SizeType threshold) :
			comparator_ (comparator),
			threshold_  {threshold} {}

		RuntimePolicy(Comparator comparator,
		              SizeType   threshold,
		              Allocator  allocator) :
			RuntimePolicy(comparator, threshold),
			Allocator    (std::move(allocator)) {}


		constexpr bool passes(SizeType size) const {
			return comparator_(threshold_, size);
		}

		void setComparator(Comparator comparator) { comparator_ = comparator; }
		void setThreshold (SizeType   threshold)  { threshold_  = threshold; }

		Comparator getComparator() const { return comparator_; }
		SizeType   getThreshold () const { return threshold_; }


	private:
		Comparator comparator_;
		SizeType   threshold_;

};


		} // size_fail_allocator



template <SizeType maxSize, class Allocator>
class SizeFailAllocator : private Allocator
{
	public:
		SizeFailAllocator() {}

		SizeFailAllocator(Allocator allocator) : Allocator {allocator} {}

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
