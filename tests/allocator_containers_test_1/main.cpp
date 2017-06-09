#include <memory>
#include <array>
#include <vector>
#include <iterator>

#include <allocators/wrappers/container_interface.h>
#include <allocators/blocks/block.h>
#include <allocators/bitmapped_block.h>

using namespace brh::allocators;

/*
template <class T>
class PointerInterface
{
	public:
		using This      = PointerInterface<T>;
		using BlockType = BasicBlock<T>;

		using difference_type   = std::ptrdiff_t;
		using value_type        = T;
		using pointer           = value_type *;
		using reference         = value_type &;
		using iterator_category = std::random_access_iterator_tag;


		constexpr PointerInterface() : PointerInterface(nullptr) {}
		constexpr PointerInterface(std::nullptr_t) :
			PointerInterface(BlockType()) {}

		constexpr PointerInterface(BlockType block) : block_ (block) {}

		constexpr friend void swap(This & first, This & second) {
			using std::swap;

			swap(first.block_, second.block_);
		}

		BlockType       & getBlock()       { return block_; }
		BlockType const & getBlock() const { return block_; }

		operator bool() { return !block_.isNull(); }

		friend bool operator==(This const & first,
		                       This const & second) {
			return (first.block_ == second.block_);
		}

		friend bool operator!=(This const & first,
		                       This const & second) {
			return (first.block_ != second.block_);
		}

		friend bool operator>(This const & first,
		                      This const & second) {
			return (first.block_.getPtr() > second.block_.getPtr());
		}

		friend bool operator<(This const & first,
		                      This const & second) {
			return (first.block_.getPtr() < second.block_.getPtr());
		}

		friend bool operator>=(This const & first,
		                       This const & second) {
			return (first.block_.getPtr() >= second.block_.getPtr());
		}

		friend bool operator<=(This const & first,
		                       This const & second) {
			return (first.block_.getPtr() <= second.block_.getPtr());
		}


		reference operator*() const {
			return *block_.getPtr();
		}

		pointer operator->() const {
			return block_.getPtr();
		}

		reference operator[](difference_type index) {
			return *(*this + index);
		}

		friend This & operator+=(This & ptr, difference_type amount) {
			ptr.block_.setPtr(ptr.block_.getPtr() + amount);
			return ptr;
		}

		friend This & operator-=(This & ptr, difference_type amount) {
			ptr.block_.setPtr(ptr.block_.getPtr() - amount);
			return ptr;
		}

		template <class N>
		friend This operator+(This first, N second) {
			return first += second;
		}

		template <class N>
		friend This operator-(This first, N second) {
			return first -= second;
		}

		template <class N>
		friend This operator+(N first, This second) {
			return second + first;
		}

		friend difference_type operator-(This first, This second) {
			return first.block_.getPtr() - second.block_.getPtr();
		}

		friend This & operator++(This & ptr) {
			ptr.block_.setPtr(ptr.block_.getPtr()+1);
			return ptr;
		}

		friend This & operator--(This & ptr) {
			ptr.block_.setPtr(ptr.block_.getPtr()-1);
			return ptr;
		}

		friend This operator++(This & ptr, int) {
			auto temp = ptr;
			++ptr;
			return temp;
		}

		friend This operator--(This & ptr, int) {
			auto temp = ptr;
			--ptr;
			return temp;
		}


	private:
		BlockType block_;
};


template <class A, class T>
class AllocatorInterface
{
	public:
		using pointer         = PointerInterface<T>;
		using const_pointer   = PointerInterface<T const>;
		using value_type      = T;
		using size_type       = std::size_t;
		using difference_type = std::ptrdiff_t;

		AllocatorInterface(A & allocator) : allocator_ {allocator} {}

		pointer allocate(size_type size) {
			return pointer(allocator_.allocate(size * sizeof(T)));
		}

		void deallocate(pointer ptr, size_type) {
			allocator_.deallocate(ptr.getBlock());
		}

	private:
		A & allocator_;
};
*/

template <class T>
class Test
{
	public:
		Test(T value) : value_(value) {
			std::cout << "Test(" << value << ")\n";
		}

		~Test() {
			std::cout << "~Test(" << value_ << ")\n";
		}


	private:
		T value_;
};

int main(int argc, char * argv[]) {
	using namespace brh::allocators;

	using Type = Test<int>;
	constexpr auto typeSize = sizeof(Type);

	using AllocatorCore = BitmappedBlock::Templated<std::array, 64, 1024>;
	using Allocator     = ContainerInterface<AllocatorCore, Type>;

	AllocatorCore core;
	Allocator     allocator {core};

	using Traits = std::allocator_traits<Allocator>;

	Traits::pointer ptr;

	//std::__split_buffer;

	std::vector<Type, Allocator> vector {allocator};

	typedef Type                                             value_type;
	typedef Allocator                                      allocator_type;
	typedef typename std::remove_reference<allocator_type>::type __alloc_rr;
	typedef std::allocator_traits<__alloc_rr>                    __alloc_traits;

	vector.push_back(1000);
	vector.push_back(100);
	vector.push_back(10);
	vector.push_back(1);
}