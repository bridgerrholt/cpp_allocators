#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_STACK_ALLOCATOR_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_STACK_ALLOCATOR_H

#include "traits/traits.h"
#include "common/common_types.h"
#include "blocks/block.h"

namespace bridgerrholt {
	namespace allocators {

template <class t_Policy>
class BasicStackAllocator : private t_Policy
{
	public:
		using Policy = t_Policy;

		BasicStackAllocator() : BasicStackAllocator (Policy()) {}

		BasicStackAllocator(Policy policy) : Policy (std::move(policy)),
		                                     next_ {getBegin()} {}

		RawBlock allocate(SizeType size) {
			if (size == 0) size = 1;

			if (getEnd() - size >= next_) {
				RawBlock toReturn {next_, size};
				next_ += size;
				return toReturn;
			}
			else {
				return RawBlock::makeNullBlock();
			}
		}

		void deallocate(RawBlock block) {
			if (static_cast<ElementType*>(block.getPtr()) + block.getSize() == next_)
				deallocateTo(block);
		}

		void deallocateTo(RawBlock block) {
			next_ = static_cast<ElementType*>(block.getPtr());
		}

		void clear() {
			next_ = getBegin();
		}

		bool owns(RawBlock block) {
			return (block.getPtr() >= getBegin() && block.getPtr() < getEnd());
		}

		bool isEmpty() {
			return (next_ == getBegin());
		}

		bool isFull() {
			return (next_ == getEnd());
		}


	private:
		using ElementType = char;

		ElementType * getBegin() { return this->getArray().data(); }
		ElementType * getEnd()   { return getBegin() + this->getStackSize(); }

		ElementType * next_;
};



class StackAllocator
{
	public:
		template <template <class T> class CoreArray>
		class RuntimePolicy {
			public:
				using ArrayType = CoreArray<char>;

				using ArrayReturn      = ArrayType       &;
				using ArrayConstReturn = ArrayType const &;

				RuntimePolicy(SizeType stackSize) : array_ (stackSize) {}

				ArrayReturn      getArray()       { return array_; }
				ArrayConstReturn getArray() const { return array_; }

				SizeType getStackSize() const { return array_.size(); }

			private:
				ArrayType array_;
		};


		template <template <class T, SizeType size> class CoreArray,
			SizeType stackSize>
		class TemplatedPolicy {
			public:
				using ArrayType = CoreArray<char, stackSize>;

				using ArrayReturn      = ArrayType       &;
				using ArrayConstReturn = ArrayType const &;

				ArrayReturn      getArray()       { return array_; }
				ArrayConstReturn getArray() const { return array_; }

				static constexpr SizeType getStackSize() { return stackSize; }

			private:
				ArrayType array_;
		};


		template <template <class> class CoreArray>
		using Runtime = BasicStackAllocator<
			RuntimePolicy<CoreArray>
		>;


		template <template <class, SizeType> class CoreArray,
			SizeType stackSize>
		using Templated = BasicStackAllocator<
			TemplatedPolicy<CoreArray, stackSize>
		>;
};



	}
}

#endif
