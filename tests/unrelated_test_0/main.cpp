#include <iostream>
#include <array>
#include <vector>


template <template <class T, std::size_t size> class ArrayType,
	std::size_t blockSize, std::size_t blockCount>
class BlockArrayHelperTemplate
{
	public:
		template <class T>
		using Type = ArrayType<T, blockSize * blockCount>;
};


template <template <class T> class ArrayType, class T>
class ArrayRuntimeWrapper : public ArrayType<T>
{
	public:
		ArrayRuntimeWrapper(std::size_t size) : ArrayType<T> (size) { }
};

template <template <class T> class ArrayType, class T>
class BlockArrayRuntimeWrapper : public ArrayRuntimeWrapper<ArrayType, T>
{
	public:
		BlockArrayRuntimeWrapper(std::size_t blockSize, std::size_t blockCount) :
			ArrayRuntimeWrapper<ArrayType, T>(blockSize * blockCount) {}
};

template <template <class T> class ArrayType>
class BlockArrayHelperRuntime
{
	public:
		template <class T>
		using Type = BlockArrayRuntimeWrapper<ArrayType, T>;
};


template <class ArrayPolicy>
class BlockArray
{
	public:
		using ArrayType = typename ArrayPolicy::template Type<char>;

		BlockArray(ArrayType array = ArrayType()) : arr (std::move(array)) {}

		ArrayType arr;
};


template <template <class T, std::size_t size> class ArrayType,
	std::size_t blockSize, std::size_t blockCount>
using BlockArrayTemplate =
	BlockArray<BlockArrayHelperTemplate<ArrayType, blockSize, blockCount> >;

template <template <class T> class ArrayType>
using BlockArrayRuntime = BlockArray<BlockArrayHelperRuntime<ArrayType> >;

template <class T>
using VectorWrapper = std::vector<T>;

int main(int argc, char* argv[])
{
	using ArrayTemplate = BlockArrayTemplate<std::array, 16, 16>;
	using ArrayRuntime  = BlockArrayRuntime<VectorWrapper>;

	ArrayTemplate t;
	ArrayRuntime  r {{16, 16}};

	std::cout << "t " << &t << " " << t.arr.end() - t.arr.begin() << '\n';
	std::cout << "r " << &r << " " << r.arr.end() - r.arr.begin() << '\n';
}