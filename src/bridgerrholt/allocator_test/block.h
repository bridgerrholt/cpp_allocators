#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_BLOCK_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_BLOCK_H

#include <type_traits>

#include "common_types.h"

namespace bridgerrholt {
	namespace allocator_test {

class RawBlock
{
	public:
		friend void swap(RawBlock & first, RawBlock & second);

		/// Default constructor. Initializes ptr as nullptr.
		RawBlock();
		RawBlock(GenericPtr ptr, SizeType size);

		bool isNull() const { return (ptr_ == nullptr); }

		void const * getPtr()  const { return ptr_; }
		void       * getPtr()        { return ptr_; }

		SizeType     getSize() const { return size_; }


	private:
		GenericPtr ptr_;
		SizeType   size_;
};


template <class T>
class BasicBlock : public RawBlock
{
	public:
		using Type         = T;
		using Pointer      = Type       *;
		using ConstPointer = Type const *;

		friend void swap(BasicBlock & first, BasicBlock & second) {
			using std::swap;

			swap(*static_cast<RawBlock *>(&first),
			     *static_cast<RawBlock *>(&second));
		}

		BasicBlock() {}

		BasicBlock(void *  ptr, SizeType size) :
			RawBlock {ptr, size} {}

		BasicBlock(Pointer ptr, SizeType size) :
			RawBlock {static_cast<void *>(ptr), size} {}

		template <class C>
		BasicBlock(BasicBlock<C> block) :
			RawBlock {block.RawBlock::getPtr(), block.getSize()} {
			static_assert(std::is_base_of<Type, C>(),
			              "Cannot convert to non-base");
		}

		ConstPointer getPtr() const {
			return static_cast<ConstPointer>(this->RawBlock::getPtr());
		}

		Pointer getPtr() {
			return static_cast<Pointer>(RawBlock::getPtr());
		}
};





	}
}

#endif
