#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_COMMON_FREE_LIST_NODE_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_COMMON_FREE_LIST_NODE_H

namespace bridgerrholt {
	namespace allocators {
		namespace common {

/// A simple POD node for singly-linked free lists. Should not be
/// used directly, use through @ref FreeListNodeView instead.
class FreeListNode {
	public:
		FreeListNode * getNextPtr() const { return nextPtr_; }
		void setNextPtr(FreeListNode * next) { nextPtr_ = next; }

	private:
		FreeListNode * nextPtr_;
};


/// Acts like an iterator through a linked list of @ref FreeListNode objects.
class FreeListNodeView
{
	public:
		constexpr FreeListNodeView() : FreeListNodeView(nullptr) {}

		constexpr FreeListNodeView(std::nullptr_t) :
			FreeListNodeView(static_cast<void*>(nullptr)) {}

		constexpr FreeListNodeView(void * node) :
			FreeListNodeView(static_cast<FreeListNode*>(node)) {}

		constexpr FreeListNodeView(FreeListNode * node) : node_ {node} {}


		void setNextPtr(FreeListNode * next) { node_->setNextPtr(next); }
		void setNextPtr(std::nullptr_t) {
			setNextPtr(static_cast<void*>(nullptr));
		}
		void setNextPtr(void * next) {
			setNextPtr(static_cast<FreeListNode*>(next));
		}

		FreeListNode       * getNextPtr()       { return node_->getNextPtr(); }
		FreeListNode const * getNextPtr() const { return node_->getNextPtr(); }

		FreeListNode       * getNodePtr()       { return node_; }
		FreeListNode const * getNodePtr() const { return node_; }

		void setNodePtr(FreeListNode * node) { node_ = node; }

		void advance() { node_ = node_->getNextPtr(); }

		bool hasNext() const { return (getNextPtr() != nullptr); }
		bool hasNode() const { return (node_        != nullptr); }


	private:
		FreeListNode * node_;
};


		}
	}
}

#endif
