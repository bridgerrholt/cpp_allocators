#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_BITMAPPED_BLOCK_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_BITMAPPED_BLOCK_H

#include <vector>
#include <bitset>
#include <cstddef>
#include <climits>

#include "../common_types.h"
#include "../allocator_wrapper.h"

namespace bridgerrholt {
	namespace allocator_test {
		namespace allocators {

template <template <class T, SizeType size> class Array,
	SizeType blockSize,
	SizeType blockCount,
  SizeType alignment = alignof(std::max_align_t)>
class alignas(alignment) BitmappedBlock
{
	private:
		// Size access operations
		constexpr static SizeType metaDataSize() {
			// Meta data overhead is 1 bit per block.
			SizeType toReturn { blockCount / CHAR_BIT };

			// If the meta data size was rounded down due to integer casting,
			// round it up instead.
			if (blockCount % CHAR_BIT != 0)
				toReturn += 1;

			// The meta data size must be a multiple of the minimumAlignment so that
			// the storage is also aligned.
			SizeType remainder = toReturn % alignment;
			if (remainder != 0)
				toReturn += alignment - remainder;

			return toReturn;
		}

		constexpr static SizeType storageSize() {
			return blockSize * blockCount;
		}

		constexpr static SizeType totalSize() {
			return metaDataSize() + storageSize();
		}


	public:
		static_assert(blockSize % alignment == 0,
		              "Block size must be divisible by the minimumAlignment");

		using ByteType  = unsigned char;
		using ArrayType = Array<ByteType, totalSize()>;

		BitmappedBlock() : BitmappedBlock(ArrayType {}) {}

		BitmappedBlock(ArrayType && array) :
			array_                 {array},
			lastInsertionMetaByte_ {0} {

			for (std::size_t i = 0; i < metaDataSize(); ++i) {
				array_[i] = 0;
			}


			/*std::cout << "blockSize  = " << blockSize << '\n'
			          << "blockCount = " << blockCount << '\n'
			          << "minimumAlignment  = " << minimumAlignment << "\n\n";

			std::cout << "Meta data size: " << metaDataSize() << '\n'
			          << "Storage size:   " << storageSize() << '\n'
			          << "Total size:     " << totalSize() << '\n'
			          << "Efficiency:     " << efficiency() << '\n';*/
		}

		bool isEmpty() {
			for (std::size_t i {0}; i < blockCount; ++i) {
				if (getMetaBit(i) == 1)
					return false;
			}

			return true;
		}


		RawBlock allocate(SizeType size) {
			// Increase size to be a multiple of the block size.
			// If the size is 0, a block is still allocated.
			std::size_t remainder {size % blockSize};
			if (remainder != 0 || size == 0)
				size += blockSize - remainder;

			// The amount of blocks that must be reserved for the allocation.
			std::size_t blocksRequired {size / blockSize};

			// Total amount of blocks searched so far.
			// Allocation fails if this reaches blockCount.
			std::size_t blocksSearched    {0};

			// Size of current group of contiguous unset bits.
			// Allocation will succeed once this reaches blocksRequired.
			std::size_t currentRegionSize {0};

			// Loop through each bit in the meta data.
			std::size_t byte {lastInsertionMetaByte_};
			std::size_t end  {metaDataSize()};
			while (byte < end) {
				for (std::size_t bit {0}; bit < CHAR_BIT; ++bit) {

					if (getMetaBit(byte, bit) == 0) {
						++currentRegionSize;

						// Allocation will be successful.
						if (currentRegionSize == blocksRequired) {
							std::size_t firstIndex {blocksSearched - blocksRequired + 1};
							std::size_t lastIndex  {blocksSearched};

							std::size_t index {firstIndex};

							while (index <= lastIndex) {
								setMetaBit(index);

								++index;
							}

							if (index == blockSize)
								lastInsertionMetaByte_ = 0;
							else
								lastInsertionMetaByte_ = index;

							return {
								array_.data() + metaDataSize() + (firstIndex * blockSize),
								size
							};
						}
					}

					// The region has ended.
					else {
						currentRegionSize = 0;
					}

					++blocksSearched;
					if (blocksSearched == blockCount)
						return {nullptr, 0};
				}

				++byte;
				if (byte == metaDataSize()) {
					end = metaDataSize();
					byte = 0;
				}

			}

			return {nullptr, 0};
		}

