#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_BITMAPPED_BLOCK_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_BITMAPPED_BLOCK_H

#define BRIDGERRHOLT_BITMAPPED_BLOCK_SET_NEXT_BYTE_ALLOCATION
//#define BRIDGERRHOLT_BITMAPPED_BLOCK_SET_NEXT_BYTE_DEALLOCATION

#include <iostream>
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
class alignas(t_Policy::alignment) BasicBitmappedBlock : private t_Policy
{
	public:
		using Policy = t_Policy;

		/// Possibly undefined behavior if the swap function for the array requires
		/// the copying of elements.
		friend void swap(BasicBitmappedBlock & first, BasicBitmappedBlock & second) {
			using std::swap;

			swap(static_cast<Policy&>(first),  static_cast<Policy&>(second));
			swap(first.lastInsertionMetaByte_, second.lastInsertionMetaByte_);
		}

		BasicBitmappedBlock() : BasicBitmappedBlock(Policy {}) {}

		constexpr BasicBitmappedBlock(Policy policy) :
			Policy                 {std::move(policy)},
			lastInsertionMetaByte_ {0} {

			auto const end = this->getMetaDataSize();
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

		BasicBitmappedBlock(BasicBitmappedBlock && other) :
			Policy(static_cast<Policy>(other)) {
			swap(*this, other);
		}

		BasicBitmappedBlock(BasicBitmappedBlock const &) = delete;


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
				size = this->getBlockSize();
			}
			else {
				// The size must take up a whole number of blocks.
				size = common::roundUpToMultiple(size, this->getBlockSize());
			}

			// The amount of blocks that must be reserved for the allocation.
			std::size_t blocksRequired {size / this->getBlockSize()};


			// Total amount of blocks searched so far.
			// Allocation fails if this reaches blockCount.
			std::size_t blocksSearched {0};

			// Size of current group of contiguous unset bits.
			// Allocation will succeed once this reaches blocksRequired.
			std::size_t currentRegionSize {0};

			// Loop through each bit in the meta data.
			std::size_t byte {lastInsertionMetaByte_};
			std::size_t end  {this->getMetaDataSize()};

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
							if (index == this->getBlockCount())
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
					if (blocksSearched == this->getBlockCount())
						return RawBlock::makeNullBlock();
				}


				++byte;
				if (byte == this->getMetaDataSize()) {
					end = lastInsertionMetaByte_;
					byte = 0;
				}
			}

			return RawBlock::makeNullBlock();
		}

		constexpr void deallocate(NullBlock) {}

		void deallocate(RawBlock block) {
			if (block.isNull()) return;

			auto ptr = static_cast<Pointer>(block.getPtr());

			// The amount of blocks it takes up.
			std::size_t blocks          {block.getSize() / this->getBlockSize()};
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
				ptr >= this->getArray().data() + this->getMetaDataSize() &&
			  ptr <  this->getArray().data() + this->getTotalSize()
			);
		}


	private:
		using Pointer      = ElementType       *;
		using ConstPointer = ElementType const *;


		SizeType getBlockIndex(Pointer blockPtr) const {
			auto normal =
				blockPtr - this->getArray().data() - this->getMetaDataSize();
			return (normal / this->getBlockSize());
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
			return this->getMetaDataSize() + (blockIndex * this->getBlockSize());
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
			return static_cast<double>(Policy::getBlockCount())
			       / (Policy::getMetaDataSize() * elementSizeBits);
		}

		std::size_t lastInsertionMetaByte_;
};



class BitmappedBlock
{
	public:
		/// Contains values regarding information about size.
		/// Satisfies LiteralType.
		template <std::size_t alignment>
		class Attributes {
			public:
				/// Primary constructor.
				constexpr Attributes(SizeType minimumBlockSize,
                             SizeType blockCount) :
					blockSize_    {std::max(alignment, minimumBlockSize)},
					blockCount_   {blockCount},
					metaDataSize_ {calcMetaDataSize()} {}

				/// Calculates the amount of bytes that the blocks require.
				constexpr SizeType getStorageSize() const {
					return getBlockSize() * getBlockCount();
				}

				/// Calculates the total amount of bytes required for the array.
				constexpr SizeType getTotalSize() const {
					return getMetaDataSize() + getStorageSize();
				}

				/// Calculates the total amount of elements required for the array.
				constexpr SizeType getElementCount() const {
					return common::roundUpToMultiple(
						getTotalSize(), elementSize_
					) / elementSize_;
				}

				// Member access
				constexpr SizeType getBlockSize()    const { return blockSize_; }
				constexpr SizeType getBlockCount()   const { return blockCount_; }
				constexpr SizeType getMetaDataSize() const { return metaDataSize_; }


			private:
				constexpr SizeType calcMetaDataSize() const {
					// Round up so that all blocks can be represented.
					SizeType toReturn {
						common::roundUpToMultiple(blockCount_, elementSizeBits_)
					};

					// Meta data overhead is 1 bit per block.
					toReturn /= elementSizeBits_;

					// The meta data size must be a multiple of the alignment so that
					// the storage is also aligned.
					// Note: This shouldn't be a problem because elementSize should be
					// a factor of the alignment anyway.
					toReturn = common::roundUpToMultiple(toReturn, alignment);

					return toReturn;
				}

				SizeType blockSize_;
				SizeType blockCount_;
				SizeType metaDataSize_;

