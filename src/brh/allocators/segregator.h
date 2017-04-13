#ifndef BRH_CPP_ALLOCATORS_SRC_BRH_ALLOCATORS_SEGREGATOR_H
#define BRH_CPP_ALLOCATORS_SRC_BRH_ALLOCATORS_SEGREGATOR_H

#include "common/common_types.h"
#include "blocks/block.h"

namespace brh {
	namespace allocators {
		namespace segregator {

/// SmallAllocator must not allocate blocks larger than the threshold if
/// the passed size is less than or equal to the threshold.
template <class t_Policy,
	        class SmallAllocator,
		      class LargeAllocator>
class Allocator : private t_Policy,
                  private SmallAllocator,
                  private LargeAllocator {
	public:
		using Policy = t_Policy;

		constexpr Allocator() : Allocator(SmallAllocator(),
		                                  LargeAllocator()) {}

		constexpr Allocator(SmallAllocator small,
		                    LargeAllocator large) :
			SmallAllocator(std::move(small)),
			LargeAllocator(std::move(large)) {}

		SizeType calcRequiredSize(SizeType desiredSize) const {
			if (belongsToSmall(desiredSize))
				return SmallAllocator::calcRequiredSize(desiredSize);
			else
				return LargeAllocator::calcRequiredSize(desiredSize);
		}

		RawBlock allocate(SizeType size) {
			if (belongsToSmall(size)) {
				return SmallAllocator::allocate(size);
			}
			else {
				return LargeAllocator::allocate(size);
			}
		}

		constexpr void deallocate(NullBlock) const {}

		void deallocate(RawBlock block) {
			if (belongsToSmall(block.getSize())) {
				SmallAllocator::deallocate(block);
			}

			else {
				LargeAllocator::deallocate(block);
			}
		}

		bool reallocate(RawBlock & block, SizeType size) {
			auto blockSize = block.getSize();

			if (blockSize == size)
				return true;

			if (belongsToSmall(blockSize)) {
				if (belongsToSmall(size))
					return SmallAllocator::reallocate(block, size);
				else {
					assert(blockSize < size);
					reallocateAcrossAllocators<SmallAllocator, LargeAllocator>(
						block, size, blockSize
					);

					return true;
				}
			}

			else {
				if (belongsToLarge(size))
					return LargeAllocator::reallocate(block, size);
				else {
					assert(size < blockSize);
					reallocateAcrossAllocators<LargeAllocator, SmallAllocator>(
						block, size, size
					);

					return true;
				}
			}
		}

		bool expand(RawBlock & block, SizeType amount) {
			auto blockSize = block.getSize();
			auto newBlockSize = blockSize + amount;

			if (blockSize == newBlockSize)
				return true;

			if (belongsToSmall(blockSize)) {
				if (belongsToSmall(newBlockSize))
					return SmallAllocator::expand(block, amount);
				else
					return false;
			}

			else {
				if (belongsToLarge(newBlockSize))
					return LargeAllocator::expand(block, amount);
				else
					return false;
			}
		}

		bool owns(RawBlock block) const {
			return (SmallAllocator::owns(block) ||
				      LargeAllocator::owns(block));
		}

		bool isEmpty() const {
			return (SmallAllocator::isEmpty() &&
			        LargeAllocator::isEmpty());
		}

		bool isFull() const {
			return (SmallAllocator::isFull() &&
			        LargeAllocator::isFull());
		}

		SizeType calcUnoccupied() const {
			return (SmallAllocator::calcUnoccupied() +
			        LargeAllocator::calcUnoccupied());
		}

		SizeType calcOccupied() const {
			return (SmallAllocator::calcOccupied() +
			        LargeAllocator::calcOccupied());
		}

		bool belongsToSmall(SizeType size) const {
			return (size <= Policy::getThreshold());
		}

		bool belongsToLarge(SizeType size) const {
			return !(belongsToSmall(size));
		}

		SmallAllocator & getSmall() {
			return static_cast<SmallAllocator&>(*this);
		}

		SmallAllocator const & getSmall() const {
			return static_cast<SmallAllocator const &>(*this);
		}


		LargeAllocator & getLarge() {
			return static_cast<LargeAllocator&>(*this);
		}

		LargeAllocator const & getLarge() const {
			return static_cast<LargeAllocator const &>(*this);
		}


	private:
		template <class OldOwner, class NewOwner>
		void reallocateAcrossAllocators(RawBlock & block, SizeType size, SizeType copySize) {
			auto newBlock = NewOwner::allocate(size);
			std::memcpy(newBlock.getPtr(), block.getPtr(), copySize);
			OldOwner::deallocate(block);
			block = newBlock;
		}
};


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
	Allocator<RuntimePolicy, SmallAllocator, LargeAllocator>;

template <class SmallAllocator, class LargeAllocator, SizeType threshold>
using Templated =
	Allocator<
		TemplatedPolicy<threshold>, SmallAllocator, LargeAllocator>;


		} // segregator



/// Chooses between two different allocators based on whether the
/// requested size is above the threshold or not.
class Segregator {
	public:
		template <class Policy, class SmallAllocator, class LargeAllocator>
		using Allocator = segregator::Allocator<
			Policy, SmallAllocator, LargeAllocator>;


		using RuntimePolicy = segregator::RuntimePolicy;

		template <SizeType threshold>
		using TemplatedPolicy = segregator::TemplatedPolicy<threshold>;


		template <class SmallAllocator, class LargeAllocator>
		using Runtime = segregator::Runtime<SmallAllocator, LargeAllocator>;

		template <class SmallAllocator, class LargeAllocator, SizeType threshold>
		using Templated = segregator::Templated<
			SmallAllocator, LargeAllocator, threshold>;

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
