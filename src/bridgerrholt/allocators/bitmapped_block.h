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

template <std::size_t> class BitmappedBlockData;

using ElementType = BitmappedBlockArrayElement;
static constexpr std::size_t elementSize {sizeof(ElementType)};
static constexpr std::size_t elementSizeBits {elementSize * CHAR_BIT};

/// An allocator with memory segmented into blocks and meta data before the
/// blocks telling whether each is occupied or not.
/// This is a 1 bit per block overhead.
template <class PolicyT>
class alignas(PolicyT::alignment) BitmappedBlock : private PolicyT
{
	public:
		using Policy    = PolicyT;
		using ArrayType = typename Policy::ArrayType;

		constexpr SizeType getBlockSize() const { return array_.getBlockSize(); }

		//using ArrayType = Array<ElementType, getElementCount()>;


		/// Possibly undefined behavior if the swap function for the array requires
		/// the copying of elements.
		friend void swap(BitmappedBlock & first, BitmappedBlock & second) {
			using std::swap;

			swap(first.array_,                 second.array_);
			swap(first.lastInsertionMetaByte_, second.lastInsertionMetaByte_);
		}

		BitmappedBlock() : BitmappedBlock(Policy {}) {
			std::cout << "BitmappedBlock()\n";
		}

		constexpr BitmappedBlock(Policy policy) :
			Policy                 {std::move(policy)},
			array_                 {Policy::makeArray()},
			lastInsertionMetaByte_ {0} {

			std::cout << "BitmappedBlock(Policy) " << Policy::getData().getBlockSize() << "\n";

			for (std::size_t i = 0; i < Policy::getData().getMetaDataSize(); ++i) {
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

		BitmappedBlock(BitmappedBlock && other) {
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
			for (std::size_t i {0}; i < Policy::getBlockCount(); ++i) {
				if (getMetaBit(i) == 0)
					return false;
			}

			return true;
		}

		template <class T>
		void printBits() const {
			printMeta();

			for (std::size_t i {0}; i < Policy::getBlockCount(); ++i) {
				std::cout << ' ' << *reinterpret_cast<T const *>(getBlockPtr(i));
			}

			std::cout << std::endl;
		}

		void printMeta() const {
			for (std::size_t i {0}; i < Policy::getBlockCount(); ++i) {
				std::cout << getMetaBit(i);
			}
		}

		SizeType countUsedBlocks() {
			SizeType count {0};
			for (std::size_t i {0}; i < Policy::getBlockCount(); ++i) {
				if (getMetaBit(i) == 1)
					++count;
			}

			return count;
		}

		RawBlock allocate(SizeType size) {
			// An allocation takes place even if the size is 0.
			if (size == 0) {
				size = getBlockSize();
			}
			else {
				// The size must take up a whole number of blocks.
				size = common::roundUpToMultiple(size, getBlockSize());
			}

			// The amount of blocks that must be reserved for the allocation.
			std::size_t blocksRequired {size / getBlockSize()};



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
			std::size_t end  {Policy::getData().getMetaDataSize()};

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
							if (index == Policy::getData().getBlockCount())
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
					if (blocksSearched == Policy::getData().getBlockCount())
						return RawBlock::makeNullBlock();
				}


				++byte;
				if (byte == Policy::getData().getMetaDataSize()) {
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
			std::size_t blocks          {block.getSize() / getBlockSize()};
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
				ptr >= array_.data() + Policy::getMetaDataSize() &&
			  ptr <  array_.data() + Policy::getTotalSize()
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
					SizeType newByte {byte_ + (newBit / elementSizeBits)};
					newBit %= elementSizeBits;

					/*std::cout << "(" << byte_ << " " << bit_ << ") + " << value <<
					          " = (" << newByte << " " << newBit << ")\n";*/

					return {allocator_, newByte, static_cast<unsigned>(newBit)};
				}

				template <class T>
				SimpleMetaIterator operator-(T value) const {
					SizeType newByte {byte_ - (value / elementSizeBits)};
					auto newBit = bit_;

					T remaining = value % elementSizeBits;
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

					if (bit_ == elementSizeBits) {
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
			return {*this, Policy::getMetaDataSize(), lastInsertionMetaByte_};
		}

		SizeType metaEnd() const {
			return Policy::getData().getBlockCount();
		}

		SizeType getBlockIndex(Pointer blockPtr) const {
			auto normal = blockPtr - array_.data() - Policy::getData().getMetaDataSize();
			return (normal / getBlockSize());
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
			return Policy::getData().getMetaDataSize() + (blockIndex * getBlockSize());
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
			return static_cast<double>(Policy::getBlockCount()) / (Policy::getMetaDataSize() * elementSizeBits);
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
		constexpr SizeType getMetaDataSize() const {
			static_assert(true);

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
			static_assert(true);
			return blockSize_ * blockCount_;
		}

		// Returns the total amount of bytes required.
		constexpr SizeType getTotalSize() const {
			static_assert(true);
			return getMetaDataSize() + getStorageSize();
		}

		constexpr SizeType getElementCount() const {
			static_assert(true);
			return common::roundUpToMultiple(
				getTotalSize(), elementSize_
			) / elementSize_;
		}

		constexpr SizeType getBlockSize()  const { static_assert(true); return blockSize_; }
		constexpr SizeType getBlockCount() const { static_assert(true); return blockCount_; }


	private:
		SizeType const blockSize_;
		SizeType const blockCount_;

		static constexpr
		std::size_t elementSize_ {elementSize};

		static constexpr
		std::size_t elementSizeBits_ {elementSizeBits};
};


template <template <class T> class ArrayType, class T>
class ArrayRuntimeWrapper
{
	public:
		ArrayRuntimeWrapper(SizeType size) :
			array_ (size) {}

		T       & operator[](SizeType i)       { return array_[i]; }
		T const & operator[](SizeType i) const { return array_[i]; }

	private:
		traits::RuntimeSizedArray<ArrayType, T> array_;
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

		constexpr SizeType getMetaDataSize() const { return getData().getMetaDataSize(); }
		constexpr SizeType getStorageSize()  const { return getData().getStorageSize(); }
		constexpr SizeType getTotalSize()    const { return getData().getTotalSize(); }
		constexpr SizeType getElementCount() const { return getData().getElementCount(); }

		constexpr SizeType getBlockSize()  const { return getData().getBlockSize(); }
		constexpr SizeType getBlockCount() const { return getData().getBlockCount(); }

		static constexpr
		BitmappedBlockData<alignment> getData() {
			return {minimumBlockSize, blockCount};
		}
};


template <template <class T> class CoreArray,
  std::size_t t_alignment>
class RuntimePolicy
{
	public:
		static constexpr std::size_t alignment {t_alignment};

		using ArrayType = ArrayRuntimeWrapper<CoreArray, BitmappedBlockArrayElement>;

		using DataType = BitmappedBlockData<alignment>;

		RuntimePolicy(SizeType minimumBlockSize, SizeType blockCount) :
			data_ {minimumBlockSize, blockCount} {}

		ArrayType makeArray() const {
			return {data_.getElementCount()};
		}

		constexpr DataType const & getData() const {
			return data_;
		}

	private:
		DataType data_;

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

		constexpr ArrayType makeArray() const {
			return {};
		}

		constexpr DataType getData() const {
			return data_;
		}

	private:
		static constexpr DataType data_ {minimumBlockSize, blockCount};
};

template <template <class, SizeType> class CoreArray,
	SizeType    minimumBlockSize,
	SizeType    blockCount,
	std::size_t t_alignment>
typename
TemplatePolicy<
	CoreArray, minimumBlockSize,
	blockCount, t_alignment
>::DataType constexpr
TemplatePolicy<
	CoreArray, minimumBlockSize,
	blockCount, t_alignment
>::data_;



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
