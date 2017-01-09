#include <iostream>
#include <chrono>
#include <vector>
#include <array>
#include <numeric>

#include <allocator_test/smart_allocator.h>
#include <allocator_test/allocators/bitmapped_block.h>

using namespace bridgerrholt::allocator_test;

template <class T = std::chrono::microseconds>
std::ptrdiff_t getTime() {
	auto now = std::chrono::system_clock::now().time_since_epoch();
	return std::chrono::duration_cast<T>(now).count();
}

template <typename Function>
std::ptrdiff_t testTime(std::size_t iterations, Function f) {
	auto start = getTime();

	for (std::size_t i = 0; i < iterations; ++i) {
		f();
	}

	auto end = getTime();

	return end - start;
}


class NewAllocator
{
	public:
		NewAllocator() = default;

		template <class T, class ... ArgTypes>
		T * construct(ArgTypes ... args) {
			return new T {std::forward<ArgTypes>(args)...};
		}

		template <class T>
		void destruct(T * ptr) {
			delete ptr;
		}
};


template <class T>
using NewReturnType = T *;

template <class T>
using AllocatorReturnType = BasicBlock<T>;


template <class Allocator, template <class T> class ReturnType>
class Test
{
	public:
		Test() = default;
		Test(Allocator allocator) : allocator_ {allocator} {}

		void operator()() {
			static constexpr std::size_t count {100};

			run<std::size_t>(count);
			run<unsigned char>(count);
		}

	private:
		template <class T>
		void run(std::size_t count) {
			std::vector<ReturnType<T> > returned;

			for (std::size_t i = 0; i < count; ++i)
				returned.emplace_back(allocator_.template construct<T>(static_cast<T>(i)));

			for (std::size_t i = 0; i < count; ++i)
				allocator_.template destruct(returned[i]);
		}

		Allocator allocator_;
};


template <class T, std::size_t size>
class VectorWrapper : public std::vector<T>
{
	public:
		VectorWrapper() : std::vector<T>(size) {}
};


template <class Allocator, template <class> class ReturnType, class T>
class FullTest
{
	public:
		FullTest(Allocator allocator, std::size_t elementCount) :
			allocator_   {allocator},
			elementCount_{elementCount} {}

		FullTest(std::size_t elementCount) :
			elementCount_{elementCount} {}

		void initialize() {
			pointers_.reserve(elementCount_);
		}

		void construct() {
			for (std::size_t i = 0; i < elementCount_; ++i) {
				pointers_[i] = allocator_.template construct<T>();
			}
		}

		void destruct() {
			for (std::size_t i = 0; i < elementCount_; ++i) {
				allocator_.template destruct(pointers_[i]);
			}
		}


		Allocator & getAllocator() { return allocator_; }


	private:
		Allocator   allocator_;
		std::size_t elementCount_;
		std::vector<ReturnType<T> > pointers_;
};


template <class TestType>
class FullTimedTest
{
	public:
		FullTimedTest(TestType & test) {
			begins_[0] = getTime();
			test.initialize();
			ends_[0] = getTime();


			begins_[1] = getTime();
			test.construct();
			ends_[1] = getTime();


			begins_[2] = getTime();
			test.destruct();
			ends_[2] = getTime();
		}


		std::ptrdiff_t get(std::size_t i) { return ends_[i] - begins_[i]; }

	private:
		std::array<std::ptrdiff_t, 3> begins_;
		std::array<std::ptrdiff_t, 3> ends_;
};



int main(int argc, char* argv[])
{
	try {
		using namespace allocators;

		std::size_t iterations {1000};

		using DataType = std::array<int, 64>;

		using AllocatorType =
			SmartAllocator<
				MemoryEfficientBitmappedBlock<
					std::array, sizeof(DataType)/alignof(DataType), 50, alignof(DataType)
				>
			>;

		constexpr std::size_t elementCount {100};

		using NewTestType = FullTest<NewAllocator, NewReturnType, DataType>;
		NewTestType newTest {elementCount};

		using AllocatorTestType = FullTest<AllocatorType, AllocatorReturnType, DataType>;
		AllocatorTestType allocatorTest {elementCount};

		std::array<std::array<std::vector<double>, 3>, 2> results;

		std::size_t testId {0};

		for (std::size_t i = 0; i < iterations * 2; ++i) {
			if (testId == 0) {
				auto test = FullTimedTest<NewTestType>{newTest};

				for (std::size_t j = 0; j < 3; ++j) {
					results[0][j].push_back(test.get(j));
				}

				testId = 1;
			}
			else {
				//std::cout << "Pre:  " << allocatorTest.getAllocator().isEmpty() << '\n';
				auto test = FullTimedTest<AllocatorTestType>{allocatorTest};
				//std::cout << "Post: " << allocatorTest.getAllocator().isEmpty() << '\n';

				for (std::size_t j = 0; j < 3; ++j) {
					results[1][j].push_back(test.get(j));
				}

				testId = 0;
			}
		}

		std::array<std::array<std::ptrdiff_t, 3>, 2> averages;
		for (std::size_t i = 0; i < 2; ++i) {
			for (std::size_t j = 0; j < 3; ++j) {
				auto const & vec = results[i][j];
				averages[i][j] = std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
			}
		}

		std::cout << std::endl;
		for (std::size_t i = 0; i < 2; ++i) {
			std::cout <<
        "Initialize: " << averages[i][0] << '\n' <<
		    "Construct:  " << averages[i][1] << '\n' <<
	      "Destruct:   " << averages[i][2] << '\n' <<
				"Total:      " << std::accumulate(averages[i].begin(),
			                                    averages[i].end(), 0) << "\n\n";
		}

	}
	catch (std::exception & e) {
		std::cout << "Main caught: " << e.what() << '\n';
		throw e;
	}
}