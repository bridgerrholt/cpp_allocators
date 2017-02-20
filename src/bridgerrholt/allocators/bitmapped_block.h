#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_BITMAPPED_BLOCK_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_BITMAPPED_BLOCK_H

#define BRH_CPP_ALLOCATORS_BITMAPPED_BLOCK_NEXT_BYTE_ALLOCATION
#define BRH_CPP_ALLOCATORS_BITMAPPED_BLOCK_NEXT_BYTE_DEALLOCATION

//#define BRH_CPP_ALLOCATORS_BITMAPPED_BLOCK_3_STAGE_ALLOCATION

#include <iostream>
#include <array>
#include <vector>
#include <bitset>
#include <cstddef>
#include <cstring>
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

/// An allocator with memory segmented into blocks and meta data before the
/// blocks telling whether each is occupied or not.
/// This is a 1 bit per block overhead.
		class BitmappedBlock {

public:
	/// The type that BitmappedBlock allocator arrays are composed of.
	/// Allows simple bit operations.
	template <class T>
	class BasicArrayElement {
		public:
			using DataType = T;

			void unsetAll() { data_ =  0; }
			void setAll()   { data_ = ~0; }

			int getBit(DataType place) const {
				return ((data_ >> place) & 1);
			}

			void setBit(DataType place) {
				data_ |= 1 << place;
			}

			void unsetBit(DataType place) {
				data_ &= ~(1 << place);
			}


		private:
			DataType data_;
	};


	using ByteType     = unsigned char;
	using ArrayElement = BasicArrayElement<ByteType>;

	static_assert(std::is_pod<ArrayElement>::value,
	              "BitmappedBlock::ArrayElement should be POD");

	static constexpr
	unsigned int arrayElementSize     {sizeof(ArrayElement)};

	static constexpr
	unsigned int arrayElementSizeBits {arrayElementSize * CHAR_BIT};

	template <std::size_t alignment>
	class alignas(alignment) AlignedType :
		public std::array<ArrayElement, alignment / arrayElementSize> {
		public:
			static_assert(alignment >= arrayElementSize, "");
			static_assert(alignment %  arrayElementSize == 0, "");
	};


	template <class t_Policy>
	class alignas(t_Policy::alignment) Allocator : private t_Policy {
		public:
			using Policy = t_Policy;
			using Handle = RawBlock;

			/// Possibly undefined behavior if the swap function for the array
			/// requires the copying of elements.
			friend void swap(Allocator & first, Allocator & second) {
				using std::swap;

				swap(static_cast<Policy&>(first),  static_cast<Policy&>(second));
				swap(first.allocateByteHint_,      second.allocateByteHint_);
			}

			Allocator() : Allocator(Policy()) {}

			constexpr Allocator(Policy policy) :
				Policy                 (std::move(policy)),
				allocateByteHint_ {0} {

				deallocateAll();

				assert(reinterpret_cast<uintptr_t>(
					       Policy::getArray().data()
				       ) % Policy::alignment == 0);
			}

			Allocator(Allocator && other) :
				Policy(static_cast<Policy>(other)) {
				swap(*this, other);
			}

			Allocator(Allocator const &) = delete;


			constexpr SizeType calcRequiredSize(SizeType desiredSize) const {
				if (desiredSize == 0) {
					return getAttributes().getBlockSize();
				}
				else {
					return common::roundUpToMultiple(
						desiredSize, getAttributes().getBlockSize()
					);
				}
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
				for (SizeType i {0}; i < getAttributes().getBlockCount(); ++i) {
					std::cout << getMetaBit(i);
				}
			}

			SizeType countUsedBlocks() {
				SizeType count {0};
				for (SizeType i {0}; i < Policy::getBlockCount(); ++i) {
					if (getMetaBit(i) == 1)
						++count;
				}

				return count;
			}

			Handle allocateBlocks(SizeType blockCount) {
				return allocate(getAttributes().getBlockSize() * blockCount);
			}

			Handle allocate(SizeType size) {
				// An allocation takes place even if the size is 0.
				size = calcRequiredSize(size);

				// The amount of blocks that must be reserved for the allocation.
				std::size_t blocksRequired {size / getAttributes().getBlockSize()};

				// The block count must be divisible by the element size in bits.
				std::size_t const metaEnd {
					getAttributes().getBlockCount() / arrayElementSizeBits
				};

				// Try to allocate from the hint to the end then from 0 to the hint.
				auto ptr =
					attemptAllocation(blocksRequired, allocateByteHint_, metaEnd);

				if (ptr == nullptr)
					ptr = attemptAllocation(blocksRequired, 0, allocateByteHint_);

				if (ptr != nullptr)
					return {ptr, size};

				return Handle::makeNullBlock();
			}

			constexpr Handle allocateAll() {
				auto const end = Policy::getMetaDataSize();

				for (SizeType i {0}; i < end; ++i) {
					Policy::getArray()[i].setAll();
				}

				return {getBlockPtr(0), Policy::getStorageSize()};
			}


			constexpr void deallocate(NullBlock) const {}

			void deallocate(Handle block) {
				if (owns(block)) {
					auto ptr = static_cast<Pointer>(block.getPtr());

					// The amount of blocks it takes up.
					std::size_t blocks {
						block.getSize() / getAttributes().getBlockSize()
					};

					std::size_t blockIndexStart {getBlockIndex(ptr)};
					std::size_t objectEnd       {blockIndexStart + blocks};

					std::size_t blockIndex {blockIndexStart};
					while (blockIndex < objectEnd) {
						unsetMetaBit(blockIndex);

						++blockIndex;
					}

#ifdef BRH_CPP_ALLOCATORS_BITMAPPED_BLOCK_NEXT_BYTE_DEALLOCATION
					allocateByteHint_ = getMetaIndex(getBlockIndex(ptr));
#endif
				}

#ifdef BRH_CPP_ALLOCATORS_THROW_IN_DEALLOCATION
				else {
					throw std::runtime_error(
						"BitmappedBlock is attempting to deallocate unowned memory"
					);
				}
#endif
			}

			constexpr void deallocateAll() {
				auto const end = getAttributes().getMetaDataSize();

				for (SizeType i {0}; i < end; ++i) {
					Policy::getElements()[i].unsetAll();
				}

				allocateByteHint_ = 0;
			}

			bool reallocate(Handle & block, SizeType newSize) {
				newSize = Policy::calcNeededSize(newSize);
				auto const blockSize = block.getSize();

				if (newSize < blockSize) {
					auto blockToDeallocate = splitBlockUnchecked(block, newSize);
					deallocate(blockToDeallocate);
					return true;
				}

				else if (newSize == blockSize) {
					return true;
				}

				else {
					if (expand(block, newSize - blockSize))
						return true;

					// Must allocate somewhere else.
					auto newBlock = allocate(newSize);

					if (newBlock.isNull())
						return false;

					std::memcpy(newBlock.getPtr(), block.getPtr(), blockSize);
					deallocate(block);
					block = newBlock;
					return true;
				}
			}

			bool expand(Handle & block, SizeType amount) {
				if (amount == 0)
					return true;

				auto extra = Policy::calcNeededSize(amount);

				auto beginPtr =
					static_cast<ConstPointer>(block.getPtr()) + block.getSize();

				auto endPtr = beginPtr + extra;

				auto beginBlock = getBlockIndex(beginPtr);
				auto endBlock   = getBlockIndex(endPtr);

				bool fits;

				if (Policy::getBlockCount() < endBlock)
					fits = false;

				else {
					fits = true;
					for (SizeType i {beginBlock}; i < endBlock; ++i) {
						if (getMetaBit(i) == 1) {
							fits = false;
							break;
						}
					}
				}

				if (fits) {
					for (SizeType i {beginBlock}; i < endBlock; ++i) {
						setMetaBit(i);
					}

					block.setSize(block.getSize() + extra);

					return true;
				}

				return false;
			}

			bool owns(Handle block) {
				auto ptr = static_cast<Pointer>(block.getPtr());

				return (
					ptr >= Policy::getElements() + getAttributes().getMetaDataSize() &&
				  ptr <  Policy::getElements() + getAttributes().getTotalSize()
				);
			}

			bool isEmpty() const {
				auto const end = Policy::getBlockCount();

				for (SizeType i {0}; i < end; ++i) {
					if (getMetaBit(i) == 1)
						return false;
				}

				return true;
			}

			bool isFull() const {
				auto const end = Policy::getBlockCount();

				for (SizeType i {0}; i < end; ++i) {
					if (getMetaBit(i) == 0)
						return false;
				}

				return true;
			}

			SizeType calcUnoccupied() const {
				SizeType count {0};

				auto const end = Policy::getBlockCount();

				for (SizeType i {0}; i < end; ++i) {
					if (getMetaBit(i) == 0)
						++count;
				}

				return count * Policy::getBlockSize();
			}

			SizeType calcOccupied() const {
				SizeType count {0};

				auto const end = Policy::getBlockCount();

				for (SizeType i {0}; i < end; ++i) {
					if (getMetaBit(i) == 1)
						++count;
				}

				return count * Policy::getBlockSize();
			}

			Handle splitBlock(Handle & block, SizeType firstBlockSize) const {
				firstBlockSize = calcRequiredSize(firstBlockSize);

				if (firstBlockSize >= block.getSize())
					return Handle::makeNullBlock();

				else
					return splitBlockUnchecked(block, firstBlockSize);
			}


		private:
			using Pointer      = ArrayElement       *;
			using ConstPointer = ArrayElement const *;

			Pointer attemptAllocation(SizeType blocksRequired,
			                          SizeType startByte,
			                          SizeType endByte) {
				SizeType currentRegionSize {0};
				auto byte = startByte;

				while (byte < endByte) {
					for (ByteType bit {0}; bit < arrayElementSizeBits; ++bit) {
						if (getMetaBit(byte, bit) == 0) {
							++currentRegionSize;

							// Allocation will be successful.
							if (currentRegionSize == blocksRequired)
								return guaranteedAllocate(byte, bit, blocksRequired);
						}

						// The region has ended if the bit is not 0.
						else {
							currentRegionSize = 0;
						}
					}

					++byte;
				}

				return nullptr;
			}

			Pointer guaranteedAllocate(SizeType byte,
			                           int      bit,
			                           SizeType blocksRequired) {
				// Set all the meta bits to indicate occupation of the blocks.
				std::size_t lastIndex  {getBlockIndex(byte, bit)};
				std::size_t firstIndex {lastIndex - blocksRequired + 1};
				std::size_t index      {firstIndex};

#ifdef BRH_CPP_ALLOCATORS_BITMAPPED_BLOCK_3_STAGE_ALLOCATION

				auto startByte = getMetaIndex   (firstIndex);
				auto startBit  = getMetaBitIndex(firstIndex);

				auto firstBits = arrayElementSizeBits - startBit;
				if (firstBits > blocksRequired)
					firstBits = static_cast<unsigned int>(blocksRequired);

				auto remaining = blocksRequired - firstBits;

				auto bytes    = remaining / arrayElementSizeBits;
				auto lastBits = remaining % arrayElementSizeBits;

				auto bitIndex = startBit;
				auto bitEnd   = startBit + firstBits;
				while (bitIndex < bitEnd) {
					setMetaBit(startByte, bitIndex);
					++bitIndex;
				}

				auto byteIndex = startByte + 1;
				auto byteEnd   = byteIndex + bytes;
				while (byteIndex < byteEnd) {
					Policy::getElements()[byteIndex].setAll();

					++byteIndex;
				}

				bitIndex = 0;
				bitEnd   = bitIndex + lastBits;
				while (bitIndex < bitEnd) {
					setMetaBit(byteEnd, bitIndex);
					++bitIndex;
				}

#ifdef BRH_CPP_ALLOCATORS_BITMAPPED_BLOCK_NEXT_BYTE_ALLOCATION
				auto toSet = lastIndex + 1;
				if (toSet == getAttributes().getBlockCount())
					allocateByteHint_ = 0;
				else
					allocateByteHint_ = getMetaIndex(toSet);
#endif


#else
				while (index <= lastIndex) {
					setMetaBit(index);

					++index;
				}

#ifdef BRH_CPP_ALLOCATORS_BITMAPPED_BLOCK_NEXT_BYTE_ALLOCATION
				auto hint = lastIndex + 1;
				if (hint == getAttributes().getBlockCount())
					allocateByteHint_ = 0;
				else
					allocateByteHint_ = getMetaIndex(hint);
#endif

#endif

				auto toReturn = getBlockPtr(firstIndex);
				return toReturn;
			}

			Handle splitBlockUnchecked(Handle & block,
			                           SizeType firstBlockSize) const {
				auto difference = block.getSize() - firstBlockSize;
				block.setSize(firstBlockSize);

				return {
					block.getEnd(), difference
				};
			}

			constexpr typename
			Policy::AttributesReturnType getAttributes() const {
				return Policy::getAttributes();
			}

			SizeType getBlockIndex(void const * blockPtr) const {
				return getBlockIndex(static_cast<ConstPointer>(blockPtr));
			}

			SizeType getBlockIndex(ConstPointer blockPtr) const {
				auto normal = blockPtr - Policy::getElements() -
				              getAttributes().getMetaDataSize();
				return (normal / getAttributes().getBlockSize());
			}


			constexpr static SizeType getMetaIndex(SizeType blockIndex) {
				return blockIndex / arrayElementSizeBits;
			}

			constexpr static unsigned getMetaBitIndex(SizeType blockIndex) {
				return static_cast<unsigned>(blockIndex % arrayElementSizeBits);
			}

			constexpr static SizeType getBlockIndex(SizeType metaIndex,
			                                        int  metaBitIndex) {
				return metaIndex * arrayElementSizeBits + metaBitIndex;
			}


			ConstPointer getBlockPtr(SizeType blockIndex) const {
				return Policy::getElements() + getBlockPtrOffset(blockIndex);
			}

			Pointer getBlockPtr(SizeType blockIndex) {
				return Policy::getElements() + getBlockPtrOffset(blockIndex);
			}

			SizeType getBlockPtrOffset(SizeType blockIndex) const {
				return getAttributes().getMetaDataSize() +
					blockIndex * getAttributes().getBlockSize();
			}


			int getMetaBit(SizeType blockIndex) const {
				return getMetaBit(getMetaIndex   (blockIndex),
				                  getMetaBitIndex(blockIndex));
			}

			int getMetaBit(SizeType metaIndex, int metaBitIndex) const {
				return Policy::getElements()[metaIndex].getBit(metaBitIndex);
			}


			void setMetaBit(SizeType blockIndex) {
				setMetaBit(getMetaIndex(blockIndex), getMetaBitIndex(blockIndex));
			}

			void setMetaBit(SizeType metaIndex, int metaBitIndex) {
				Policy::getElements()[metaIndex].setBit(metaBitIndex);
			}


			void unsetMetaBit(SizeType blockIndex) {
				unsetMetaBit(getMetaIndex(blockIndex), getMetaBitIndex(blockIndex));
			}

			void unsetMetaBit(SizeType metaIndex, int metaBitIndex) {
				Policy::getElements()[metaIndex].unsetBit(metaBitIndex);
			}


			// Calculates how efficiently memory space is used.
			double efficiency() {
				return static_cast<double>(getAttributes().getBlockCount())
				       / (getAttributes().getMetaDataSize() * arrayElementSizeBits);
			}

			SizeType allocateByteHint_;
	};


	/// Contains values regarding information about size.
	/// Satisfies LiteralType.
	template <std::size_t alignment>
	class Attributes {
		public:
			using ArrayElementType = AlignedType<alignment>;

			/// Primary constructor.
			constexpr Attributes(SizeType minimumBlockSize,
                           SizeType minimumBlockCount) :
				blockSize_    {common::roundUpToMultiple(minimumBlockSize,
				                                         alignment)},
				blockCount_   {common::roundUpToMultiple(minimumBlockCount,
				                                         arrayElementSizeBits)},
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
					getTotalSize(), arrayElementSize_
				) / arrayElementSize_;
			}

			// Member access
			constexpr SizeType getBlockSize()    const { return blockSize_; }
			constexpr SizeType getBlockCount()   const { return blockCount_; }
			constexpr SizeType getMetaDataSize() const { return metaDataSize_; }


		private:
			constexpr SizeType calcMetaDataSize() const {
				// Round up so that all blocks can be represented.
				// TODO: Remove this, it is now useless because blockCount_
				//       is already rounded up in the constructor.
				SizeType toReturn {
					common::roundUpToMultiple(blockCount_, elementSizeBits_)
				};

				// Meta data overhead is 1 bit per block.
				toReturn /= elementSizeBits_;

				// The meta data size must be a multiple of the alignment so that
				// the storage is also aligned.
				toReturn = common::roundUpToMultiple(toReturn, alignment);

				return toReturn;
			}

			SizeType blockSize_;
			SizeType blockCount_;
			SizeType metaDataSize_;

			static constexpr
			std::size_t arrayElementSize_ {sizeof(ArrayElementType)};

			static constexpr
			std::size_t elementSize_ {arrayElementSize};

			static constexpr
			std::size_t elementSizeBits_ {arrayElementSizeBits};
	};


	template <template <class T, SizeType size> class ArrayType, class T,
		SizeType    minimumBlockSize,
		SizeType    blockCount,
		std::size_t alignment>
	class ArrayTemplateWrapper :
		public ArrayType<T, Attributes<alignment>(
			minimumBlockSize, blockCount
		).getElementCount()> {
	};


