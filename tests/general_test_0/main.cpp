#include <iostream>
#include <memory>
#include <array>

#include <allocators/wrappers/allocator_wrapper.h>
#include <allocators/wrappers/allocator_singleton.h>
#include <allocators/wrappers/smart_allocator.h>

#include <allocators/malloc_allocator.h>
#include <allocators/bitmapped_block.h>
#include <allocators/free_list.h>
#include <allocators/full_free_list.h>


class Base
{
	public:
		Base() { std::cout << "Base()\n"; }

		virtual ~Base() { std::cout << "~Base()\n"; }
};

class A : public Base
{
	public:
		void swap(A & first, A & second) {
			using std::swap;

			swap(first.a_, second.a_);
		}

		A() : Base() { std::cout << "A()\n"; }

		~A() { std::cout << "~A() a_: " << a_ << '\n'; }

		A & operator=(A other) {
			swap(*this, other);

			std::cout << "operator=(A)\n";

			return *this;
		}

	private:
		int a_ { 124002140 };
};

class B : public Base
{
	public:
		B() : Base() { std::cout << "B()\n"; }

		~B() { std::cout << "~B()\n"; }

	private:
		long b1_ { 10 };
		long b2_ { 20 };
};



int main() {
	using namespace bridgerrholt::allocators;

	using Type = A;
	using BaseAllocator = FullFreeList<std::array, sizeof(Type), 8>;
	using AllocatorType = BlockAllocatorWrapper<BaseAllocator>;

	AllocatorType allocator;

	Type * arr[9];

	for (std::size_t i {0}; i < 8; ++i) {
		arr[i] = allocator.construct<Type>();
		std::cout << i << ": " << arr[i] << '\n';
	}

	allocator.destruct(arr[3]);
	allocator.destruct(arr[5]);
	allocator.destruct(arr[1]);
	arr[3] = allocator.construct<Type>();
	arr[5] = allocator.construct<Type>();
	arr[1] = allocator.construct<Type>();

	std::cout << std::endl;

	for (std::size_t i {0}; i < 8; ++i) {
		allocator.destruct(arr[i]);
		std::cout << i << ": " << arr[i] << '\n';
	}


	return 0;
}