				static constexpr
				std::size_t elementSize_ {elementSize};

				static constexpr
				std::size_t elementSizeBits_ {elementSizeBits};
		};


		template <template <class T, SizeType size> class ArrayType, class T,
			SizeType    minimumBlockSize,
			SizeType    blockCount,
			std::size_t alignment>
		class ArrayTemplateWrapper :
			public ArrayType<T, Attributes<alignment>(
				minimumBlockSize, blockCount
			).getElementCount()> {

			public:
				static_assert((minimumBlockSize % alignment == 0),
				              "Minimum block size must be divisible by the alignment");
		};


		// Runtime policy
	private:
		template <template <class> class A>
		using RuntimeArrayType =
			traits::RuntimeSizedArray<
				A, BitmappedBlockArrayElement
			>;

		template <template <class> class A>
		using RuntimeArrayPolicyBase =
			traits::ArrayPolicyBase<RuntimeArrayType<A> >;


	public:
		template <template <class T> class CoreArray,
		  std::size_t t_alignment>
		class RuntimePolicy : public Attributes<t_alignment>,
		                      public RuntimeArrayPolicyBase<CoreArray> {
			public:
				static constexpr std::size_t alignment {t_alignment};

				using ArrayType = RuntimeArrayPolicyBase<CoreArray>;

				friend void swap(RuntimePolicy & first, RuntimePolicy & second) {
					using std::swap;

					swap(static_cast<DataType&>(first), static_cast<DataType&>(second));
					swap(static_cast<DataType&>(first), static_cast<DataType&>(second));
				}

				RuntimePolicy(SizeType minimumBlockSize, SizeType blockCount) :
					DataType   (minimumBlockSize, blockCount),
					PolicyBase (this->getElementCount()) {}


			private:
				using DataType   = Attributes<t_alignment>;
				using PolicyBase = RuntimeArrayPolicyBase<CoreArray>;
		};


		// Templated policy
	private:
		template <template <class, SizeType> class A,
			SizeType    s,
			SizeType    c,
			std::size_t a>
		using TemplatedArrayType =
			ArrayTemplateWrapper<A, BitmappedBlockArrayElement, s, c, a>;

		template <template <class, SizeType> class A,
			SizeType    s,
			SizeType    c,
			std::size_t a>
		using TemplatedArrayPolicyBase =
			traits::ArrayPolicyBase<TemplatedArrayType<A, s, c, a> >;

	public:
		template <template <class T, SizeType size> class CoreArray,
			SizeType    minimumBlockSize,
			SizeType    t_blockCount,
			std::size_t t_alignment>
		class TemplatedPolicy :
			public TemplatedArrayPolicyBase<CoreArray,    minimumBlockSize,
			                                t_blockCount, t_alignment> {
			private:
				using PolicyBase =
					TemplatedArrayPolicyBase<CoreArray,    minimumBlockSize,
					                         t_blockCount, t_alignment>;

			public:
				static constexpr std::size_t alignment {t_alignment};

				using ArrayType        = typename PolicyBase::ArrayType;
				using ArrayReturn      = typename PolicyBase::ArrayReturn;
				using ArrayConstReturn = typename PolicyBase::ArrayConstReturn;

				/*static constexpr SizeType getMetaDataSize()
					{ return metaDataSize; }

				static constexpr SizeType getStorageSize()
					{ return storageSize; }

				static constexpr SizeType getTotalSize()
					{ return totalSize; }

				static constexpr SizeType getElementCount()
					{ return elementCount; }

				static constexpr SizeType getBlockSize()
					{ return blockSize; }

				static constexpr SizeType getBlockCount()
					{ return blockCount; }*/

				static constexpr SizeType getMetaDataSize()
				{ return getAttributes().getMetaDataSize(); }

				static constexpr SizeType getStorageSize()
				{ return getAttributes().getStorageSize(); }

				static constexpr SizeType getTotalSize()
				{ return getAttributes().getTotalSize(); }

				static constexpr SizeType getElementCount()
				{ return getAttributes().getElementCount(); }

				static constexpr SizeType getBlockSize()
				{ return getAttributes().getBlockSize(); }

				static constexpr SizeType getBlockCount()
				{ return getAttributes().getBlockCount(); }


			private:
				static constexpr Attributes<alignment> getAttributes() {
					return {minimumBlockSize, t_blockCount};
				};

				/*// Forces compile-time evaluation.
				enum : SizeType {
					metaDataSize = getData().getMetaDataSize(),
					storageSize  = getData().getStorageSize(),
					totalSize    = getData().getTotalSize(),
					elementCount = getData().getElementCount(),
					blockSize    = getData().getBlockSize(),
					blockCount   = getData().getBlockCount()
				};*/
		};


		template <template <class T> class ArrayType,
			std::size_t alignment = alignof(std::max_align_t)>
		using Runtime =
		BasicBitmappedBlock<RuntimePolicy<ArrayType, alignment> >;


		template <template <class T, SizeType size> class CoreArray,
			std::size_t minimumBlockSize,
			std::size_t blockCount,
			std::size_t alignment = alignof(std::max_align_t)>
		using Templated =
		BasicBitmappedBlock<TemplatedPolicy<
		                    CoreArray, minimumBlockSize, blockCount, alignment>
		>;
};



	}
}

#endif
