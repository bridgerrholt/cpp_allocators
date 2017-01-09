#include "block.h"

namespace bridgerrholt {
	namespace allocator_test {

void swap(RawBlock & first, RawBlock & second) {
	using std::swap;

	swap(first.ptr_,  second.ptr_);
	swap(first.size_, second.size_);
}



RawBlock::RawBlock(GenericPtr ptr, SizeType size) :
	ptr_ {ptr}, size_ {size}
{

}


RawBlock::RawBlock() :
	ptr_ {nullptr}, size_ {0}
{

}


	}
}