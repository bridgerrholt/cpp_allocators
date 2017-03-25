#include "run_stack_test.h"

#include "../corruption_test_0/instructions.h"

namespace {

using namespace bridgerrholt::allocators::tests;

class Instance
{
	public:
		Instance(GenerationArgPack args) :
			args_             {std::move(args)},
			status_           {false},
			size_             {0},
			instructionCount_ {0} {


		}

		bool status() const { return status_; }

	private:
		GenerationArgPack const args_;
		bool                    status_;
		std::size_t             size_;
		std::size_t             instructionCount_;
};

}


namespace bridgerrholt {
	namespace allocators {
		namespace tests {

bool runStackTest(GenerationArgPack args)
{
	Instance instance(args);

	return instance.status();
}

		}
	}
}