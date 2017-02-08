#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_BITMAPPED_BLOCK_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_BITMAPPED_BLOCK_H

#define BRIDGERRHOLT_BITMAPPED_BLOCK_SET_NEXT_BYTE_ALLOCATION
//#define BRIDGERRHOLT_BITMAPPED_BLOCK_SET_NEXT_BYTE_DEALLOCATION

#include <vector>
#include <bitset>
#include <cstddef>
#include <climits>
#include <limits>
#include <utility>
#include <cassert>

#include "traits/traits.h"
#include "common/common_types.h"
#include "wrappers/allocator_wrapper.h"
#include "common/round_up_to_multiple.h"

namespace bridgerrholt {
	namespace allocators {

class BitmappedBlockArrayElement
{
	public:
		void clearBits() { data_ = 0; }

		int getBit(std::size_t place) const {
			return static_cast<int>((data_ >> place) & 1);
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

template <std::size_t> class BitmappedBlockData;

using ElementType = BitmappedBlockArrayElement;
static constexpr std::size_t elementSize {sizeof(ElementType)};
static constexpr std::size_t elementSizeBits {elementSize * CHAR_BIT};

/// An allocator with memory segmented into blocks and meta data before the
/// blocks telling whether each is occupied or not.
/// This is a 1 bit per block overhead.
template <class t_Policy>
class alignas(t_Policy::alignment) BitmappedBlock : private t_Policy
{
	public:
		using Policy = t_Policy;

		//using ArrayType = Array<ElementType, getElementCount()>;


		/// Possibly undefined behavior if the swap function for the array requires
		/// the copying of elements.
		friend void swap(BitmappedBlock & first, BitmappedBlock & second) {
			using std::swap;

			swap(static_cast<Policy&>(first),  static_cast<Policy&>(second));
			swap(first.lastInsertionMetaByte_, second.lastInsertionMetaByte_);
		}

		BitmappedBlock() : BitmappedBlock(Policy {}) {
			std::cout << "BitmappedBlock()\n";
		}

		constexpr BitmappedBlock(Policy policy) :
			Policy                 {std::move(policy)},
			lastInsertionMetaByte_ {0} {

			std::cout << "BitmappedBlock(Policy) " << noexcept(this->getData().getBlockSize()) << "\n";

			auto const end = this->getData().getMetaDataSize();
			for (std::size_t i = 0; i < end; ++i) {
				this->getArray()[i].clearBits();
			}


			/*std::cout << "minimumBlockSize  = " << minimumBlockSize << '\n'
			          << "blockCount = " << blockCount << '\n'
			          << "minimumAlignment  = " << minimumAlignment << "\n\n";

			std::cout << "Meta data size: " << getMetaDataSize() << '\n'
			          << "Storage size:   " << storageSize() << '\n'
			          << "Total size:     " << totalSize() << '\n'
			          << "Efficiency:     " << efficiency() << '\n';*/
		}

		BitmappedBlock(BitmappedBlock && other) : Policy(static_cast<Policy>(other)) {
			std::cout << "BitmappedBlock(BitmappedBlock &&)\n";
			swap(*this, other);
		}

		BitmappedBlock(BitmappedBlock const &) = delete;


		bool isEmpty() {
			for (std::size_t i {0}; i < Policy::getBlockCount(); ++i) {
				if (getMetaBit(i) == 1)
					return false;
			}

			return true;
		}

		bool isFull() {
			for (std::size_t i {0}; i < this->getBlockCount(); ++i) {
				if (getMetaBit(i) == 0)
					return false;
			}

			return true;
		}

		template <class T>
		void printBits() const {
			printMeta();

			for (std::size_t i {0}; i < this->getBlockCount(); ++i) {
				std::cout << ' ' << *reinterpret_cast<T const *>(getBlockPtr(i));
			}

			std::cout << std::endl;
		}

		void printMeta() const {
			for (std::size_t i {0}; i < this->getBlockCount(); ++i) {
				std::cout << getMetaBit(i);
			}
		}

		SizeType countUsedBlocks() {
			SizeType count {0};
			for (std::size_t i {0}; i < this->getBlockCount(); ++i) {
				if (getMetaBit(i) == 1)
					++count;
			}

			return count;
		}

		RawBlock allocate(SizeType size) {
			// An allocation takes place even if the size is 0.
			if (size == 0) {
				size = this->getData().getBlockSize();
			}
			else {
				// The size must take up a whole number of blocks.
				size = common::roundUpToMultiple(size, this->getData().getBlockSize());
			}

			// The amount of blocks that must be reserved for the allocation.
			std::size_t blocksRequired {size / this->getData().getBlockSize()};


			// Total amount of blocks searched so far.
			// Allocation fails if this reaches blockCount.
			std::size_t blocksSearched {0};

			// Size of current group of contiguous unset bits.
			// Allocation will succeed once this reaches blocksRequired.
			std::size_t currentRegionSize {0};

			// Loop through each bit in the meta data.
			std::size_t byte {lastInsertionMetaByte_};
			std::size_t end  {this->getData().getMetaDataSize()};

			while (byte < end) {
				for (std::size_t bit {0}; bit < elementSizeBits; ++bit) {

					if (getMetaBit(byte, bit) == 0) {

						++currentRegionSize;

						// Allocation will be successful.
						if (currentRegionSize == blocksRequired) {
							// Set all the meta bits to indicate occupation of the blocks.
							std::size_t lastIndex  {getBlockIndex(byte, bit)};
							std::size_t firstIndex {lastIndex - blocksRequired + 1};
							std::size_t index      {firstIndex};

							while (index <= lastIndex) {
								setMetaBit(index);

								++index;
							}

#ifdef BRIDGERRHOLT_BITMAPPED_BLOCK_SET_NEXT_BYTE_ALLOCATION
							// index should be 1 past lastIndex.
							if (index == this->getData().getBlockCount())
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
					if (blocksSearched == this->getData().getBlockCount())
						return RawBlock::makeNullBlock();
				}


				++byte;
				if (byte == this->getData().getMetaDataSize()) {
					end = lastInsertionMetaByte_;
					byte = 0;
				}
			}

			return RawBlock::makeNullBlock();
		}

		void deallocate(RawBlock block) {
			auto ptr = static_cast<Pointer>(block.getPtr());

			// The amount of blocks it takes up.
			std::size_t blocks          {block.getSize() / this->getData().getBlockSize()};
			std::size_t blockIndexStart {getBlockIndex(ptr)};
			std::size_t objectEnd       {blockIndexStart + blocks};

			std::size_t blockIndex {blockIndexStart};
			while (blockIndex < objectEnd) {
				/*if (getMetaBit(blockIndex) != 1)
					throw std::runtime_error("Bad deallocate");*/
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
				ptr >= this->getArray().data() + this->getData().getMetaDataSize() &&
			  ptr <  this->getArray().data() + this->getData().getTotalSize()
			);
		}


	private:
		using Pointer      = ElementType       *;
		using ConstPointer = ElementType const *;


		SizeType getBlockIndex(Pointer blockPtr) const {
			auto normal = blockPtr - this->getArray().data() - this->getData().getMetaDataSize();
			return (normal / this->getData().getBlockSize());
		}


		constexpr static SizeType getMetaIndex(SizeType blockIndex) {
			return blockIndex / elementSizeBits;
		}

		constexpr static unsigned getMetaBitIndex(SizeType blockIndex) {
			return static_cast<unsigned>(blockIndex % elementSizeBits);
		}

		constexpr static SizeType getBlockIndex(SizeType metaIndex,
		                                        SizeType metaBitIndex) {
			return metaIndex * elementSizeBits + metaBitIndex;
		}


		ConstPointer getBlockPtr(SizeType blockIndex) const {
			return this->getArray().data() + getBlockPtrOffset(blockIndex);
		}

		Pointer getBlockPtr(SizeType blockIndex) {
			return this->getArray().data() + getBlockPtrOffset(blockIndex);
		}

		SizeType getBlockPtrOffset(SizeType blockIndex) const {
			return this->getData().getMetaDataSize() + (blockIndex * this->getData().getBlockSize());
		}


		int getMetaBit(SizeType blockIndex) const {
			return getMetaBit(getMetaIndex(blockIndex), getMetaBitIndex(blockIndex));
		}

		int getMetaBit(SizeType metaIndex, SizeType metaBitIndex) const {
			return this->getArray()[metaIndex].getBit(metaBitIndex);
		}


		void setMetaBit(SizeType blockIndex) {
			setMetaBit(getMetaIndex(blockIndex), getMetaBitIndex(blockIndex));
		}

		void setMetaBit(SizeType metaIndex, SizeType metaBitIndex) {
			this->getArray()[metaIndex].setBit(metaBitIndex);
		}


		void unsetMetaBit(SizeType blockIndex) {
			unsetMetaBit(getMetaIndex(blockIndex), getMetaBitIndex(blockIndex));
		}

		void unsetMetaBit(SizeType metaIndex, unsigned metaBitIndex) {
			this->getArray()[metaIndex].unsetBit(metaBitIndex);
		}


		// Calculates how efficiently memory space is used.
		double efficiency() {
			return static_cast<double>(Policy::getBlockCount()) / (Policy::getMetaDataSize() * elementSizeBits);
		}

		std::size_t lastInsertionMetaByte_;
};

/*
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
*/





template <std::size_t alignment>
class BitmappedBlockData
{
	public:
		constexpr BitmappedBlockData(SizeType minimumBlockSize, SizeType blockCount) :
			blockSize_  {std::max(alignment, minimumBlockSize)},
			blockCount_ {blockCount} {

		}

		//  Size access operations
		// Returns the amount of bytes that the meta header data requires.
		constexpr SizeType getMetaDataSize() const {
			// Meta data overhead is 1 bit per block.
			SizeType toReturn {
				blockCount_ / elementSizeBits_
			};

			// If the meta data size was rounded down due to integer casting,
			// round it up instead.
			if (blockCount_ % elementSizeBits_ != 0)
				toReturn += 1;

			// The meta data size must be a multiple of the alignment so that
			// the storage is also aligned.
			// Note: This shouldn't be a problem because elementSize should be
			// a factor of the alignment anyway.
			toReturn = common::roundUpToMultiple(toReturn, alignment);

			return toReturn;
		}

		// Returns the amount of bytes that the block list requires.
		constexpr SizeType getStorageSize() const {
			return blockSize_ * blockCount_;
		}

		// Returns the total amount of bytes required.
		constexpr SizeType getTotalSize() const {
			return getMetaDataSize() + getStorageSize();
		}

		constexpr SizeType getElementCount() const {
			return common::roundUpToMultiple(
				getTotalSize(), elementSize_
			) / elementSize_;
		}

		constexpr SizeType getBlockSize()  const { return blockSize_; }
		constexpr SizeType getBlockCount() const { return blockCount_; }


	private:
		SizeType blockSize_;
		SizeType blockCount_;

		static constexpr
		std::size_t elementSize_ {elementSize};

		static constexpr
		std::size_t elementSizeBits_ {elementSizeBits};
};


template <template <class T> class ArrayType, class T>
class ArrayRuntimeWrapper : public traits::RuntimeSizedArray<ArrayType, T>
{
	public:
		ArrayRuntimeWrapper(SizeType size) :
			traits::RuntimeSizedArray<ArrayType, T> (size) {}
};


template <template <class T, SizeType size> class ArrayType, class T,
	SizeType minimumBlockSize,
	SizeType blockCount,
	std::size_t alignment>
class ArrayTemplateWrapper : public ArrayType<T,
                                    BitmappedBlockData<alignment>(
	                                    minimumBlockSize, blockCount
                                    ).getElementCount()>
{
	public:
		static_assert((minimumBlockSize % alignment == 0),
		              "Minimum block size must be divisible by the alignment");

		ArrayTemplateWrapper() {}
};


template <template <class T> class CoreArray,
  std::size_t t_alignment>
class RuntimePolicy
{
	public:
		static constexpr std::size_t alignment {t_alignment};

		using ArrayType = ArrayRuntimeWrapper<CoreArray, BitmappedBlockArrayElement>;
		using DataType  = BitmappedBlockData<alignment>;

		using ArrayReturn      = ArrayType &;
		using ArrayConstReturn = ArrayType const &;
		using DataReturn       = DataType  const &;

		friend void swap(RuntimePolicy & first, RuntimePolicy & second) {
			using std::swap;

			swap(first.data_, second.data_);
			swap(first.array_, second.array_);
		}

		RuntimePolicy(SizeType minimumBlockSize, SizeType blockCount) :
			data_ {minimumBlockSize, blockCount},
			array_{data_.getElementCount()} {}

		ArrayReturn      getArray()       { return array_; }
		ArrayConstReturn getArray() const { return array_; }

		DataReturn getData() const {
			return data_;
		}

	private:
		DataType  data_;
		ArrayType array_;
};


template <template <class T, SizeType size> class CoreArray,
	SizeType    minimumBlockSize,
	SizeType    blockCount,
	std::size_t t_alignment>
class TemplatePolicy
{
	public:
		static constexpr std::size_t alignment {t_alignment};

		using ArrayType = ArrayTemplateWrapper<
			CoreArray, BitmappedBlockArrayElement, minimumBlockSize, blockCount, alignment
		>;

		using DataType = BitmappedBlockData<alignment>;

		using ArrayReturn      = ArrayType &;
		using ArrayConstReturn = ArrayType const &;
		using DataReturn       = DataType  const;

		ArrayReturn      getArray()       { return array_; }
		ArrayConstReturn getArray() const { return array_; }

		static constexpr DataReturn getData() {
			return {minimumBlockSize, blockCount};
		}

		static constexpr DataReturn getDataS() {
			return {minimumBlockSize, blockCount};
		}

	private:
		ArrayType array_;
};




template <template <class T> class ArrayType,
	std::size_t alignment = alignof(std::max_align_t)>
using BitmappedBlockRuntime =
BitmappedBlock<RuntimePolicy<ArrayType, alignment> >;


template <template <class T, SizeType size> class CoreArray,
	std::size_t minimumBlockSize,
	std::size_t blockCount,
	std::size_t alignment = alignof(std::max_align_t)>
using BitmappedBlockTemplate =
BitmappedBlock<TemplatePolicy<
	CoreArray, minimumBlockSize, blockCount, alignment>
>;

template <class Allocator>
constexpr BitmappedBlockData<Allocator::Policy::alignment> getBitmappedData() {
	return Allocator::ArrayType::getData();
}


	}
}

#endif