		void deallocate(RawBlock block) {
			auto ptr = static_cast<Pointer>(block.getPtr());

			// The amount of blocks it takes up.
			std::size_t blocks     {block.getSize() / blockSize};
			std::size_t blockIndex {getBlockIndex(ptr)};
			std::size_t objectEnd  {blockIndex + blocks};

			while (blockIndex < objectEnd) {
				unsetMetaBit(blockIndex);

				++blockIndex;
			}
		}

		bool owns(RawBlock block) {
			auto ptr = static_cast<Pointer>(block.getPtr());

			return (
				ptr >= array_.data() + metaDataSize() &&
			  ptr <  array_.data() + totalSize()
			);
		}


	private:
		using Pointer = ByteType *;
		
		SizeType getBlockIndex(Pointer blockPtr) {
			return (blockPtr - array_.data() - metaDataSize()) / blockSize;
		}

		SizeType getMetaIndex(Pointer blockPtr) {
			return getMetaIndex(getBlockIndex(blockPtr));
		}

		SizeType getMetaBitIndex(Pointer blockPtr) {
			return getMetaIndex(getBlockIndex(blockPtr));
		}


		constexpr static SizeType getBlockIndex(
			SizeType metaIndex, SizeType metaBitIndex) {
			return (CHAR_BIT * metaIndex) + metaBitIndex;
		}

		constexpr static SizeType getMetaIndex(SizeType blockIndex) {
			return blockIndex / CHAR_BIT;
		}

		constexpr static SizeType getMetaBitIndex(SizeType blockIndex) {
			return blockIndex % CHAR_BIT;
		}


		Pointer getMetaPtr(Pointer blockPtr) {
			return array_[getMetaBitIndex(blockPtr)];
		}


		int getMetaBit(Pointer blockPtr) {
			return getMetaBit(getMetaIndex(blockPtr), getMetaBitIndex(blockPtr));
		}

		int getMetaBit(SizeType blockIndex) {
			return getMetaBit(getMetaIndex(blockIndex), getMetaBitIndex(blockIndex));
		}

		int getMetaBit(SizeType metaIndex, SizeType metaBitIndex) {
			ByteType & metaPtr {array_[metaIndex]};
			int temp = (metaPtr >> metaBitIndex) & 1;

			//std::cout << "(" << metaIndex << ", " << metaBitIndex << ") = " << temp << '\n';

			return temp;
		}


		void setMetaBit(Pointer blockPtr) {
			setMetaBit(getMetaIndex(blockPtr), getMetaBitIndex(blockPtr));
		}

		void setMetaBit(SizeType blockIndex) {
			setMetaBit(getMetaIndex(blockIndex), getMetaBitIndex(blockIndex));
		}

		void setMetaBit(SizeType metaIndex, SizeType metaBitIndex) {
			ByteType & metaRef {array_[metaIndex]};
			metaRef |= 1 << metaBitIndex;

			//std::cout << "set (" << metaIndex << ", " << metaBitIndex << ")\n";
		}


		void unsetMetaBit(Pointer blockPtr) {
			unsetMetaBit(getMetaIndex(blockPtr), getMetaBitIndex(blockPtr));
		}

		void unsetMetaBit(SizeType blockIndex) {
			unsetMetaBit(getMetaIndex(blockIndex), getMetaBitIndex(blockIndex));

			//std::cout << "Unset block " << blockIndex << '\n';
		}

		void unsetMetaBit(SizeType metaIndex, SizeType metaBitIndex) {
			ByteType & metaRef {array_[metaIndex]};
			metaRef &= ~(1 << metaBitIndex);
		}


		constexpr static double efficiency() {
			return static_cast<double>(blockCount) / (metaDataSize() * CHAR_BIT);
		}

		ArrayType   array_;
		std::size_t lastInsertionMetaByte_;
};


template <
	template <class T, SizeType size> class Allocator,
	SizeType blockCoefficient,
	SizeType blockCountCoefficient,
	SizeType alignment = alignof(std::max_align_t)>
using MemoryEfficientBitmappedBlock =
BitmappedBlock<
	Allocator,
	blockCoefficient * alignment,
  blockCountCoefficient * alignment * CHAR_BIT,
  alignment>;


		}
	}
}

#endif
