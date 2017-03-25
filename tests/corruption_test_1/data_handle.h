#ifndef BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_1_DATA_HANDLE_H
#define BRIDGERRHOLT_ALLOCATORS_CORRUPTION_TEST_1_DATA_HANDLE_H

#include <random>
#include <vector>

#include <allocators/blocks/block.h>

namespace bridgerrholt {
	namespace allocators {
		namespace tests {

class DataHandle
{
	private:
		using ByteType = unsigned char;

	public:
		template <class Generator>
		DataHandle(Generator & randomGenerator, RawBlock block) :
			block_ (std::move(block)) {

			std::uniform_int_distribution<ByteType> dist {0, 255};

			filledData_.reserve(block_.getSize());
			for (std::size_t i {0}; i < block_.getSize(); ++i) {
				auto number = dist(randomGenerator);
				filledData_.push_back(number);
				getPtr()[i] = number;
			}
		}

		bool test() const {
			for (std::size_t i {0}; i < block_.getSize(); ++i) {
				if (getPtr()[i] != filledData_[i]) {
					return false;
				}
			}

			return true;
		}


	private:
		ByteType * getPtr() {
			return static_cast<ByteType*>(block_.getPtr());
		}

		ByteType const * getPtr() const {
			return static_cast<ByteType const *>(block_.getPtr());
		}

		RawBlock              block_;
		std::vector<ByteType> filledData_;
};

		}
	}
}

#endif
