#include <allocators/bitmapped_block.h>
#include <array>
#include <vector>
#include <random>
#include <type_traits>
#include "../performance_test_0/get_time.h"

template <class T>
using VectorWrapper = std::vector<T>;

template <class T, std::size_t size>
class VectorArray : public std::vector<T>
{
	public:
		VectorArray() {
			this->reserve(size);
		}
};

int main(int argc, char* argv[])
{
	using namespace bridgerrholt::allocators;

	using Template = BitmappedBlockTemplate<VectorArray, 16, 16>;
	using Runtime  = BitmappedBlockRuntime <VectorWrapper>;

	std::mt19937 randomEngine {
		static_cast<typename std::mt19937::result_type>(bridgerrholt::getTime())
	};
	std::normal_distribution<> distribution {0, 5};

	//Runtime  r {{16, 16}};
	Template t;



	t.allocate(std::abs(std::round(distribution(randomEngine))) + 1);

	std::cout << std::is_literal_type<BitmappedBlockData<16>>::value << '\n';
}