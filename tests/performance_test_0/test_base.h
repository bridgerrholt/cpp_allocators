#ifndef BRH_CPP_ALLOCATORS_TESTS_PERFORMANCE_TEST_0_TEST_BASE_H
#define BRH_CPP_ALLOCATORS_TESTS_PERFORMANCE_TEST_0_TEST_BASE_H

#include <string>

namespace brh {
	namespace allocators {
		namespace tests {

class TestBase {
	public:
		TestBase(std::string name) : name_{name} {}

		virtual ~TestBase() = 0;

		virtual void initialize() = 0;
		virtual void construct()  = 0;
		virtual void destruct()   = 0;

		virtual void restart() = 0;

		static constexpr std::size_t testCount{3};

		std::string const & getName() const { return name_; }


	private:
		std::string name_;
};


		}
	}
}


#endif
