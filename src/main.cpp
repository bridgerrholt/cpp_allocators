#include <iostream>
#include <memory>
#include <array>

#include "bridgerrholt/allocator_test/allocator_wrapper.h"
#include "bridgerrholt/allocator_test/allocator_singleton.h"
#include "bridgerrholt/allocator_test/smart_allocator.h"

#include "bridgerrholt/allocator_test/allocators/malloc_allocator.h"
#include "bridgerrholt/allocator_test/allocators/bitmapped_block.h"
#include "bridgerrholt/allocator_test/allocators/free_list.h"
#include "bridgerrholt/allocator_test/allocators/full_free_list.h"


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
	using namespace bridgerrholt::allocator_test::allocators;

	using Type = std::max_align_t;

	FullFreeList<std::array, sizeof(Type), 8> allocator;

	void * arr[9];

	for (std::size_t i {0}; i < 9; ++i) {
		arr[i] = allocator.allocate();
		std::cout << arr[i] << '\n';
	}


	return 0;
}