#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_STACK_ALLOCATOR_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_STACK_ALLOCATOR_H

#include "../common_types.h"

#include "../block.h"

namespace bridgerrholt {
	namespace allocator_test {

template <template <class T, SizeType size> class Array,
  SizeType stackSize>
class StackAllocator
{
	public:
		StackAllocator() : next_ {getBegin()} {}

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
			return (block.getPtr() >= getBegin && block.getPtr() < getEnd());
		}

		bool isEmpty() {
			return (next_ == getBegin());
		}

		bool isFull() {
			return (next_ == getEnd());
		}


	private:
		using ElementType = char;
		using ArrayType   = Array<ElementType, stackSize>;

		ElementType * getBegin() { return array_.data(); }
		ElementType * getEnd()   { return getBegin() + stackSize; }

		ArrayType     array_;
		ElementType * next_;
};

	}
}

#endif
