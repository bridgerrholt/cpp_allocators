#include <iostream>
#include <memory>

#include <boost/pool/simple_segregated_storage.hpp>

/*#include "bridgerrholt/allocator_test/allocator_wrapper.h"
#include "bridgerrholt/allocator_test/allocator_singleton.h"
#include "bridgerrholt/allocator_test/allocators/malloc_allocator.h"*/

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
	/*constexpr std::size_t arrSize {5000};
	char arr [arrSize];
	for (std::size_t i = 0; i < arrSize; ++i)
		arr[i] = 0;

	//std::unique_ptr<Base> uniquePtr { new A {} };

	using Type = A;

	//reinterpret_cast<Type*>(arr)[0] = Type {};

	Base * ptr = new (reinterpret_cast<void*>(&arr[0])) Type {};

	std::cout << (std::ptrdiff_t)arr % sizeof(Type) << ' ' << sizeof(Type) << '\n';

	for (std::size_t i = 0; i < arrSize; ++i) {
		if (arr[i] != 0) {
			std::cout << "arr[" << i << "]: " << static_cast<int>(arr[i]) << '\n';
		}
	}



	ptr->~Base();*/


	/*using namespace bridgerrholt::allocator_test;
	using AllocatorType = AllocatorSingleton<MallocAllocator>;

	auto block = AllocatorType::get().construct<A>();

	SmartBlockSingleton<MallocAllocator, A> smartBlock {block};

	auto block2 = makeBlock<AllocatorType &, SmartBlockSingleton<MallocAllocator, Base>, A>(AllocatorType::get());*/


	boost::simple_segregated_storage<std::size_t> storage;
	char v [1024];
	storage.add_block(v, 1024, 256);

	int *i = static_cast<int*>(storage.malloc());
	*i = 1;

	int *j = static_cast<int*>(storage.malloc_n(1, 512));
	j[10] = 2;

	storage.free(i);
	storage.free_n(j, 1, 512);


	return 0;
}