#ifndef BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_BITMAPPED_BLOCK_H
#define BRIDGERRHOLT_ALLOCATOR_TEST_ALLOCATORS_BITMAPPED_BLOCK_H

#define BRIDGERRHOLT_BITMAPPED_BLOCK_SET_NEXT_BYTE_ALLOCATION
#define BRIDGERRHOLT_BITMAPPED_BLOCK_SET_NEXT_BYTE_DEALLOCATION
#define BRIDGERRHOLT_BITMAPPED_BLOCK_USE_ITERATOR

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

		int getBit(std::size_t place) const {
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
	SizeType    blockCountT,
  std::size_t alignment = alignof(std::max_align_t)>
class alignas(alignment) BitmappedBlock
{
	public:
		static constexpr SizeType blockCount {blockCountT};

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

		/// Possibly undefined behavior if the swap function for the array requires
		/// the copying of elements.
		friend void swap(BitmappedBlock & first, BitmappedBlock & second) {
			using std::swap;

			swap(first.array_,                 second.array_);
			swap(first.lastInsertionMetaByte_, second.lastInsertionMetaByte_);
		}

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

		BitmappedBlock(BitmappedBlock && other) :
			BitmappedBlock {} {
			swap(*this, other);
		}

		BitmappedBlock(BitmappedBlock const &) = delete;


		bool isEmpty() {
			for (std::size_t i {0}; i < blockCount; ++i) {
				if (getMetaBit(i) == 1)
					return false;
			}

			return true;
		}

		template <class T>
		void printBits() const {
			for (std::size_t i {0}; i < blockCount; ++i) {
				std::cout << getMetaBit(i);
			}

			for (std::size_t i {0}; i < blockCount; ++i) {
				std::cout << ' ' << *reinterpret_cast<T const *>(getBlockPtr(i));
			}

			std::cout << std::endl;
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

#ifdef BRIDGERRHOLT_BITMAPPED_BLOCK_USE_ITERATOR

			SizeType end {getMetaDataSize()};
			auto i = metaBegin();

			while (i != end) {
				if (i.get() == 0) {
					++currentRegionSize;

					if (currentRegionSize == blocksRequired) {
						std::size_t firstIndex{blocksSearched - blocksRequired + 1};

						while (blocksRequired > 0) {
							i.set();

							++i;
							--blocksRequired;
						}

#ifdef BRIDGERRHOLT_BITMAPPED_BLOCK_SET_NEXT_BYTE_ALLOCATION
						if (i == blockCount)
							lastInsertionMetaByte_ = 0;
						else
							lastInsertionMetaByte_ = i.getByte();
#endif
						return {getBlockPtr(firstIndex), size};
					}
				}
				else {
					currentRegionSize = 0;
				}

				++blocksSearched;
				std::cout << blocksSearched << " " << blockCount << '\n';
				if (blocksSearched == blockCount)
					return RawBlock::makeNullBlock();

				++i;

				if (i.getByte() == getMetaDataSize()) {
					end = lastInsertionMetaByte_;
					i.setByte(0);
				}
			}

#else
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

#ifdef BRIDGERRHOLT_BITMAPPED_BLOCK_SET_NEXT_BYTE_ALLOCATION
							if (index == blockCount)
								lastInsertionMetaByte_ = 0;
							else
								lastInsertionMetaByte_ = getMetaIndex(index);
#endif

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
					end = lastInsertionMetaByte_;
					byte = 0;
				}
			}
#endif

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

#ifdef BRIDGERRHOLT_BITMAPPED_BLOCK_SET_NEXT_BYTE_DEALLOCATION
			lastInsertionMetaByte_ = getMetaIndex(getBlockIndex(ptr));
#endif

		}

		bool owns(RawBlock block) {
			auto ptr = static_cast<Pointer>(block.getPtr());

			return (
				ptr >= array_.data() + getMetaDataSize() &&
			  ptr <  array_.data() + getTotalSize()
			);
		}


	private:
		using Pointer      = ElementType *;
		using ConstPointer = ElementType const *;

		class MetaBitIterator {
			public:
				constexpr MetaBitIterator(BitmappedBlock & allocator,
				                          SizeType         byteStart,
				                          unsigned         bitStart = 0) :
					allocator_ {allocator},
					byte_      {byteStart},
					bit_       {bitStart} {}

				SizeType getByte() const { return byte_; }
				unsigned getBit()  const { return bit_; }

				void setByte(SizeType byteIndex) { byte_ = byteIndex; }

				int get() const { return allocator_.getMetaBit(byte_, bit_); }

				void set()   { allocator_.setMetaBit  (byte_, bit_); }
				void unset() { allocator_.unsetMetaBit(byte_, bit_); }

				bool incrementBit() {
					++bit_;
					if (bit_ == CHAR_BIT) {
						bit_ = 0;
						return false;
					}
					else {
						return true;
					}
				}

				bool incrementByte() {
					++byte_;
					if (byte_ == end_) {
						byte_ = 0;
						return false;
					}
					else {
						return true;
					}
				}



				void setEnd(SizeType end) {
					end_ = end;
				}


				MetaBitIterator & operator++() {
					++bit_;
					if (bit_ == CHAR_BIT) {
						bit_ = 0;
						++byte_;
					}

					return *this;
				}

				MetaBitIterator & operator++(int) {
					auto temp = *this;
					++(*this);

					return temp;
				}

				bool operator==(MetaBitIterator const & other) {
					return (this->byte_ == other.byte_ &&
					        this->bit_  == other.bit_);
				}

				bool operator!=(MetaBitIterator const & other) {
					return !((*this) == other);
				}

				bool operator==(SizeType blockIndex) {
					return (allocator_.getMetaIndex(blockIndex) == byte_ &&
						      allocator_.getMetaBitIndex(blockIndex) == bit_);
				}

				bool operator!=(SizeType blockIndex) {
					return !((*this) == blockIndex);
				}


			private:
				BitmappedBlock & allocator_;
				SizeType         byte_;
				unsigned         bit_;
				SizeType         end_;
		};


		MetaBitIterator metaBegin() {
			return {*this, lastInsertionMetaByte_};
		}

		constexpr MetaBitIterator metaEnd() {
			return {*this, getMetaIndex(blockCount - 1), getMetaBitIndex(blockCount - 1)};
		}

		SizeType getBlockIndex(Pointer blockPtr) const {
			return (blockPtr - array_.data() - getMetaDataSize()) / blockSize;
		}


		constexpr static SizeType getMetaIndex(SizeType blockIndex) {
			return blockIndex / CHAR_BIT;
		}

		constexpr static unsigned getMetaBitIndex(SizeType blockIndex) {
			return static_cast<unsigned>(blockIndex % CHAR_BIT);
		}


		ConstPointer getBlockPtr(SizeType blockIndex) const {
			return array_.data() + getBlockPtrOffset(blockIndex);
		}

		Pointer getBlockPtr(SizeType blockIndex) {
			return array_.data() + getBlockPtrOffset(blockIndex);
		}

		SizeType getBlockPtrOffset(SizeType blockIndex) const {
			return getMetaDataSize() + (blockIndex * blockSize);
		}


		int getMetaBit(SizeType blockIndex) const {
			return getMetaBit(getMetaIndex(blockIndex), getMetaBitIndex(blockIndex));
		}

		int getMetaBit(SizeType metaIndex, SizeType metaBitIndex) const {
			return array_[metaIndex].getBit(metaBitIndex);
		}


		void setMetaBit(SizeType blockIndex) {
			setMetaBit(getMetaIndex(blockIndex), getMetaBitIndex(blockIndex));
		}

		void setMetaBit(SizeType metaIndex, SizeType metaBitIndex) {
			array_[metaIndex].setBit(metaBitIndex);
		}


		void unsetMetaBit(SizeType blockIndex) {
			unsetMetaBit(getMetaIndex(blockIndex), getMetaBitIndex(blockIndex));
		}

		void unsetMetaBit(SizeType metaIndex, unsigned metaBitIndex) {
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
