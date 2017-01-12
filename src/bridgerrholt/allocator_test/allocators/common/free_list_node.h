#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_COMMON_FREE_LIST_NODE_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_COMMON_FREE_LIST_NODE_H

namespace bridgerrholt {
	namespace allocator_test {
		namespace allocators {
			namespace common {

class FreeListNode {
	public:
		FreeListNode * getNextPtr() const { return nextPtr; }
		void setNextPtr(FreeListNode * next) { nextPtr = next; }

		FreeListNode * nextPtr;
};


class FreeListNodeView
{
	public:
		FreeListNodeView() : FreeListNodeView(nullptr) {}
		FreeListNodeView(void         * node) :
			FreeListNodeView(static_cast<FreeListNode*>(node)) {}
		FreeListNodeView(FreeListNode * node) : node_ {node} {}

		FreeListNode * getNextPtr() const { return node_->getNextPtr(); }

		void setNextPtr(FreeListNode * next) { node_->setNextPtr(next); }
		void setNextPtr(void         * next) {
			setNextPtr(static_cast<FreeListNode*>(next));
		}

		FreeListNode       & getNext()       { return *getNextPtr(); }
		FreeListNode const & getNext() const { return *getNextPtr(); }

		FreeListNode * getNodePtr() { return node_; }

		void advance() {
			setNextPtr(getNext().getNextPtr());
		}

		bool hasNext() const { return (getNextPtr() != nullptr); }

	private:
		FreeListNode * node_;
};


			}
		}
	}
}

#endif
