#ifndef BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_AFFIX_ALLOCATOR_H
#define BRH_CPP_ALLOCATORS_SRC_BRIDGERRHOLT_ALLOCATORS_AFFIX_ALLOCATOR_H

#include <type_traits>

#include "common/common_types.h"

namespace bridgerrholt {
	namespace allocators {


template <class T>
constexpr void voidSafePlacementNew(void * destination) {
	new (destination) T ();
}

template <>
constexpr void voidSafePlacementNew<void>(void *) {}

template <class t_Allocator, class Prefix, class Suffix = void>
class AffixAllocator : private t_Allocator
{
	public:
		using Allocator = t_Allocator;
		using Handle    = typename Allocator::Handle;
		using Pointer   = typename Handle::Pointer;

		SizeType calcRequiredSize(SizeType objectSize) {
			return Allocator::calcRequiredSize(calcFullSize(objectSize));
		}

		Handle allocate(SizeType objectSize) {
			Handle handle = Allocator::allocate(calcFullSize(objectSize));

			voidSafePlacementNew<Prefix>(handle.getPtr());
			voidSafePlacementNew<Suffix>(handle.getEndChar() - getSuffixSize());

			//std::cout << "Prefix: " << handle.getPtr() << '\n';
			//std::cout << "Suffix: " << static_cast<void*>(handle.getEndChar() - getSuffixSize()) << '\n';

			handle.setPtr(handle.getCharPtr() + getPrefixSize());

			return handle;
		}

		void deallocate(Handle handle) {
			handle.setPtr(handle.getCharPtr() - getPrefixSize());
			Allocator::deallocate(handle);
		}

		SizeType calcFullSize(SizeType objectSize) const {
			return (objectSize + getExtraSize());
		}

		constexpr SizeType getExtraSize() const {
			return getPrefixSize() + getSuffixSize();
		}

		constexpr SizeType getPrefixSize() const {
			return getSize<Prefix>();
		}

		constexpr SizeType getSuffixSize() const {
			return getSize<Suffix>();
		}

	private:
		template <class T>
		constexpr SizeType getSize() const {
			if (std::is_void<T>::value)
				return 0;
			else
				return sizeof(T);
		}
};



template <class T, T value>
class VarWrapper
{
	public:
		VarWrapper() : value_(value) {}

		T getValue() const { return value_; }

		friend bool operator==(VarWrapper const & first,
		                       VarWrapper const & second) {
			return (first.getValue() == second.getValue());
		}

		friend bool operator!=(VarWrapper const & first,
		                       VarWrapper const & second) {
			return !(first == second);
		}

	private:
		T value_;
};

template <class CoreAllocator, class Affix>
class AffixChecker : public AffixAllocator<CoreAllocator, Affix, Affix>
{
	public:
		using Allocator = AffixAllocator<CoreAllocator, Affix, Affix>;
		using Handle    = typename Allocator::Handle;
		using Pointer   = typename Handle::Pointer;

		void deallocate(Handle handle) {
			if (!verify(handle))
				throw std::runtime_error("Corrupt data");

			Allocator::deallocate(handle);
		}

	private:
		bool verify(Handle handle) const {
			handle.setPtr(handle.getCharPtr() - Allocator::getPrefixSize());

			Affix * prefix = reinterpret_cast<Affix*>(
				handle.getPtr()
			);

			Affix * suffix = reinterpret_cast<Affix*>(
				handle.getEndChar() - Allocator::getSuffixSize()
			);

			//std::cout << "Prefix: " << prefix << '\n';
			//std::cout << "Suffix: " << suffix << '\n';

			return (*prefix == *suffix);
		}
};


	}
}

#endif
