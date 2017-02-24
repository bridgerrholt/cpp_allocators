#include "generate_instructions.h"

namespace {

using namespace bridgerrholt::allocators::tests;



class Generator
{
	public:
		Generator(
			std::size_t    totalSize,
			std::size_t    minimumSize,
			std::size_t    maximumSize,
			GeneratorFlags flags) : totalSize_   {totalSize},
		                          minimumSize_ {minimumSize},
		                          maximumSize_ {maximumSize},
		                          flags_       {flags} {}

		std::string generate() {
			return {};
		}


	private:
		std::size_t    totalSize_;
		std::size_t    minimumSize_;
		std::size_t    maximumSize_;
		GeneratorFlags flags_;
};

}


namespace bridgerrholt {
	namespace allocators {
		namespace tests {

GeneratorFlags operator|(GeneratorFlags a, GeneratorFlags b) {
	return static_cast<GeneratorFlags>(
		static_cast<int>(a) | static_cast<int>(b)
	);
}

std::string generateInstructions(
	std::size_t    totalSize,
	std::size_t    minimumSize,
	std::size_t    maximumSize,
	GeneratorFlags flags)
{
	Generator generator {totalSize, minimumSize, maximumSize, flags};
	auto instructions = generator.generate();

	std::string toReturn;

	for (auto i : instructions) {
		toReturn += i + '\n';
	}

	return toReturn;
}

		}
	}
}