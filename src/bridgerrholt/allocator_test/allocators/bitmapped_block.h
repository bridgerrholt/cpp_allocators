#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_BITMAPPED_BLOCK_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_BITMAPPED_BLOCK_H

#include <vector>
#include <bitset>
#include <cstddef>
#include <climits>

#include "../common_types.h"
#include "../allocator_wrapper.h"
#include "common/round_up_to_multiple.h"

namespace bridgerrholt {
	namespace allocator_test {
		namespace allocators {

class BitmappedBlockArrayElement
{
	public:
		void clearBits() { data_ = 0; }

		int getBit(std::size_t place) {
			return ((data_ >> place) & 1);
		}

		void setBit(std::size_t place) {
			data_ |= 1 << place;
		}

		void unsetBit(std::size_t place) {
			data_ &= ~(1 << place);
		}


	private:
		char data_;
};

static_assert(std::is_pod<BitmappedBlockArrayElement>::value,
              "BitmappedBlockArrayElement should be POD");

/// An allocator with memory segmented into blocks and meta data before the
/// blocks telling whether each is occupied or not.
/// This is a 1 bit per block overhead.
template <template <class T, SizeType size> class Array,
	SizeType    minimumBlockSize,
	SizeType    blockCount,
  std::size_t alignment = alignof(std::max_align_t)>
class alignas(alignment) BitmappedBlock
{
	private:
		//  Size access operations
		// Returns the amount of bytes that the meta header data requires.
		static constexpr SizeType getMetaDataSize() {
			// Meta data overhead is 1 bit per block.
			SizeType toReturn { blockCount / CHAR_BIT };

			// If the meta data size was rounded down due to integer casting,
			// round it up instead.
			if (blockCount % CHAR_BIT != 0)
				toReturn += 1;

			// The meta data size must be a multiple of the alignment so that
			// the storage is also aligned.
			toReturn = common::roundUpToMultiple(toReturn, alignment);

			return toReturn;
		}

		// Returns the amount of bytes that the block list requires.
		static constexpr SizeType getStorageSize() {
			return blockSize * blockCount;
		}

		// Returns the total amount of bytes required.
		static constexpr SizeType getTotalSize() {
			return getMetaDataSize() + getStorageSize();
		}


	public:
		static_assert((minimumBlockSize % alignment == 0),
		              "Minimum block size must be divisible by the alignment");

		static constexpr
		SizeType blockSize {std::max(alignment, minimumBlockSize)};

		using ElementType = BitmappedBlockArrayElement;
		using ArrayType   = Array<ElementType, getTotalSize()>;

		BitmappedBlock() : BitmappedBlock(ArrayType {}) {}

		BitmappedBlock(ArrayType array) :
			array_                 {std::move(array)},
			lastInsertionMetaByte_ {0} {

			for (std::size_t i = 0; i < getMetaDataSize(); ++i) {
				array_[i].clearBits();
			}


			/*std::cout << "minimumBlockSize  = " << minimumBlockSize << '\n'
			          << "blockCount = " << blockCount << '\n'
			          << "minimumAlignment  = " << minimumAlignment << "\n\n";

			std::cout << "Meta data size: " << getMetaDataSize() << '\n'
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
			// An allocation takes place even if the size is 0.
			if (size == 0) {
				size = blockSize;
			}
			else {
				// The size must take up a whole number of blocks.
				size = common::roundUpToMultiple(size, blockSize);
			}

			// The amount of blocks that must be reserved for the allocation.
			std::size_t blocksRequired {size / blockSize};

			// Total amount of blocks searched so far.
			// Allocation fails if this reaches blockCount.
			std::size_t blocksSearched {0};

			// Size of current group of contiguous unset bits.
			// Allocation will succeed once this reaches blocksRequired.
			std::size_t currentRegionSize {0};

			// Loop through each bit in the meta data.
			std::size_t byte {lastInsertionMetaByte_};
			std::size_t end  {getMetaDataSize()};

			while (byte < end) {
				for (std::size_t bit {0}; bit < CHAR_BIT; ++bit) {

					if (getMetaBit(byte, bit) == 0) {
						++currentRegionSize;

						// Allocation will be successful.
						if (currentRegionSize == blocksRequired) {
							// Set all the meta bits to indicate occupation of the blocks.
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

							return {getBlockPtr(firstIndex), size};
						}
					}

					// The region has ended if the bit is not 0.
					else {
						currentRegionSize = 0;
					}

					++blocksSearched;
					if (blocksSearched == blockCount)
						return RawBlock::makeNullBlock();
				}

				++byte;
				if (byte == getMetaDataSize()) {
					end = getMetaDataSize();
					byte = 0;
				}

			}

			return RawBlock::makeNullBlock();
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
				ptr >= array_.data() + getMetaDataSize() &&
			  ptr <  array_.data() + getTotalSize()
			);
		}


	private:
		using Pointer = ElementType *;
		
		SizeType getBlockIndex(Pointer blockPtr) {
			return (blockPtr - array_.data() - getMetaDataSize()) / blockSize;
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

		Pointer getBlockPtr(SizeType blockIndex) {
			return array_.data() + getMetaDataSize() + (blockIndex * blockSize);
		}


		int getMetaBit(Pointer blockPtr) {
			return getMetaBit(getMetaIndex(blockPtr), getMetaBitIndex(blockPtr));
		}

		int getMetaBit(SizeType blockIndex) {
			return getMetaBit(getMetaIndex(blockIndex), getMetaBitIndex(blockIndex));
		}

		int getMetaBit(SizeType metaIndex, SizeType metaBitIndex) {
			return array_[metaIndex].getBit(metaBitIndex);
		}


		void setMetaBit(Pointer blockPtr) {
			setMetaBit(getMetaIndex(blockPtr), getMetaBitIndex(blockPtr));
		}

		void setMetaBit(SizeType blockIndex) {
			setMetaBit(getMetaIndex(blockIndex), getMetaBitIndex(blockIndex));
		}

		void setMetaBit(SizeType metaIndex, SizeType metaBitIndex) {
			array_[metaIndex].setBit(metaBitIndex);
		}


		void unsetMetaBit(Pointer blockPtr) {
			unsetMetaBit(getMetaIndex(blockPtr), getMetaBitIndex(blockPtr));
		}

		void unsetMetaBit(SizeType blockIndex) {
			unsetMetaBit(getMetaIndex(blockIndex), getMetaBitIndex(blockIndex));

			//std::cout << "Unset block " << blockIndex << '\n';
		}

		void unsetMetaBit(SizeType metaIndex, SizeType metaBitIndex) {
			array_[metaIndex].unsetBit(metaBitIndex);
		}


		constexpr static double efficiency() {
			return static_cast<double>(blockCount) / (getMetaDataSize() * CHAR_BIT);
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