private:
	// Runtime policy
	template <template <class> class A, std::size_t alignment>
	using RuntimeArrayType =
		traits::RuntimeSizedArray<
			A, AlignedType<alignment>
		>;

	template <template <class> class A, std::size_t alignment>
	using RuntimeArrayPolicyBase =
		traits::ArrayPolicyBase<RuntimeArrayType<A, alignment> >;


public:
	template <template <class T> class CoreArray,
	  std::size_t t_alignment>
	class RuntimePolicy :
		public RuntimeArrayPolicyBase<CoreArray, t_alignment> {

		public:
			static constexpr std::size_t alignment {t_alignment};

			using AttributesType       = Attributes<alignment>;
			using AttributesReturnType = AttributesType const &;

			friend void swap(RuntimePolicy & first, RuntimePolicy & second) {
				using std::swap;

				swap(static_cast<PolicyBase&>(first),
				     static_cast<PolicyBase&>(second));

				swap(first.attributes_, second.attributes_);
			}

			RuntimePolicy(SizeType minimumBlockSize,
			              SizeType minimumBlockCount) :
				RuntimePolicy(AttributesType(minimumBlockSize,
				                             minimumBlockCount)) {}

			/*SizeType calcNeededSize(SizeType desiredSize) const {
				return BitmappedBlock::calcNeededSize(
					DataType::getBlockSize(), desiredSize
				);
			}*/

			ArrayElement * getElements() {
				return &static_cast<ArrayElement&>(
					PolicyBase::getArray().front()[0]
				);
			}

			ArrayElement const * getElements() const {
				return &static_cast<ArrayElement const &>(
					PolicyBase::getArray().front()[0]
				);
			}

			AttributesReturnType getAttributes() const {
				return attributes_;
			}


		private:
			using PolicyBase = RuntimeArrayPolicyBase<CoreArray, alignment>;
			using ArrayElementType = typename PolicyBase::ArrayType::value_type;

			RuntimePolicy(AttributesType attributes) :
				PolicyBase  (attributes.getElementCount()),
			  attributes_ (std::move(attributes)) {}

			AttributesType attributes_;
	};


	// Templated policy
