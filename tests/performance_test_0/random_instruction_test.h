#ifndef BRH_CPP_ALLOCATORS_PERFORMANCE_TEST_0_RANDOM_INSTRUCTION_TEST_H
#define BRH_CPP_ALLOCATORS_PERFORMANCE_TEST_0_RANDOM_INSTRUCTION_TEST_H

#include <random>

#include "test_base.h"
#include "get_time.h"

namespace brh {
	namespace allocators {
		namespace tests {

template <class Allocator, class BlockType>
class RandomInstructionTest : public TestBase
{
	public:
		RandomInstructionTest(std::string name,
		                         Allocator   allocator,
		                         std::size_t elementCount) :
			allocator_   {std::move(allocator)},
			elementCount_{elementCount},
			TestBase     {std::move(name)} {

			restart();


		}


		RandomInstructionTest(std::string name, std::size_t elementCount) :
			RandomInstructionTest(name, Allocator {}, elementCount) {}

		~RandomSizeAllocationTest() {
			destruct();
		}

		void initialize() {
			pointers_.reserve(elementCount_);
			//std::cout << allocator_.isEmpty() << '\n';
			//allocator_.printMeta();
			//std::cout << std::endl;
		}

		void construct() {
			std::size_t j {0};
			//for (auto i : sizes_) {
			std::size_t i;
			for (std::size_t index {0}; index < sizes_.size(); ++index) {
				i = sizes_[index];

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

			std::ptrdiff_t const seed = 0; //getTime();

			std::mt19937 randomEngine {
				static_cast<typename std::mt19937::result_type>(seed)
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



union InstructionValue
{
	public:
		std::size_t size;
		std::size_t index;
};


class Instruction
{
	public:
		enum class Kind {
				ALLOCATE,
				DEALLOCATE
		};

		constexpr Instruction(Kind kind, InstructionValue value) :
			kind_  {kind} {

			if (kind_ == Kind::ALLOCATE)
				value_.size = value.size;
			else
				value_.index = value.index;
		}


		Kind        getKind()  const { return kind_; }
		std::size_t getSize()  const { return value_.size; }
		std::size_t getIndex() const { return value_.index; }

	private:
		Kind             kind_;
		InstructionValue value_;
};


		}
	}
}

#endif
