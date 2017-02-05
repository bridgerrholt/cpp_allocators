#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_BITMAPPED_BLOCK_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_BITMAPPED_BLOCK_H

#define BRIDGERRHOLT_BITMAPPED_BLOCK_SET_NEXT_BYTE_ALLOCATION
//#define BRIDGERRHOLT_BITMAPPED_BLOCK_SET_NEXT_BYTE_DEALLOCATION
//#define BRIDGERRHOLT_BITMAPPED_BLOCK_USE_ITERATOR

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


using ElementType = BitmappedBlockArrayElement;
static constexpr std::size_t elementSize {sizeof(ElementType)};
static constexpr std::size_t elementSizeBits {elementSize * CHAR_BIT};

/// An allocator with memory segmented into blocks and meta data before the
/// blocks telling whether each is occupied or not.
/// This is a 1 bit per block overhead.
template <class PolicyT>
class alignas(PolicyT::alignment) BitmappedBlock
{
	public:
		using Policy    = PolicyT;
		using ArrayType = typename Policy::template ArrayType<ElementType>;



		//using ArrayType = Array<ElementType, getElementCount()>;


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

			std::cout << "BitmappedBlock()\n";

			for (std::size_t i = 0; i < array_.getMetaDataSize(); ++i) {
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
			for (std::size_t i {0}; i < array_.getBlockCount(); ++i) {
				if (getMetaBit(i) == 1)
					return false;
			}

			return true;
		}

		bool isFull() {
			for (std::size_t i {0}; i < array_.getBlockCount(); ++i) {
				if (getMetaBit(i) == 0)
					return false;
			}

			return true;
		}

		template <class T>
		void printBits() const {
			printMeta();

			for (std::size_t i {0}; i < array_.getBlockCount(); ++i) {
				std::cout << ' ' << *reinterpret_cast<T const *>(getBlockPtr(i));
			}

			std::cout << std::endl;
		}

		void printMeta() const {
			for (std::size_t i {0}; i < array_.getBlockCount(); ++i) {
				std::cout << getMetaBit(i);
			}
		}

		SizeType countUsedBlocks() {
			SizeType count {0};
			for (std::size_t i {0}; i < array_.getBlockCount(); ++i) {
				if (getMetaBit(i) == 1)
					++count;
			}

			return count;
		}

		RawBlock allocate(SizeType size) {
			// An allocation takes place even if the size is 0.
			if (size == 0) {
				size = array_.getBlockSize();
			}
			else {
				// The size must take up a whole number of blocks.
				size = common::roundUpToMultiple(size, array_.getBlockSize());
			}

			// The amount of blocks that must be reserved for the allocation.
			std::size_t blocksRequired {size / array_.getBlockSize()};



			// Total amount of blocks searched so far.
			// Allocation fails if this reaches blockCount.
			std::size_t blocksSearched {0};

			// Size of current group of contiguous unset bits.
			// Allocation will succeed once this reaches blocksRequired.
			std::size_t currentRegionSize {0};

#ifdef BRIDGERRHOLT_BITMAPPED_BLOCK_USE_ITERATOR

			SizeType end {getMetaDataSize()};
			auto i = metaBegin();

			do {
				if (i.get() == 0) {
					++currentRegionSize;

					if (currentRegionSize == blocksRequired) {
						auto iteratorStart = i.SimpleMetaIterator::operator-(
							static_cast<long long>(blocksRequired - 1)
						);

						auto iterator = iteratorStart;

						while (iterator != i) {
							iterator.set();

							++iterator;
						}

						iterator.set();

						//printMeta();
						//std::cout << std::endl;

#ifdef BRIDGERRHOLT_BITMAPPED_BLOCK_SET_NEXT_BYTE_ALLOCATION
						if (i == blockCount)
							lastInsertionMetaByte_ = 0;
						else
							lastInsertionMetaByte_ = i.getByte();
#endif
						/*auto countAfter = countUsedBlocks();
						if (countAfter != countBefore + blocksRequired) {
							throw std::runtime_error("Failed allocation");
						}*/
						return {getBlockPtr(iteratorStart), size};
					}
				}
				else {
					currentRegionSize = 0;
				}

				++blocksSearched;
				if (blocksSearched == blockCount)
					return RawBlock::makeNullBlock();

			} while (i.increment());

#else
			// Loop through each bit in the meta data.
			std::size_t byte {lastInsertionMetaByte_};
			std::size_t end  {array_.getMetaDataSize()};

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
							if (index == array_.getBlockCount())
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
					if (blocksSearched == array_.getBlockCount())
						return RawBlock::makeNullBlock();
				}


