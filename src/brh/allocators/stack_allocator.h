#ifndef BRH_CPP_ALLOCATORS_SRC_BRH_ALLOCATORS_STACK_ALLOCATOR_H
#define BRH_CPP_ALLOCATORS_SRC_BRH_ALLOCATORS_STACK_ALLOCATOR_H

#include <supports/calc_is_aligned.h>

#include "blocks/block.h"
#include "common/common_types.h"
#include "traits/traits.h"

namespace brh {
	namespace allocators {
		namespace stack_allocator {

template <class t_Policy>
class Allocator : private t_Policy
{
	public:
		using Policy = t_Policy;
		using Handle = RawBlock;

		Allocator() : Allocator (Policy()) {}

		Allocator(Policy policy) : Policy (std::move(policy)),
		                           next_  {getBegin()} {}

		/// Does nothing but turn it into 1 if it's 0.
		static constexpr SizeType calcRequiredSize(SizeType desiredSize) {
			if (desiredSize == 0)
				return 1;
			else
				return desiredSize;
		}

		constexpr SizeType getStorageSize() const {
			return Policy::getStackSize();
		}

		/// Allocates the next blocks in the container.
		Handle allocate(SizeType size) {
			size = calcRequiredSize(size);

			if (next_ <= getEnd() - size) {
				auto ptr = next_;
				next_ += size;
				return {ptr, size};
			}
			else {
				return Handle::makeNullBlock();
			}
		}

		Handle allocateAligned(SizeType size, SizeType alignment) {
			if (supports::calcIsAligned(next_, alignment))
				return allocate(size);

			else
				return Handle::makeNullBlock();
		}

		/// Effectively deallocates and then allocates the whole container.
		Handle allocateAll() {
			next_ = getEnd();
			return {getBegin(), getEnd() - getBegin()};
		}

		/// Allocates everything after the top of the stack.
		Handle allocateRemaining() {
			if (!isFull()) {
				auto ptr = next_;
				next_ = getEnd();
				return {ptr, getEnd() - ptr};
			}

			return Handle::makeNullBlock();
		}

		constexpr void deallocate(NullBlock) const {}

		/// Deallocates only if the block is on top.
		void deallocate(Handle block) {
			if (!block.isNull()) {

				if (isTop(block))
					deallocateTo(block.getPtr());
			}
		}

		/// Deallocates all memory past the block.
		void deallocateTo(Handle block) {
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
		bool reallocate(Handle & block, SizeType newSize) {
			newSize = calcRequiredSize(newSize);
			auto const blockSize = block.getSize();

			if (isTop(block)) {
				if (newSize < blockSize) {
					next_ -= blockSize - newSize;
					block.setSize(newSize);
					return true;
				}

				else if (blockSize > newSize){
					auto difference = newSize - blockSize;
					return expandTop(block, difference);
				}
			}

			if (newSize == blockSize)
				return true;

			return false;
		}

		/// Only works if the block is on top or the new size is 0.
		/// Also fails if the container can't hold the extra amount.
		bool expand(Handle & block, SizeType amount) {
			if (isTop(block)) {
				if (expandTop(block, amount))
					return true;
			}
			else if (amount == 0)
				return true;

			return false;
		}

		/// Checks if the block points to within the container.
		bool owns(Handle block) const {
			return (getBegin() <= block.getPtr() && block.getPtr() < getEnd());
		}

		bool isEmpty() const {
			return (next_ == getBegin());
		}

		bool isFull() const {
			return (next_ == getEnd());
		}

		/// Checks if a block is at the back of the container.
		bool isTop(Handle block) const {
			auto blockEnd =
				static_cast<ElementPtr>(block.getPtr()) + block.getSize();

			return (blockEnd == next_);
		}

		/// Calculates the remaining space in the container.
		SizeType calcUnoccupied() const {
			return (getEnd() - next_);
		}

		SizeType calcOccupied() const {
			return (next_ - getBegin());
		}


	private:
		using ElementType      = char;
		using ElementPtr       = ElementType       *;
		using ElementConstPtr  = ElementType const *;

		bool expandTop(Handle & block, SizeType amount) {
			if (calcOccupied() >= amount) {
				block.setSize(block.getSize() + amount);
				next_ += amount;

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
	public traits::ArrayPolicyInterface<CoreArray, char> {

	public:
		using ArrayType = CoreArray<char>;

		using ArrayReturn      = ArrayType       &;
		using ArrayConstReturn = ArrayType const &;

		RuntimePolicy(SizeType stackSize) : BaseType (stackSize) {}

		SizeType getStackSize() const { return BaseType::getArray().size(); }

	private:
		using BaseType = traits::ArrayPolicyInterface<CoreArray, char>;
};


template <template <class T, SizeType size> class CoreArray,
	SizeType stackSize>
class TemplatedPolicy :
	public traits::ArrayPolicyInterface<
		traits::TemplateSizedArrayWrapper<
			CoreArray, stackSize
		>::template Array, char> {

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

		} // stack_allocator



/// Allocates by pushing to a stack and deallocates by popping from the stack.
class StackAllocator {
	public:
		template <class Policy>
		using Allocator = stack_allocator::Allocator<Policy>;


		template <template <class T> class CoreArray>
		using RuntimePolicy = stack_allocator::RuntimePolicy<CoreArray>;

		template <template <class T, SizeType size> class CoreArray,
			SizeType stackSize>
		using TemplatedPolicy =
			stack_allocator::TemplatedPolicy<CoreArray, stackSize>;


		template <template <class> class CoreArray>
		using Runtime = stack_allocator::Runtime<CoreArray>;

		template <template <class T, SizeType size> class CoreArray,
			SizeType stackSize>
		using Templated = stack_allocator::Templated<CoreArray, stackSize>;
};



	}
}

#endif
