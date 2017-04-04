#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_BITMAPPED_BLOCK_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_BITMAPPED_BLOCK_H

#define BRH_CPP_ALLOCATORS_BITMAPPED_BLOCK_NEXT_BYTE_ALLOCATION
#define BRH_CPP_ALLOCATORS_BITMAPPED_BLOCK_NEXT_BYTE_DEALLOCATION

//#define BRH_CPP_ALLOCATORS_MULTITHREADED
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
#include <functional>

#include "traits/traits.h"
#include "common/common_types.h"
#include "wrappers/allocator_wrapper.h"
#include "common/round_up_to_multiple.h"
#include "common/calc_lcm.h"

#include "multithread/thread.h"

namespace bridgerrholt {
	namespace allocators {
		namespace bitmapped_block {

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
class alignas(t_Policy::alignment)
Allocator : private t_Policy {
	private:
		using Pointer      = ArrayElement       *;
		using ConstPointer = ArrayElement const *;

	public:
		using Policy = t_Policy;
		using Handle = RawBlock;

		friend void swap(Allocator & first, Allocator & second) {
			using std::swap;

			swap(static_cast<Policy&>(first),  static_cast<Policy&>(second));
			swap(first.allocateByteHint_,      second.allocateByteHint_);
		}

		Allocator() : Allocator(Policy()) {}

		Allocator(Policy policy) :
			Policy            (std::move(policy)),
			allocateByteHint_ {0} {

			deallocateAll();

			// Make sure the array is aligned correctly.
			assert(
				reinterpret_cast<uintptr_t>(
				       Policy::getArray().data()
				) % Policy::alignment == 0
			);
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

		constexpr SizeType getStorageSize() const {
			return Policy::getAttributes().getStorageSize();
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

		// Allocate
		Handle allocateBlocks(SizeType blockCount) {
			return allocate(getAttributes().getBlockSize() * blockCount);
		}

		Handle allocate(SizeType size) {
			//std::cout << "allocate\n";
			SizeType blocksRequired;
			allocationSetup(size, blocksRequired);

			bool     finished;
			auto     byte = allocateByteHint_;
			ByteType bit  = 0;

			do {
				//std::cout << byte << ", " << static_cast<unsigned int>(bit) << '\n';
				if (findAllocationFromLocation(finished, byte, bit, blocksRequired))
					return {guaranteedAllocate(byte, bit, blocksRequired), size};
			} while (!finished);

			byte = 0;
			bit  = 0;
			auto end = getMetaEnd();

			do {
				if (findAllocationFromLocation(finished, byte, bit, blocksRequired))
					return {guaranteedAllocate(byte, bit, blocksRequired), size};
			} while (byte < end);

			//std::cout << "endAllocate\n";
			return Handle::makeNullBlock();


			/*// Try allocating on the last half then the first half.
			auto ptr = attemptAllocationFromHint(
				blocksRequired, allocateByteHint_
			);

			if (ptr == nullptr) {
				ptr = attemptAllocationToHint(
					blocksRequired, allocateByteHint_
				);
			}

			if (ptr == nullptr) {
				return Handle::makeNullBlock();
			}
			else {
				return {ptr, size};
			}*/
		}

		Handle allocateAligned(SizeType size, SizeType alignment) {
			SizeType blocksRequired;
			allocationSetup(size, blocksRequired);


			std::cout << "calcLcm(" << getAttributes().getBlockSize() <<  ", " << alignment << ")\n";
			auto lcm = common::calcLcm(
				getAttributes().getBlockSize(), alignment
			);

			auto step = lcm / getAttributes().getBlockSize();

			std::cout << "attemptAllocationFromHint()\n";
			// Try allocating on the last half then the first half.
			/*auto ptr = attemptAllocationFromHint(
				blocksRequired, getBlockIndex(nextPtr), step
			);*/

			auto hintPtr = getBlockPtr(getBlockIndex(allocateByteHint_, 0));
			auto firstPtr = findNextAligned(hintPtr, alignment);
			auto firstPtrIndex = getBlockIndex(firstPtr);

			bool     finished;
			auto     byte = getMetaIndex   (firstPtrIndex);
			ByteType bit  = getMetaBitIndex(firstPtrIndex);

			do {
				//std::cout << byte << ", " << static_cast<unsigned int>(bit) << '\n';
				if (findAllocationFromLocation(finished, byte, bit, blocksRequired))
					return {guaranteedAllocate(byte, bit, blocksRequired), size};
			} while (!finished);

			byte = 0;
			bit  = 0;
			auto end = getMetaEnd();

			do {
				if (findAllocationFromLocation(finished, byte, bit, blocksRequired))
					return {guaranteedAllocate(byte, bit, blocksRequired), size};
			} while (byte < end);

			//std::cout << "endAllocate\n";
			return Handle::makeNullBlock();

			/*auto ptr = attemptAlignedAllocationFromIndex(
				blocksRequired, firstPtrIndex, step
			);

			if (ptr == nullptr) {
				auto nextPtr = findNextAligned(Policy::getElements(), alignment);
				ptr = attemptAlignedAllocationToHint(
					blocksRequired, getBlockIndex(nextPtr), firstPtrIndex, step
				);
			}

			if (ptr == nullptr) {
				return Handle::makeNullBlock();
			}
			else {
				return {ptr, size};
			}*/
		}

		constexpr Handle allocateAll() {
			auto const end = Policy::getMetaDataSize();

			for (SizeType i {0}; i < end; ++i) {
				Policy::getArray()[i].setAll();
			}

			return {getBlockPtr(0), Policy::getStorageSize()};
		}


		// Deallocate
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
			newSize = calcRequiredSize(newSize);
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

			auto extra = calcRequiredSize(amount);

			auto beginPtr =
				static_cast<ConstPointer>(block.getPtr()) + block.getSize();

			auto endPtr = beginPtr + extra;

			auto beginBlock = getBlockIndex(beginPtr);
			auto endBlock   = getBlockIndex(endPtr);

			bool fits;

			if (Policy::getAttributes().getBlockCount() < endBlock)
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
				beforeEnd(ptr)
			);
		}

		bool withinBounds(Handle block) {
			auto ptrLeft  = static_cast<Pointer>(block.getPtr());
			auto ptrRight = ptrLeft + block.getSize();

			return (
				ptrLeft >= Policy::getElements() +
				           getAttributes().getMetaDataSize() &&
				ptrRight <= Policy::getElements() + getAttributes().getTotalSize()
			);
		}

		bool beforeEnd(Pointer ptr) const {
			return (ptr <  Policy::getElements() + getAttributes().getTotalSize());
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

		/// @return Second block.
		Handle splitBlock(Handle & block, SizeType firstBlockSize) const {
			firstBlockSize = calcRequiredSize(firstBlockSize);

			if (firstBlockSize >= block.getSize())
				return Handle::makeNullBlock();

			else
				return splitBlockUnchecked(block, firstBlockSize);
		}


	private:
#ifdef BRH_CPP_ALLOCATORS_MULTITHREADED
		using MutexType = std::mutex;
		using LockType  = std::lock_guard<MutexType>;
#endif

		/// Calculates values for variables common to all allocation methods.
		void allocationSetup(SizeType & size,
		                     SizeType & outBlocksRequired) const {
			size = calcRequiredSize(size);

			// The amount of blocks that must be reserved for the allocation.
			outBlocksRequired = size / getAttributes().getBlockSize();
		}

		Handle allocate(SizeType size, SizeType alignment) {
			auto multiple = Policy::alignment / alignment;

			// An allocation takes place even if the size is 0.
			size = calcRequiredSize(size);

			// The amount of blocks that must be reserved for the allocation.
			std::size_t blocksRequired {size / getAttributes().getBlockSize()};

			// The block count must be divisible by the element size in bits.
			std::size_t const metaEnd {
				getAttributes().getBlockCount() / arrayElementSizeBits
			};

			// Try allocating on the last half then the first half.
			auto ptr = attemptAllocation(
				blocksRequired, allocateByteHint_, metaEnd
			);

			if (ptr == nullptr) {
				ptr = attemptAllocation(
					blocksRequired, 0, metaEnd
				);
			}

			if (ptr == nullptr) {
				return Handle::makeNullBlock();
			}
			else {
				return {ptr, size};
			}
		}

		Pointer findNextAligned(Pointer start, std::size_t alignment) const {
			auto current = start;

			while (beforeEnd(current)) {
				if (reinterpret_cast<std::uintptr_t>(current) % alignment == 0)
					return current;

				current += Policy::getAttributes().getBlockSize();
			}

			return nullptr;
		}

		/// @param outFinished Indicates whether the loop reached the end.
		/// @param byte Start byte, is altered as the function loops.
		/// @param bit  Start bit, is altered as the function loops.
		bool findAllocationFromLocation(bool     & outFinished,
		                                SizeType & byte,
		                                ByteType & bit,
		                                SizeType   blocksRequired) {
			SizeType currentRegionSize {0};

			auto endByte = getMetaEnd();

			while (byte < endByte) {
				while (bit < arrayElementSizeBits) {
					if (getMetaBit(byte, bit) == 0) {
						++currentRegionSize;

						if (currentRegionSize == blocksRequired) {
							outFinished = false;
							return true;
						}
					}

					else {
						outFinished = false;
						++bit;
						return false;
					}

					++bit;
				}

				bit = 0;
				++byte;
			}

			outFinished = true;
			return false;
		}

		/// Iterates through all the bits in a specified meta byte.
		/// Accepts a functor to be executed upon finding a set bit.
		/// @return Whether currentRegionSize reaches blocksRequired.
		template <class F>
		bool testByte(ByteType & outBit,
	                SizeType & currentRegionSize,
	                SizeType   byte,
	                SizeType   blocksRequired,
	                F          occupiedBlockFunction) {
			for (ByteType bit {0}; bit < arrayElementSizeBits; ++bit) {
				if (getMetaBit(byte, bit) == 0) {
					++currentRegionSize;

					// Allocation will be successful.
					if (currentRegionSize == blocksRequired) {
						outBit = bit;
						return true;
					}
				}

					// The region has ended if the bit is not 0.
				else {
					currentRegionSize = 0;
					occupiedBlockFunction();
				}
			}

			return false;
		}

		bool testByte(ByteType & outBit,
		              SizeType & currentRegionSize,
		              SizeType   byte,
		              SizeType   blocksRequired) {
			return testByte(outBit, currentRegionSize, byte, blocksRequired, []{});
		}

		/// Indicates whether an occupied block was found or not.
		bool testByte(bool     & outFoundOccupied,
		              ByteType & outBit,
			            SizeType & currentRegionSize,
		              SizeType   byte,
		              SizeType   blocksRequired) {
			// Passes outFoundOccupied to the templated function as a functor.
			class OccupiedCatcher {
				public:
					void operator()() { found = true; }

					bool & found;
			} catcher {outFoundOccupied};

			outFoundOccupied = false;

			return testByte(
				outBit, currentRegionSize, byte, blocksRequired,
				[&outFoundOccupied] { outFoundOccupied = true; }
			);
		}

		bool findAllocation(SizeType & outByte,
		                    ByteType & outBit,
				                SizeType   blocksRequired,
		                    SizeType   startByte,
		                    SizeType   endByte,
		                    SizeType   step = 1) {
			SizeType currentRegionSize {0};
			auto byte = startByte;

			while (byte < endByte) {
				if (testByte(outBit, currentRegionSize, byte, blocksRequired)) {
					outByte = byte;
					return true;
				}

				byte += step;
			}

			return false;
		}

		bool findAllocationToHint(SizeType & outByte,
		                          ByteType & outBit,
		                          SizeType   blocksRequired,
		                          SizeType   hintByte,
		                          SizeType   start = 0,
		                          SizeType   step = 1) {
			std::cout << blocksRequired << ", " << hintByte << ", " << step << '\n';
			SizeType currentRegionSize {0};
			SizeType byte {start};
			auto endByte = getMetaEnd();

			if (byte > hintByte) return false;

			while (true) {
				if (testByte(outBit, currentRegionSize, byte, blocksRequired)) {
					outByte = byte;
					return true;
				}

				if (hintByte - step <= byte)
					break;

				byte += step;
			}

			byte += step;

			std::cout << "Second stage " << byte << ", " << currentRegionSize << ", " << step << '\n';
			while (true) {
				bool foundOccupied;

				if (testByte(foundOccupied, outBit, currentRegionSize,
				             byte, blocksRequired)) {
					outByte = byte;
					std::cout << byte << '\n';
					return true;
				}

				std::cout << foundOccupied << '\n';

				if (foundOccupied || endByte - step <= byte)
					return false;

				byte += step;
			}
		}

		Pointer attemptAllocation(SizeType blocksRequired,
		                          SizeType startByte,
		                          SizeType endByte,
		                          SizeType step = 1) {
			SizeType byte;
			ByteType bit;

			if (findAllocation(byte, bit, blocksRequired, startByte, endByte, step))
				return guaranteedAllocate(byte, bit, blocksRequired);

			else return nullptr;
		}

		Pointer attemptAllocationFromHint(SizeType blocksRequired,
		                                  SizeType hintByte,
		                                  SizeType step = 1) {
			SizeType byte;
			ByteType bit;

			if (findAllocation(byte, bit, blocksRequired, hintByte, getMetaEnd(), step))
				return guaranteedAllocate(byte, bit, blocksRequired);

			else return nullptr;
		}

		Pointer attemptAllocationToHint(SizeType blocksRequired,
		                                SizeType hintByte,
		                                SizeType start = 0,
																		SizeType step = 1) {
			SizeType byte;
			ByteType bit;

			if (findAllocationToHint(byte, bit, blocksRequired, hintByte, start, step))
				return guaranteedAllocate(byte, bit, blocksRequired);

			else return nullptr;
		}

		Pointer attemptAlignedAllocationFromIndex(SizeType blocksRequired,
		                                          SizeType startIndex,
		                                          SizeType step) {
			SizeType currentRegionSize {0};

			auto end       = getAttributes().getBlockCount();
			auto nextStart = startIndex;
			auto i         = startIndex;

			Pointer lambdaOut;
			bool    allocated {false};

			// Executed before the main loop and within the main loop.
			auto loopBody = [this, blocksRequired, &currentRegionSize,
				               &i, &lambdaOut, &allocated] {
				if (getMetaBit(i) == 0) {
					++currentRegionSize;

					if (currentRegionSize == blocksRequired) {
						lambdaOut = guaranteedAllocate(
							getMetaIndex(i), getMetaBitIndex(i), blocksRequired
						);
						allocated = true;
					}

					++i;
					return true;
				}

				else return false;
			};

			// First iteration to ensure i advances past the start.
			if (loopBody()) {
				if (allocated) return lambdaOut;
			}
			else i += step;

			// Check if i overflowed.
			if (i > startIndex) {

				// Loop as normal until the end.
				while (i < end) {
					if (loopBody()) {
						if (allocated) return lambdaOut;
					}
					else {
						currentRegionSize = 0;
						auto previousIndex = i;
						i = startIndex + common::roundUpToMultiple(i - startIndex, step);

						// If i is less than previousIndex, an overflow occurred.
						if (i >= end || i < previousIndex)
							break;
					}
				}
			}

			return nullptr;
		}

		Pointer attemptAlignedAllocationToIndex(SizeType blocksRequired,
	                                          SizeType endIndex,
	                                          SizeType step) {
			SizeType currentRegionSize {0};
			SizeType startIndex {0};

			auto end       = getAttributes().getBlockCount();
			auto nextStart = startIndex;
			auto i         = startIndex;

			Pointer lambdaOut;
			bool    allocated {false};

			// Executed before the main loop and within the main loop.
			auto loopBody = [this, blocksRequired, &currentRegionSize,
				               &i, &lambdaOut, &allocated] {
				if (getMetaBit(i) == 0) {
					++currentRegionSize;

					if (currentRegionSize == blocksRequired) {
						lambdaOut = guaranteedAllocate(
							getMetaIndex(i), getMetaBitIndex(i), blocksRequired
						);
						allocated = true;
					}

					++i;
					return true;
				}

				else return false;
			};

			// First iteration to ensure i advances past the start.
			if (loopBody()) {
				if (allocated) return lambdaOut;
			}
			else i += step;

			// Check if i overflowed.
			if (i > startIndex) {

				// Loop as normal until the end.
				while (i < end) {
					if (loopBody()) {
						if (allocated) return lambdaOut;
					}
					else {
						currentRegionSize = 0;
						auto previousIndex = i;
						i = startIndex + common::roundUpToMultiple(i - startIndex, step);

						// If i is less than previousIndex, an overflow occurred.
						if (i >= end || i < previousIndex)
							break;
					}
				}
			}

			return nullptr;
		}

		Handle attemptAllocation(SizeType blocksRequired, SizeType size) {
			// Try allocating on the last half then all of it.
			auto ptr = attemptAllocation(
				blocksRequired, allocateByteHint_, getMetaEnd()
			);

			if (ptr == nullptr) {
				ptr = attemptAllocation(
					blocksRequired, 0, getMetaEnd()
				);
			}

			if (ptr == nullptr) {
				return Handle::makeNullBlock();
			}
			else {
				return {ptr, size};
			}
		}

		Handle attemptAllocationAligned(SizeType blocksRequired,
		                                SizeType size,
		                                SizeType alignment) {

			/*Pointer ptr = getBlockPtr(getBlockIndex(allocateByteHint_, 0));

			auto ptrVal = reinterpret_cast<std::uintptr_t>(ptr);
			ptrVal = common::roundUpToMultiple(ptrVal, alignment);

			Pointer & ptrRef = *reinterpret_cast<Pointer *>(&ptrVal);


			while (true) {
				if (beforeEnd(ptrRef)) {
					if (ptrVal % alignment == 0)
				}
				else {
					ptrRef = Policy::getElements();
				}
			}

			auto hint = common::roundUpToMultiple(
				getBlockPtr(getBlockIndex(allocateByteHint_, 0)),
			  alignment
			);*/
		}

		Pointer guaranteedAllocate(SizeType byte,
		                           ByteType bit,
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
#ifdef BRH_CPP_ALLOCATORS_MULTITHREADED
			std::lock_guard<std::mutex> lock {allocationMutex_};
#endif

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

		constexpr static ByteType getMetaBitIndex(SizeType blockIndex) {
			return static_cast<ByteType>(blockIndex % arrayElementSizeBits);
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

		constexpr SizeType getMetaEnd() const {
			// The block count must be divisible by the element size in bits.
			return getAttributes().getBlockCount() / arrayElementSizeBits;
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

#ifdef BRH_CPP_ALLOCATORS_MULTITHREADED
		LockType makeAllocationLock() const {
			return {allocationMutex_};
		}
#else
		char makeAllocationLock() const {
				return 0;
			}
#endif


		// Calculates how efficiently memory space is used.
		double efficiency() {
			return static_cast<double>(getAttributes().getBlockCount())
			       / (getAttributes().getMetaDataSize() * arrayElementSizeBits);
		}

		SizeType allocateByteHint_;

#ifdef BRH_CPP_ALLOCATORS_MULTITHREADED
		MutexType allocationMutex_;
#endif
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

// Runtime policy
template <template <class> class A>
struct RuntimeArrayType {
	template <class T>
	using Array = traits::RuntimeSizedArray<
		A, T
	>;
};

template <template <class> class A, std::size_t alignment>
using RuntimeArrayPolicyBase =
	traits::ArrayPolicyInterface<RuntimeArrayType<A>::template Array, AlignedType<alignment> >;


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
template <template <class, SizeType> class A,
	SizeType    s,
	SizeType    c,
	std::size_t a>
struct TemplatedArrayType {
	template <class T>
	using Array = ArrayTemplateWrapper<A, T, s, c, a>;
};

template <template <class, SizeType> class A,
	SizeType    s,
	SizeType    c,
	std::size_t a>
using TemplatedArrayPolicyBase =
	traits::ArrayPolicyInterface<TemplatedArrayType<A, s, c, a>::template Array, AlignedType<a> >;

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

		} // bitmapped_block



/// An allocator with memory segmented into blocks and meta data before the
/// blocks telling whether each is occupied or not.
/// This is a 1 bit per block overhead.
class BitmappedBlock
{
	public:
		template <class Policy>
		using Allocator = bitmapped_block::Allocator<Policy>;

		template <template <class T> class CoreArray,
			std::size_t alignment>
		using RuntimePolicy =
			bitmapped_block::RuntimePolicy<CoreArray, alignment>;

		template <template <class T, SizeType size> class CoreArray,
			SizeType    minimumBlockSize,
			SizeType    blockCount,
			std::size_t alignment>
		using TemplatedPolicy = bitmapped_block::TemplatedPolicy<
			CoreArray, minimumBlockSize, blockCount, alignment>;

		template <template <class T> class ArrayType,
			std::size_t alignment = alignof(std::max_align_t)>
		using Runtime = bitmapped_block::Runtime<ArrayType, alignment>;


		template <template <class T, SizeType size> class CoreArray,
			std::size_t minimumBlockSize,
			std::size_t blockCount,
			std::size_t alignment = alignof(std::max_align_t)>
		using Templated = bitmapped_block::Templated<
			CoreArray, minimumBlockSize, blockCount, alignment>;
};


	}
}

#endif
