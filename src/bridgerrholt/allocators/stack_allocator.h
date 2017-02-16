#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_STACK_ALLOCATOR_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_STACK_ALLOCATOR_H

#include "traits/traits.h"
#include "common/common_types.h"
#include "blocks/block.h"

namespace bridgerrholt {
	namespace allocators {


/// Allocates by pushing to a stack and deallocates by popping from the stack.
		class StackAllocator {

public:
	template <class t_Policy>
	class Allocator : private t_Policy
	{
		public:
			using Policy = t_Policy;

			Allocator() : Allocator (Policy()) {}

			Allocator(Policy policy) : Policy (std::move(policy)),
			                           next_  {getBegin()} {}

			static constexpr SizeType calcNeededSize(SizeType desiredSize) {
				if (desiredSize == 0)
					return 1;
				else
					return desiredSize;
			}

			RawBlock allocate(SizeType size) {
				size = calcNeededSize(size);

				if (next_ <= getEnd() - size) {
					auto ptr {next_};
					next_ += size;
					return {ptr, size};
				}
				else {
					return RawBlock::makeNullBlock();
				}
			}

			RawBlock allocateAll() {
				next_ = getEnd();
				return {getBegin(), getEnd() - getBegin()};
			}

			RawBlock allocateRemaining() {
				if (!isFull()) {
					auto ptr {next_};
					next_ = getEnd();
					return {ptr, getEnd() - ptr};
				}

				return RawBlock::makeNullBlock();
			}

			constexpr void deallocate(NullBlock) {}

			void deallocate(RawBlock block) {
				if (!block.isNull()) {

					auto blockEnd {
						static_cast<ElementPtr>(block.getPtr()) + block.getSize()
					};

					if (blockEnd == next_)
						deallocateTo(block.getPtr());
				}
			}

			void deallocateTo(RawBlock block) {
				deallocateTo(block.getPtr());
			}

			void deallocateTo(void * ptr) {
				next_ = static_cast<ElementPtr>(ptr);
			}

			void deallocateAll() {
				next_ = getBegin();
			}

			bool expand(RawBlock & block, SizeType amount) {
				ElementPtr ptr {static_cast<ElementPtr>(block.getPtr())};

				if (isTop(block)) {
					if (calcRemaining() >= amount) {
						block.getSize() += amount;
						next_           += amount;

						return true;
					}
				}

				return false;
			}

			bool owns(RawBlock block) {
				return (getBegin() <= block.getPtr() && block.getPtr() < getEnd());
			}

			bool isEmpty() {
				return (next_ == getBegin());
			}

			bool isFull() {
				return (next_ == getEnd());
			}

			bool isTop(RawBlock block) const {
				auto blockEnd {
					static_cast<ElementPtr>(block.getPtr()) + block.getSize()
				};

				return (blockEnd == next_);
			}

			SizeType calcRemaining() const {
				return getEnd() - next_;
			}


		private:
			using ElementType      = char;
			using ElementPtr       = ElementType       *;
			using ElementConstPtr  = ElementType const *;

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
	class TemplatedPolicy : public traits::ArrayPolicyBase<
														       CoreArray<char, stackSize> > {
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

		};

	}
}

#endif
