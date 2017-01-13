#include <iostream>
#include <chrono>
#include <vector>
#include <array>
#include <numeric>
#include <algorithm>

#include <allocator_test/smart_allocator.h>
#include <allocator_test/allocators/bitmapped_block.h>
#include <allocator_test/allocators/free_list.h>
#include <allocator_test/allocators/full_free_list.h>

using namespace bridgerrholt::allocator_test;

template <class T = std::chrono::microseconds>
std::ptrdiff_t getTime() {
	auto now = std::chrono::system_clock::now().time_since_epoch();
	return std::chrono::duration_cast<T>(now).count();
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

template <class T>
using BlockAllocatorReturnType = T *;



template <class T, std::size_t size>
class VectorWrapper : public std::vector<T>
{
	public:
		VectorWrapper() : std::vector<T>(size) {}
};


class TestBase
{
	public:
		TestBase(std::string name) : name_ {name} {}

		virtual ~TestBase() = 0;

		virtual void initialize() = 0;
		virtual void construct()  = 0;
		virtual void destruct()   = 0;

		static constexpr std::size_t testCount {3};

		std::string const & getName() const { return name_; }

	private:
		std::string name_;
};

TestBase::~TestBase() {}


template <class Allocator, template <class> class BlockType, class T>
class BasicTest : public TestBase
{
	public:
		BasicTest(std::string name, Allocator allocator, std::size_t elementCount) :
			allocator_   {std::move(allocator)},
			elementCount_{elementCount},
			TestBase     {std::move(name)} {}

		BasicTest(std::string name, std::size_t elementCount) :
			elementCount_{elementCount},
			TestBase     {std::move(name)} {}

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
		std::vector<BlockType<T> > pointers_;
};


class FullTimedTest
{
	public:
		using ResultType = std::ptrdiff_t;
		using ResultList = std::array<ResultType, TestBase::testCount>;

		FullTimedTest(TestBase & test) {
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


		ResultType get(std::size_t i) { return ends_[i] - begins_[i]; }

		ResultList getResults() {
			ResultList toReturn;

			for (std::size_t i = 0; i < TestBase::testCount; ++i) {
				toReturn[i] = get(i);
			}

			return toReturn;
		}

	private:
		std::array<ResultType, TestBase::testCount> begins_;
		std::array<ResultType, TestBase::testCount> ends_;
};


std::vector<std::array<double, TestBase::testCount> >
runTests(std::vector<TestBase *> tests, std::size_t iterations) {

	using FunctionResultsList = std::vector<FullTimedTest::ResultType>;
	using FunctionList = std::array<FunctionResultsList, TestBase::testCount>;

	std::vector<FunctionList> results;
	results.resize(tests.size());

	for (std::size_t i {0}; i < iterations; ++i) {
		for (std::size_t test {0}; test < tests.size(); ++test) {
			FullTimedTest timedTest {*tests[test]};
			auto singleResults = timedTest.getResults();
			for (std::size_t result {0}; result < singleResults.size(); ++result) {
				results[test][result].emplace_back(singleResults[result]);
			}
		}
	}

	std::vector<std::array<double, TestBase::testCount> > averages;
	averages.resize(tests.size());
	for (std::size_t i {0}; i < results.size(); ++i) {
		for (std::size_t j {0}; j < results[i].size(); ++j) {
			auto result = results[i][j];
			double average {std::accumulate(result.begin(), result.end(), 0.0) / iterations};
			averages[i][j] = average;
		}
	}

	return averages;
}


int main(int argc, char* argv[])
{
	try {
		using namespace allocators;

		std::size_t iterations {1'000};
		constexpr std::size_t elementCount {1};

		using DataType = std::array<int, 64>;

		using AllocatorBaseType =
			/*MemoryEfficientBitmappedBlock<
				VectorWrapper, sizeof(DataType)/alignof(DataType), elementCount/(16*8)
			>;*/
			FullFreeList<
				std::array, std::max(sizeof(DataType), alignof(std::max_align_t)), elementCount
			>;

		using AllocatorType =
			//AllocatorWrapper<
			BlockAllocatorWrapper<
				AllocatorBaseType
			>;


		using NewTestType = BasicTest<NewAllocator, NewReturnType, DataType>;
		NewTestType newTest {"New", elementCount};

		using AllocatorTestType = BasicTest<AllocatorType, /*AllocatorReturnType*/ BlockAllocatorReturnType, DataType>;
		AllocatorTestType allocatorTest {"Mine", elementCount};

		std::vector<TestBase *> tests { &newTest, &allocatorTest };
		auto testResults = runTests(tests, iterations);
		std::vector<double> totals;
		totals.resize(tests.size());

		for (std::size_t i {0}; i < testResults.size(); ++i) {
			auto result = testResults[i];

			totals[i] = std::accumulate(result.begin(),
			                            result.end(), 0.0);

			std::cout <<
        tests.at(i)->getName() << '\n' <<
        "Initialize: " << result[0] << '\n' <<
        "Construct:  " << result[1] << '\n' <<
        "Destruct:   " << result[2] << '\n' <<
        "Total:      " << totals[i] << "\n\n";
		}

		for (std::size_t i {0}; i < testResults.size(); ++i) {
			for (std::size_t j {0}; j < testResults.size(); ++j) {
				if (i == j) continue;

				std::cout << tests[i]->getName() << " : " << tests[j]->getName() << " = " << totals[i] / totals[j] << '\n';
			}
		}

	}
	catch (std::exception & e) {
		std::cout << "Main caught: " << e.what() << '\n';
		throw e;
	}
}