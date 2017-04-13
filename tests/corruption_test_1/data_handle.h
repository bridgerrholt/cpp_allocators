#ifndef BRH_CPP_ALLOCATORS_CORRUPTION_TEST_1_DATA_HANDLE_H
#define BRH_CPP_ALLOCATORS_CORRUPTION_TEST_1_DATA_HANDLE_H

#include <random>
#include <vector>

#include <allocators/blocks/block.h>

namespace brh {
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

			refresh(randomGenerator);
		}

		bool test() const {
			for (std::size_t i {0}; i < block_.getSize(); ++i) {
				if (getPtr()[i] != filledData_[i]) {
					return false;
				}
			}

			return true;
		}

		template <class Generator>
		void refresh(Generator & randomGenerator) {
			std::uniform_int_distribution<ByteType> dist {0, 255};

			filledData_.clear();
			filledData_.reserve(block_.getSize());
			for (std::size_t i {0}; i < block_.getSize(); ++i) {
				auto number = dist(randomGenerator);
				filledData_.push_back(number);
				getPtr()[i] = number;
			}
		}

		void dataString(std::string & str) {
			for (std::size_t i {0}; i < block_.getSize(); ++i) {
				str += std::to_string(static_cast<unsigned int>(getPtr()[i])) + " ";
			}
		}

		void set(RawBlock block) {
			block_ = block;
		}

		RawBlock & get() {
			return block_;
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
