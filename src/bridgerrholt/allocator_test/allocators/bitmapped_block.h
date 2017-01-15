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

/// This allocator does not have an inherit alignment requirement, meaning if
/// you want custom alignment you are responsible for ensuring it is
/// strict enough for all your data types.
template <template <class T, SizeType size> class Array,
	SizeType    minimumBlockSize,
	SizeType    blockCount,
  std::size_t alignment = alignof(std::max_align_t)>
class alignas(alignment) BitmappedBlock
{
	private:
		// Size access operations
		static constexpr SizeType getMetaDataSize() {
			// Meta data overhead is 1 bit per block.
			SizeType toReturn { blockCount / CHAR_BIT };

			// If the meta data size was rounded down due to integer casting,
			// round it up instead.
			if (blockCount % CHAR_BIT != 0)
				toReturn += 1;

			// The meta data size must be a multiple of the alignment so that
			// the storage is also aligned.
			SizeType remainder {toReturn % alignment};
			if (remainder != 0)
				toReturn += alignment - remainder;

			return toReturn;
		}

		static constexpr SizeType storageSize() {
			return minimumBlockSize * blockCount;
		}

		static constexpr SizeType totalSize() {
			return getMetaDataSize() + storageSize();
		}


	public:
		static_assert(minimumBlockSize % alignment == 0,
		              "Minimum block size must be divisible by the alignment");

		using ElementType = unsigned char;
		using ArrayType   = Array<ElementType, totalSize()>;

		static constexpr
		SizeType blockSize {std::max(alignment, minimumBlockSize)};

		constexpr BitmappedBlock() : BitmappedBlock(ArrayType {}) {}

		BitmappedBlock(ArrayType array) :
			array_                 {std::move(array)},
			lastInsertionMetaByte_ {0} {

			for (std::size_t i = 0; i < getMetaDataSize(); ++i) {
				array_[i] = 0;
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
			// Increase size to be a multiple of the block size.
			// If the size is 0, a block is still allocated.
			std::size_t remainder {size % minimumBlockSize};
			if (remainder != 0 || size == 0)
				size += minimumBlockSize - remainder;

			// The amount of blocks that must be reserved for the allocation.
			std::size_t blocksRequired {size / minimumBlockSize};

			// Total amount of blocks searched so far.
			// Allocation fails if this reaches blockCount.
			std::size_t blocksSearched    {0};

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
							std::size_t firstIndex {blocksSearched - blocksRequired + 1};
							std::size_t lastIndex  {blocksSearched};

							std::size_t index {firstIndex};

							while (index <= lastIndex) {
								setMetaBit(index);

								++index;
							}

							if (index == minimumBlockSize)
								lastInsertionMetaByte_ = 0;
							else
								lastInsertionMetaByte_ = index;

							return {
								array_.data() + getMetaDataSize() + (firstIndex * minimumBlockSize),
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
				if (byte == getMetaDataSize()) {
					end = getMetaDataSize();
					byte = 0;
				}

			}

			return {nullptr, 0};
		}

		void deallocate(RawBlock block) {
			auto ptr = static_cast<Pointer>(block.getPtr());

			// The amount of blocks it takes up.
			std::size_t blocks     {block.getSize() / minimumBlockSize};
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
			  ptr <  array_.data() + totalSize()
			);
		}


	private:
		using Pointer = ElementType *;
		
		SizeType getBlockIndex(Pointer blockPtr) {
			return (blockPtr - array_.data() - getMetaDataSize()) / minimumBlockSize;
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
			ElementType & metaPtr {array_[metaIndex]};
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
			ElementType & metaRef {array_[metaIndex]};
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
			ElementType & metaRef {array_[metaIndex]};
			metaRef &= ~(1 << metaBitIndex);
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
