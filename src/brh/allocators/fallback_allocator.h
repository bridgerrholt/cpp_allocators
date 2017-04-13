#ifndef BRH_CPP_ALLOCATORS_SRC_BRH_ALLOCATORS_FALLBACK_ALLOCATOR_H
#define BRH_CPP_ALLOCATORS_SRC_BRH_ALLOCATORS_FALLBACK_ALLOCATOR_H

#include "common/common_types.h"

#include "blocks/block.h"

namespace brh {
	namespace allocators {

template <class Primary, class Fallback>
class FallbackAllocator : private Primary,
                          private Fallback
{
	public:
		FallbackAllocator() {}

		RawBlock allocate(SizeType size) {
			RawBlock toReturn {Primary::allocate(size)};

			if (toReturn.isNull())
				toReturn = Fallback::allocate(size);

			return toReturn;
		}

		constexpr void deallocate(NullBlock) {}

		void deallocate(RawBlock block) {
			if (Primary::owns(block))
				Primary::deallocate(block);

			else
				Fallback::deallocate(block);
		}

		bool owns(RawBlock block) {
			return (Primary::owns(block) || Fallback::owns(block));
		}
};



	}
}

#endif
