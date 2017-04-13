#include "run_stack_test.h"

#include <vector>
#include <random>
#include <fstream>

#include "data_handle.h"

#include "../corruption_test_0/instructions.h"

namespace {

using namespace brh::allocators::tests;

class Instance
{
	public:
		Instance(GenerationArgPack args) :
			args_             {std::move(args)},
			status_           {false},
			size_             {0},
			instructionCount_ {0} {

			std::uniform_int_distribution<int> dist   {1, 4};
			std::uniform_int_distribution<int> chance {0, 20};
			int setChoice {0};
			bool fail {false};

			while (instructionCount_ < args_.minInstructionCount) {
				int choice;

				if (handles_.empty())
					choice = 1;

				else if (size_ == args_.totalSize)
					choice = 2;

				else if (setChoice == 0) {
					choice = dist(randomEngine_);
					if (choice == 2) {
						if (chance(randomEngine_) != 0)
							choice = 1;
					}
				}

				else {
					choice = setChoice;
					setChoice = 0;
				}


				switch (choice) {
					case 1:
						if (!allocate())
							setChoice = 2;
						break;

					case 2:
						if (!deallocate())
							fail = true;
						break;

					case 3:
						if (!reallocate())
							setChoice = 2;
						break;

					case 4:
						if (!expand())
							setChoice = 2;
						break;

					default:
						throw std::runtime_error("Invalid case");
				}

				outputData();

				if (fail) break;
			}

			status_ = !fail;

			while (!handles_.empty()) {
				auto & handle = handles_.back();
				if (!handle.test()) {
					fail = true;
					break;
				}

				args_.allocator.deallocate(handle.get());
				handles_.pop_back();
				outputData();
			}

			status_ = !fail;
		}

		bool status() const { return status_; }

		std::string dataString() {
			std::string str;
			for (auto & i : handles_) {
				i.dataString(str);
				str += " | ";
			}
			return str;
		}


	private:
		std::size_t generateSize() {
			auto max = std::min(args_.maxAllocationSize,
			                    args_.totalSize - size_);

			return randomSize(args_.minAllocationSize, max);
		}

		bool allocate() {
			auto block = args_.allocator.allocate(generateSize());

			auto success = !block.isNull();

			if (success) {
				handles_.emplace_back(randomEngine_, block);
				instructionCount_ += 2;
				size_ += block.getSize();
			}

			return success;
		}

		bool deallocate() {
			auto count = randomSize(1, handles_.size());

			for (std::size_t i {0}; i < count; ++i) {
				auto handle = handles_.back();

				bool success = handle.test();

				args_.allocator.deallocate(handle.get());

				size_ -= handle.get().getSize();

				handles_.pop_back();

				if (i != count-1)
					outputData();

				if (!success)
					return false;
			}

			return true;
		}

		bool reallocate() {
			auto & handle = handles_.back();
			auto & block = handle.get();

			auto oldSize = block.getSize();

			auto success = args_.allocator.reallocate(
				block, generateSize()
			);

			if (success) {
				++instructionCount_;
				size_ -= oldSize;
				size_ += block.getSize();

				handle.refresh(randomEngine_);
			}

			return success;
		}

		bool expand() {
			auto & handle = handles_.back();
			auto & block = handle.get();

			auto oldSize = block.getSize();

			auto success = args_.allocator.expand(
				handles_.back().get(), generateSize()
			);

			if (success) {
				++instructionCount_;
				size_ -= oldSize;
				size_ += block.getSize();

				handle.refresh(randomEngine_);
			}

			return success;
		}

		std::size_t randomSize(std::size_t min, std::size_t max) {
			std::uniform_int_distribution<std::size_t> distribution {
				min, max
			};

			return distribution(randomEngine_);
		}

		void outputData() {
			static std::ofstream out {"stack_out.txt"};

			out << dataString() << '\n';
		}



		GenerationArgPack args_;
		bool        status_;
		std::size_t size_;
		std::size_t instructionCount_;

		std::mt19937            randomEngine_;
		std::vector<DataHandle> handles_;

};

}


namespace brh {
	namespace allocators {
		namespace tests {

bool runStackTest(GenerationArgPack args)
{
	Instance instance(std::move(args));

	return instance.status();
}

		}
	}
}