private:
	template <template <class, SizeType> class A,
		SizeType    s,
		SizeType    c,
		std::size_t a>
	using TemplatedArrayType =
		ArrayTemplateWrapper<A, AlignedType<a>, s, c, a>;

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

			using AttributesType       = Attributes<alignment>;
			using AttributesReturnType = AttributesType;

			/*static constexpr SizeType calcNeededSize(SizeType desiredSize) {
				return BitmappedBlock::calcNeededSize(getBlockSize(), desiredSize);
			}

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
				{ return getAttributes().getBlockCount(); }*/

			static constexpr AttributesReturnType getAttributes() {
				return {minimumBlockSize, t_blockCount};
			};

			ArrayElement * getElements() {
				return &static_cast<ArrayElement&>(
					PolicyBase::getArray().front()[0]
				);
			}

			ArrayElement const * getElements() const {
				return &static_cast<ArrayElement const &>(
					PolicyBase::getArray().front()[0]
				);
			}


		private:
			using ArrayElementType = typename PolicyBase::ArrayType::value_type;
	};


	template <template <class T> class ArrayType,
		std::size_t alignment = alignof(std::max_align_t)>
	using Runtime =
	Allocator<RuntimePolicy<ArrayType, alignment> >;


	template <template <class T, SizeType size> class CoreArray,
		std::size_t minimumBlockSize,
		std::size_t blockCount,
		std::size_t alignment = alignof(std::max_align_t)>
	using Templated =
	Allocator<TemplatedPolicy<
						CoreArray, minimumBlockSize, blockCount, alignment>
	>;

		};

	}
}

#endif
