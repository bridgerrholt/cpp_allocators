#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_BLOCK_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_BLOCK_H

#include <type_traits>

#include "common_types.h"

namespace bridgerrholt {
	namespace allocator_test {

class RawBlock
{
	public:
		RawBlock(GenericPtr ptr, SizeType size);

		bool isNull() const { return (ptr_ == nullptr); }

		ConstGenericPtr getPtr()  const { return ptr_; }
		GenericPtr      getPtr()        { return ptr_; }

		SizeType        getSize() const { return size_; }


	private:
		GenericPtr  ptr_;
		SizeType size_;
};


template <class T>
class BasicBlock : public RawBlock
{
	public:
		using Type         = T;
		using Pointer      = Type       *;
		using ConstPointer = Type const *;

		BasicBlock(GenericPtr ptr, SizeType size) : RawBlock {ptr, size} {}
		BasicBlock(Pointer    ptr, SizeType size) : RawBlock { reinterpret_cast<GenericPtr>(ptr), size} {}

		template <class C>
		BasicBlock(BasicBlock<C> block) :
			RawBlock {block.RawBlock::getPtr(), block.getSize()} {
			static_assert(std::is_base_of<Type, C>(), "Cannot convert to non-base");
		}

		ConstPointer getPtr() const {
			return reinterpret_cast<ConstPointer>(this->RawBlock::getPtr());
		}

		Pointer getPtr() { return reinterpret_cast<Pointer>(RawBlock::getPtr()); }

};


	}
}

#endif
