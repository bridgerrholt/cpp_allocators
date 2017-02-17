#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_STACK_ALLOCATOR_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_STACK_ALLOCATOR_H

#include "traits/traits.h"
#include "common/common_types.h"
#include "blocks/block.h"

namespace bridgerrholt {
	namespace allocators {

/// Allocates by pushing to a stack and deallocates by popping from the stack.
		class StackAllocator {

public:
	template <class t_Policy>
	class Allocator : private t_Policy
	{
		public:
			using Policy = t_Policy;

			Allocator() : Allocator (Policy()) {}

			Allocator(Policy policy) : Policy (std::move(policy)),
			                           next_  {getBegin()} {}

			/// Does nothing but turn it into 1 if it's 0.
			static constexpr SizeType calcNeededSize(SizeType desiredSize) {
				if (desiredSize == 0)
					return 1;
				else
					return desiredSize;
			}

			/// Allocates the next blocks in the container.
			RawBlock allocate(SizeType size) {
				size = calcNeededSize(size);

				if (next_ <= getEnd() - size) {
					auto ptr = next_;
					next_ += size;
					return {ptr, size};
				}
				else {
					return RawBlock::makeNullBlock();
				}
			}

			/// Effectively deallocates and then allocates the whole container.
			RawBlock allocateAll() {
				next_ = getEnd();
				return {getBegin(), getEnd() - getBegin()};
			}

			/// Allocates everything after the top of the stack.
			RawBlock allocateRemaining() {
				if (!isFull()) {
					auto ptr = next_;
					next_ = getEnd();
					return {ptr, getEnd() - ptr};
				}

				return RawBlock::makeNullBlock();
			}

			constexpr void deallocate(NullBlock) {}

			/// Deallocates only if the block is on top.
			void deallocate(RawBlock block) {
				if (!block.isNull()) {

					if (isTop(block))
						deallocateTo(block.getPtr());
				}
			}

			/// Deallocates all memory past the block.
			void deallocateTo(RawBlock block) {
				deallocateTo(block.getPtr());
			}

			/// Deallocates all memory past the pointer.
			void deallocateTo(void * ptr) {
				next_ = static_cast<ElementPtr>(ptr);
			}

			/// Resets the stack.
			void deallocateAll() {
				next_ = getBegin();
			}

			/// Only works if the block is on top or the new size is
			/// equal to the block's size. Also fails if the container can't
			/// hold the new size.
			bool reallocate(RawBlock & block, SizeType newSize) {
				newSize = calcNeededSize(newSize);

				if (isTop(block)) {
					if (newSize < block.getSize()) {
						next_ -= block.getSize() - newSize;
						block.getSize() = newSize;
						return true;
					}

					else if (block.getSize() > newSize){
						auto difference = newSize - block.getSize();
						return expandTop(block, difference);
					}
				}

				if (newSize == block.getSize())
					return true;

				return false;
			}

			/// Only works if the block is on top or the new size is 0.
			/// Also fails if the container can't hold the extra amount.
			bool expand(RawBlock & block, SizeType amount) {
				if (isTop(block)) {
					if (expandTop(block, amount))
						return true;
				}
				else if (amount == 0)
					return true;

				return false;
			}

			/// Checks if the block points to within the container.
			bool owns(RawBlock block) const {
				return (getBegin() <= block.getPtr() && block.getPtr() < getEnd());
			}

			bool isEmpty() const {
				return (next_ == getBegin());
			}

			bool isFull() const {
				return (next_ == getEnd());
			}

			/// Checks if a block is at the back of the container.
			bool isTop(RawBlock block) const {
				auto blockEnd =
					static_cast<ElementPtr>(block.getPtr()) + block.getSize();

				return (blockEnd == next_);
			}

			/// Calculates the remaining space in the container.
			SizeType calcRemaining() const {
				return getEnd() - next_;
			}


		private:
			using ElementType      = char;
			using ElementPtr       = ElementType       *;
			using ElementConstPtr  = ElementType const *;

			bool expandTop(RawBlock & block, SizeType amount) {
				if (calcRemaining() >= amount) {
					block.getSize() += amount;
					next_           += amount;

					return true;
				}

				return false;
			}

			ElementPtr      getBegin()       { return Policy::getArray().data(); }
			ElementConstPtr getBegin() const { return Policy::getArray().data(); }

			ElementPtr getEnd() {
				return getBegin() + Policy::getStackSize();
			}

			ElementConstPtr getEnd() const {
				return getBegin() + Policy::getStackSize();
			}

			ElementPtr next_;
	};


	template <template <class T> class CoreArray>
	class RuntimePolicy :
		public traits::ArrayPolicyBase<CoreArray<char> > {

		public:
			using ArrayType = CoreArray<char>;

			using ArrayReturn      = ArrayType       &;
			using ArrayConstReturn = ArrayType const &;

			RuntimePolicy(SizeType stackSize) : BaseType (stackSize) {}

			SizeType getStackSize() const { return BaseType::getArray().size(); }

		private:
			using BaseType = traits::ArrayPolicyBase<CoreArray<char> >;
	};


	template <template <class T, SizeType size> class CoreArray,
		SizeType stackSize>
	class TemplatedPolicy :
		public traits::ArrayPolicyBase<CoreArray<char, stackSize> > {

		public:
			static constexpr SizeType getStackSize() { return stackSize; }
	};


	template <template <class> class CoreArray>
	using Runtime = Allocator<
		RuntimePolicy<CoreArray>
	>;


	template <template <class, SizeType> class CoreArray,
		SizeType stackSize>
	using Templated = Allocator<
		TemplatedPolicy<CoreArray, stackSize>
	>;

		};

	}
}

#endif
