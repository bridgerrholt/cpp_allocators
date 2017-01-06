#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_BLOCK_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_BLOCK_H

#include "common_types.h"

namespace bridgerrholt {
	namespace allocator_test {

class Block
{
	public:
		Block(GenericPtr ptr, SizeType size);

		bool isNull() const { return (ptr_ == nullptr); }

		ConstGenericPtr getPtr()  const { return ptr_; }
		GenericPtr      getPtr()        { return ptr_; }

		SizeType        getSize() const { return size_; }


	private:
		GenericPtr  ptr_;
		SizeType size_;
};


template <class T>
class BasicBlock : public Block
{
	public:
		using Pointer      = T       *;
		using ConstPointer = T const *;

		BasicBlock(GenericPtr ptr, SizeType size) : Block {ptr, size} {}

		ConstPointer getPtr() const {
			return reinterpret_cast<ConstPointer>(this->Block::getPtr());
		}

		Pointer getPtr() { return reinterpret_cast<Pointer>(Block::getPtr()); }

};


	}
}

#endif
