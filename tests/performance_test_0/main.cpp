#include <iostream>
#include <chrono>
#include <vector>
#include <array>
#include <numeric>
#include <algorithm>
#include <cassert>
#include <memory>

#include <allocators/wrappers/smart_allocator.h>
#include <allocators/bitmapped_block.h>
#include <allocators/free_list.h>
#include <allocators/full_free_list.h>
#include <allocators/segregator.h>
#include <allocators/fallback_allocator.h>
#include <allocators/affix_allocator.h>

#include "test_base.h"
#include "random_size_allocation_test.h"
#include "get_time.h"
#include "test_factory.h"


namespace brh {
	namespace allocators {
		namespace tests {

class NewReturnTypeSimple
{
	public:
		NewReturnTypeSimple(char * ptr) : ptr_ {ptr} {}

		bool isNull() const { return ptr_ == nullptr; }

		char * getPtr() { return ptr_; }

	private:
		char * ptr_;
};

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

		NewReturnTypeSimple allocate(std::size_t size) {
			return {new char[size]};
		}

		void deallocate(NewReturnTypeSimple ptr) {
			delete[] ptr.getPtr();
		}

		bool isEmpty() { return true; }
};


template <class T>
using NewReturnType = T *;

template <class T>
using AllocatorReturnType = BasicBlock<T>;

using AllocatorReturnTypeSimple = RawBlock;

template <class T>
using BlockAllocatorReturnType = T *;



template <class T, std::size_t size>
class VectorWrapper : public std::vector<T>
{
	public:
		VectorWrapper() : std::vector<T>() { std::vector<T>::reserve(size); }
};

template <class T>
using VectorSingle = std::vector<T>;


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

		void restart() {
			pointers_ = {};
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
			auto initializeStart = getTime();
			test.initialize();
			results_[0] = getTime() - initializeStart;

			auto constructStart = getTime();
			test.construct();
			results_[1] = getTime() - constructStart;

			auto destructStart = getTime();
			test.destruct();
			results_[2] = getTime() - destructStart;

			/*test.restart();

			constructStart = getTime();
			test.construct();
			results_[1] += getTime() - constructStart;


			destructStart = getTime();
			test.destruct();
			results_[2] += getTime() - destructStart;*/


			test.restart();
		}


		ResultType get(std::size_t i) { return results_[i]; }

		ResultList getResults() {
			return results_;
		}

	private:
		ResultList results_;
};


std::vector<std::array<double, TestBase::testCount> >
runTests(std::vector<TestBase *> tests, std::size_t iterations) {

	using FunctionResultsList = std::vector<FullTimedTest::ResultType>;
	using FunctionList = std::array<FunctionResultsList, TestBase::testCount>;

	std::vector<FunctionList> results;
	results.resize(tests.size());

	for (std::size_t i {0}; i < iterations; ++i) {
		std::cout << "Iteration " << i << '\n';
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

		}
	}
}


using namespace brh::allocators;
using namespace tests;

template <std::size_t elementCount>
class BestAllocator {
	public:
		static constexpr std::size_t freeListBlock {16};
		static constexpr std::size_t smallBlock    {32};
		static constexpr std::size_t largeBlock    {128};
		static constexpr std::size_t blockCount    {elementCount};

		using FreeListCore = FullFreeList::Templated<
			VectorWrapper, freeListBlock, elementCount
		>;

		using SmallAllocatorCore = BitmappedBlock::Templated<
			VectorWrapper, smallBlock, elementCount
		>;

		using LargeAllocatorCore = BitmappedBlock::Templated<
			VectorWrapper, largeBlock, 1024 * 1024 / largeBlock
		>;

		using FreeList = BlockAllocatorRegularInterface<
			FreeListCore
		>;

		using SmallAllocator = Segregator::Templated<
			FreeList, SmallAllocatorCore, freeListBlock
		>;

		using LargeAllocator = LargeAllocatorCore;

		/*using AllocatorCore = Segregator::Templated<
			SmallAllocator, LargeAllocator, smallBlock
		>;*/

		using AllocatorCore = Segregator::Templated<
			FreeList, LargeAllocator, freeListBlock
		>;

		using Allocator = AllocatorCore;
		//using Allocator = SmallAllocatorCore;

		using TestType =
			RandomSizeAllocationTest<Allocator, AllocatorReturnTypeSimple>;
};


