#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_COMMON_FREE_LIST_NODE_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_COMMON_FREE_LIST_NODE_H

namespace bridgerrholt {
	namespace allocator_test {
		namespace allocators {
			namespace common {

class FreeListNode {
	public:
		using Pointer = FreeListNode *;

		constexpr FreeListNode() : nextPtr_ {nullptr} {}
		constexpr FreeListNode(Pointer nextPtr) : nextPtr_{nextPtr} {}
		constexpr FreeListNode(void  * nextPtr) :
			FreeListNode(static_cast<FreeListNode>(nextPtr)) {}

		FreeListNode * getNextPtr() const { return nextPtr_; }
		void setNextPtr(FreeListNode * next) { nextPtr_ = next; }

		FreeListNode       & getNext()       { return *nextPtr_; }
		FreeListNode const & getNext() const { return *nextPtr_; }

		void advance() {
			setNextPtr(getNext().getNextPtr());
		}

		bool hasNext() const { return (nextPtr_ != nullptr); }


	private:
		FreeListNode * nextPtr_;
};


			}
		}
	}
}

#endif
