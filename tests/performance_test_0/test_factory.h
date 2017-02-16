#ifndef BRH_CPP_ALLOCATORS_TESTS_PERFORMANCE_TEST_0_TEST_FACTORY_H
#define BRH_CPP_ALLOCATORS_TESTS_PERFORMANCE_TEST_0_TEST_FACTORY_H

#include "test_base.h"

#include <memory>
#include <tuple>

namespace bridgerrholt {
	namespace allocators {
		namespace tests {

class TestFactoryBase
{
	public:
		virtual std::shared_ptr<TestBase> create() = 0;
};

template <class TestType>
class BasicTestFactory
{
	public:
		BasicTestFactory(std::string name) : name_(std::move(name)) {}

		std::shared_ptr<TestBase> create() {
			return std::make_shared<TestBase>(TestType());
		}

	private:
		std::string name_;
};

		}
	}
}

#endif