void runTestsOld()
{

	/*using Type = long;
	using AllocatorBaseType = BitmappedBlock<VectorWrapper, sizeof(Type), alignof(Type), alignof(Type)>;
	using AllocatorType = AllocatorWrapper<AllocatorBaseType>;

	AllocatorType allocator;

	allocator.printBits<Type>();

	std::vector<BasicBlock<Type> > blocks;

	for (Type i = 0; i < AllocatorBaseType::blockCount; ++i) {
		blocks.emplace_back(allocator.construct<Type>(i + 1));
		allocator.printBits<Type>();
	}

	std::cout << allocator.construct<Type>().getPtr() << '\n';
	allocator.printBits<Type>();

	for (auto i : blocks) {
		allocator.destruct(i);
		allocator.printBits<Type>();
	}*/



	std::size_t iterations {1000};
	constexpr std::size_t elementCount {1'000};

	constexpr std::size_t smallBlockSize {16};
	constexpr std::size_t largeBlockSize {smallBlockSize * 4};
	constexpr std::size_t blockCount     {elementCount};


	/*using SmallAllocatorBase =
		FullFreeList<
			VectorWrapper, smallBlockSize, blockCount
		>;

	using SmallAllocator =
		BlockAllocatorRegularInterface<
			SmallAllocatorBase
		>;

	using LargeAllocatorBase =
		FullFreeList<
			VectorWrapper, largeBlockSize, blockCount
		>;

	using LargeAllocator =
		BlockAllocatorRegularInterface<
			LargeAllocatorBase
		>;

	using SecondaryAllocator =
		BitmappedBlock<
			VectorWrapper, largeBlockSize, 1024 * 1024
		>;

	using AllocatorType =
		Segregator<
			LargeAllocatorBase::blockSize, Segregator<
				SmallAllocatorBase::blockSize, SmallAllocator, LargeAllocator
			>, SecondaryAllocator
		>;*/

	using SmallAllocator =
		BitmappedBlock::Templated<
			VectorWrapper, smallBlockSize, 1024 * 1024
		>;

	using LargeAllocator =
		BitmappedBlock::Templated<
			VectorWrapper, largeBlockSize*4, 1024 * 512
		>;

	using SemiFreeListAllocator =
		Segregator::Templated<
			BlockAllocatorRegularInterface<FullFreeList::Templated<
				VectorWrapper, largeBlockSize, 1024 * 512>>,
			LargeAllocator, largeBlockSize
		>;

	/*using AllocatorType =
		Segregator<
			getBitmappedData<SmallAllocator>().getBlockSize(), SmallAllocator, LargeAllocator
		>;*/

	/*using AllocatorType =
		AffixChecker<
			BitmappedBlock::Templated<
				VectorWrapper, smallBlockSize, 1024 * 1024 * 4
			>, VarWrapper<int, 123>
		>;*/

	using RuntimeAllocator =
		BitmappedBlock::Runtime<VectorSingle>;

	using AllocatorType = BitmappedBlock::Templated<
		VectorWrapper, 16 * 16, 1024 * 1024
	>;

	/*AllocatorType all {};
	auto blk = all.allocate(32);
	auto blk2 = all.allocate(50);
	all.deallocate(blk);
	all.deallocate(blk2);*/


	using NewTestType = RandomSizeAllocationTest<NewAllocator, NewReturnTypeSimple>;
	NewTestType newTest {"C++ 'new'", elementCount};

	/*using AllocatorTestType = RandomSizeAllocationTest<AllocatorType, AllocatorReturnTypeSimple>;
	AllocatorTestType allocatorTest {"Template Bitmap", elementCount};*/

	using SemiFreeListTestType = RandomSizeAllocationTest<SemiFreeListAllocator, AllocatorReturnTypeSimple>;
	SemiFreeListTestType freeListTest {"Semi- Free List", elementCount};

	using RuntimeTestType = RandomSizeAllocationTest<RuntimeAllocator, AllocatorReturnTypeSimple>;
	using TemplatedTestType = RandomSizeAllocationTest<AllocatorType, AllocatorReturnTypeSimple>;
	RuntimeTestType runtimeTest1 {"Runtime Bitmap 1", RuntimeAllocator({AllocatorType::Policy::getAttributes().getBlockSize(), AllocatorType::Policy::getAttributes().getBlockCount()}), elementCount};
	RuntimeTestType runtimeTest2 {"Runtime Bitmap 2", RuntimeAllocator({largeBlockSize, AllocatorType::Policy::getAttributes().getBlockCount() * 8}), elementCount};
	TemplatedTestType templatedTest {"Templated Test", elementCount};

	std::vector<TestBase *> tests { &newTest, /*&allocatorTest, */&freeListTest, &runtimeTest1, &runtimeTest2, &templatedTest};

	/*BestAllocator<elementCount>::TestType bestTest {"Best Allocator", elementCount};

	std::vector<TestBase *> tests { &newTest, &bestTest };*/

	auto testResults = runTests(tests, iterations);
	std::vector<double> totals;
	totals.reserve(tests.size());

	for (std::size_t i {0}; i < testResults.size(); ++i) {
		auto result = testResults[i];

		totals[i] = std::accumulate(result.begin(),
		                            result.end(), 0.0);

		std::cout << '\n' <<
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
		std::cout << '\n';
	}
}


template <class TestType>
void runTests(std::size_t totalIterations,
              TestType    test,
							std::vector<std::shared_ptr<TestFactoryBase>> factories)
{

}


class DummyTest : TestBase
{
	public:
		DummyTest(std::string name) : TestBase(std::move(name)) {}

		void initialize() {};
		void construct()  {};
		void destruct()   {};

		void restart() {};
};


int main(int argc, char* argv[])
{
	try {
		/*using NewTestType = RandomSizeAllocationTest<NewAllocator, NewReturnTypeSimple>;

		runTests(4, 1024, {
			std::make_shared<TestFactoryBase>(BasicTestFactory<NewTestType>("C++ 'new'"))
		});*/

		runTestsOld();
	}
	catch (std::exception & e) {
		std::cout << "Main caught: " << e.what() << '\n';
		throw e;
	}

}