				++byte;
				if (byte == array_.getMetaDataSize()) {
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
			std::size_t blocks          {block.getSize() / array_.getBlockSize()};
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
				ptr >= array_.data() + array_.getMetaDataSize() &&
			  ptr <  array_.data() + array_.getTotalSize()
			);
		}


	private:
		using Pointer      = ElementType       *;
		using ConstPointer = ElementType const *;

		class SimpleMetaIterator {
			public:
				constexpr SimpleMetaIterator(BitmappedBlock & allocator,
				                             SizeType         byteStart,
																		 unsigned         bitStart = 0) :
					allocator_ {allocator},
					byte_      {byteStart},
				  bit_       {bitStart} {}


				int get() const { return allocator_.getMetaBit(byte_, bit_); }

				void set()   { allocator_.setMetaBit  (byte_, bit_); }
				void unset() { allocator_.unsetMetaBit(byte_, bit_); }


				SizeType getByte() const { return byte_; }
				unsigned getBit()  const { return bit_; }

				BitmappedBlock & getAllocator() const { return allocator_; }


				SimpleMetaIterator & operator++() {
					incrementBit();

					return *this;
				}

				template <class T>
				SimpleMetaIterator operator+(T value) const {
					T newBit {static_cast<T>(bit_) + value};
					SizeType newByte {byte_ + (newBit / BitmappedBlock::elementSizeBits)};
					newBit %= BitmappedBlock::elementSizeBits;

					/*std::cout << "(" << byte_ << " " << bit_ << ") + " << value <<
					          " = (" << newByte << " " << newBit << ")\n";*/

					return {allocator_, newByte, static_cast<unsigned>(newBit)};
				}

				template <class T>
				SimpleMetaIterator operator-(T value) const {
					SizeType newByte {byte_ - (value / BitmappedBlock::elementSizeBits)};
					auto newBit = bit_;

					T remaining = value % BitmappedBlock::elementSizeBits;
					if (remaining > newBit) {
						newBit += 8 - remaining;
						newByte -= 1;
					}
					else {
						newBit -= remaining;
					}

					return {allocator_, newByte, newBit};
				}

				bool operator==(SimpleMetaIterator const & other) const {
					return (byte_ == other.byte_ &&
						      bit_  == other.bit_);
				}

				bool operator!=(SimpleMetaIterator const & other) const {
					return !(*this == other);
				}

				bool operator==(SizeType blockIndex) {
					return (BitmappedBlock::getMetaIndex   (blockIndex) == byte_ &&
					        BitmappedBlock::getMetaBitIndex(blockIndex) == bit_);
				}

				bool operator!=(SizeType blockIndex) {
					return !((*this) == blockIndex);
				}


			protected:
				bool incrementBit() {
					++bit_;

					if (bit_ == BitmappedBlock::elementSizeBits) {
						bit_ = 0;
						++byte_;
						return false;
					}
					else {
						return true;
					}
				}


			private:
				BitmappedBlock & allocator_;
				SizeType         byte_;
				unsigned         bit_;
		};


		class MetaBitIterator : public SimpleMetaIterator {
			public:
				constexpr MetaBitIterator(BitmappedBlock & allocator,
				                          SizeType         byteEnd,
				                          SizeType         byteStart,
				                          unsigned         bitStart = 0) :
				  SimpleMetaIterator {allocator, byteStart, bitStart},
					byteEnd_   {byteEnd},
					byteStart_ {byteStart} {}

				bool increment() {
					if (!this->incrementBit()) {
						if (this->getByte() == byteEnd_) {
							byteEnd_ = byteStart_;
						}

						return (this->getByte() != byteEnd_);
					}

					return (*this != this->getAllocator().metaEnd());
				}


				MetaBitIterator & operator++() {
					SimpleMetaIterator::operator++();

					return *this;
				}

				bool operator==(SimpleMetaIterator const & other) const {
					return (SimpleMetaIterator::operator==(other));
				}

				bool operator!=(SimpleMetaIterator const & other) const {
					return !(*this == other);
				}

				bool operator==(SizeType blockIndex) {
					return (SimpleMetaIterator::operator==(blockIndex));
				}

				bool operator!=(SizeType blockIndex) {
					return !(*this == blockIndex);
				}


			private:
				SizeType         byteEnd_;
				SizeType         byteStart_;
		};


		MetaBitIterator metaBegin() {
			return {*this, array_.getMetaDataSize(), lastInsertionMetaByte_};
		}

		SizeType metaEnd() const {
			return array_.getBlockCount();
		}

		SizeType getBlockIndex(Pointer blockPtr) const {
			auto normal = blockPtr - array_.data() - array_.getMetaDataSize();
			return (normal / array_.getBlockSize());
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
			return array_.data() + getBlockPtrOffset(blockIndex);
		}

		Pointer getBlockPtr(SizeType blockIndex) {
			return array_.data() + getBlockPtrOffset(blockIndex);
		}

		Pointer getBlockPtr(SimpleMetaIterator const & iterator) {
			return getBlockPtr(
				iterator.getByte() * elementSizeBits + iterator.getBit()
			);
		}

		SizeType getBlockPtrOffset(SizeType blockIndex) const {
			return array_.getMetaDataSize() + (blockIndex * array_.getBlockSize());
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


		// Calculates how efficiently memory space is used.
		double efficiency() {
			return static_cast<double>(array_.getBlockCount()) / (array_.getMetaDataSize() * elementSizeBits);
		}

		ArrayType   array_;
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
		SizeType getMetaDataSize() const {
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
		SizeType getStorageSize() const {
			return blockSize_ * blockCount_;
		}

		// Returns the total amount of bytes required.
		SizeType getTotalSize() const {
			return getMetaDataSize() + getStorageSize();
		}

		SizeType getElementCount() const {
			return common::roundUpToMultiple(
				getTotalSize(), elementSize_
			) / elementSize_;
		}

		SizeType getBlockSize()  const { return blockSize_; }
		SizeType getBlockCount() const { return blockCount_; }



	private:
		SizeType const blockSize_;
		SizeType const blockCount_;

		static constexpr
		std::size_t elementSize_ {elementSize};

		static constexpr
		std::size_t elementSizeBits_ {elementSizeBits};
};


template <template <class T> class ArrayType, class T,
	std::size_t alignment>
class ArrayRuntimeWrapper : public BitmappedBlockData<alignment>
{
	public:
		ArrayRuntimeWrapper(SizeType blockSize, SizeType blockCount) :
			BitmappedBlockData<alignment>  {blockSize, blockCount},
			array_                         (BitmappedBlockData<alignment>::getElementCount()) {}

		T       & operator[](SizeType i)       { return array_[i]; }
		T const & operator[](SizeType i) const { return array_[i]; }

	private:
		traits::RuntimeSizedArray<ArrayType, T> array_;
};


template <template <class T, SizeType size> class ArrayType, class T,
	SizeType minimumBlockSize,
	SizeType blockCount,
	std::size_t alignment>
class ArrayTemplateWrapper
{
	public:
		static_assert((minimumBlockSize % alignment == 0),
		              "Minimum block size must be divisible by the alignment");

		ArrayTemplateWrapper() {}

		T       & operator[](SizeType i)       { return array_[i]; }
		T const & operator[](SizeType i) const { return array_[i]; }

		SizeType getMetaDataSize() const { return data_.getMetaDataSize(); }
		SizeType getStorageSize()  const { return data_.getStorageSize(); }
		SizeType getTotalSize()    const { return data_.getTotalSize(); }
		SizeType getElementCount() const { return data_.getElementCount(); }

		SizeType getBlockSize()  const { return data_.getBlockSize(); }
		SizeType getBlockCount() const { return data_.getBlockCount(); }

	private:
		static constexpr
		BitmappedBlockData<alignment> data_ {minimumBlockSize, blockCount};

		ArrayType<T, data_.getElementCount()> array_;
};


template <template <class T> class CoreArray,
  std::size_t t_alignment>
class RuntimePolicy
{
	public:
		static constexpr std::size_t alignment {t_alignment};

		template <class T>
		using ArrayType = ArrayRuntimeWrapper<CoreArray, T, alignment>;
};


template <template <class T, SizeType size> class CoreArray,
	std::size_t minimumBlockSize,
	std::size_t blockCount,
	std::size_t t_alignment>
class TemplatePolicy
{
	public:
		static constexpr std::size_t alignment {t_alignment};

		template <class T>
		using ArrayType = ArrayTemplateWrapper<
			CoreArray, T, minimumBlockSize, blockCount, alignment
		>;

	private:
		static constexpr BitmappedBlockData<alignment> data_ {minimumBlockSize, blockCount};
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


	}
}

#endif
