#ifndef CPP_ALLOCATOR_TEST_PERFORMANCE_TESTS_RANDOM_SIZE_ALLOCATION_TEST_H
#define CPP_ALLOCATOR_TEST_PERFORMANCE_TESTS_RANDOM_SIZE_ALLOCATION_TEST_H

#include <random>

#include "test_base.h"
#include "get_time.h"

namespace bridgerrholt {
	namespace allocator_test {
		namespace performance_tests {

template <class Allocator, class BlockType>
class RandomSizeAllocationTest : public TestBase
{
	public:
		RandomSizeAllocationTest(std::string name,
		                         Allocator   allocator,
		                         std::size_t elementCount) :
			allocator_   {std::move(allocator)},
			elementCount_{elementCount},
			TestBase     {std::move(name)} {

			restart();


		}


		RandomSizeAllocationTest(std::string name, std::size_t elementCount) :
			RandomSizeAllocationTest(name, Allocator {}, elementCount) {}

		~RandomSizeAllocationTest() {
			destruct();
		}

		void initialize() {
			pointers_.reserve(elementCount_);
		}

		void construct() {
			std::size_t j {0};
			for (auto i : sizes_) {
				auto temp = allocator_.allocate(i);

				if (temp.isNull()) {
					throw std::runtime_error("Allocation failed");
				}

				else {
					pointers_.emplace_back(temp);
				}
			}

			++j;
		}

		void destruct() {
			for (auto i : pointers_) {
				allocator_.deallocate(i);
			}
		}

		void restart() {
			pointers_ = {};
			sizes_ = {};

			std::mt19937 randomEngine {
				static_cast<typename std::mt19937::result_type>(getTime())
			};
			std::normal_distribution<> distribution {0, 128};

			for (std::size_t i = 0; i < elementCount_; ++i) {
				auto num = std::abs(std::round(distribution(randomEngine))) + 1;
				sizes_.emplace_back(num);
				//std::cout << num << '\n';
			}
		}


	private:
		Allocator   allocator_;
		std::size_t elementCount_;
		std::vector<BlockType> pointers_;

		std::vector<std::size_t> sizes_;
};


		}
	}
}

#endif
