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
#include <allocators/stack_allocator.h>


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
	using TypeArray = std::array<A, 10>;
	using BaseAllocator = StackAllocator<std::array, sizeof(Type) * 1024>;
	using AllocatorType = SmartAllocator<BaseAllocator>;

	AllocatorType allocator;

	std::size_t const size {8};
	BasicBlock<Type> arr[size];

	for (std::size_t i {0}; i < size; ++i) {
		arr[i] = allocator.construct<Type>();
		std::cout << i << ": " << arr[i].getPtr() << '\n';
	}

	allocator.destruct(arr[size-1]);
	arr[size-1] = allocator.construct<Type>();

	auto temp = allocator.construct<TypeArray>();
	std::cout << temp.getPtr() << '\n';
	allocator.destruct(temp);

	std::cout << std::endl;

	for (std::size_t i {size}; i > 0; --i) {
		std::size_t j = i - 1;
		allocator.destruct(arr[j]);
		std::cout << j << ": " << arr[j].getPtr() << '\n';
	}


	return 0;
}