#ifndef BRH_GET_TIME_H
#define BRH_GET_TIME_H

#include <chrono>

namespace brh {

template <class T = std::chrono::microseconds>
std::ptrdiff_t getTime() {
	auto now = std::chrono::system_clock::now().time_since_epoch();
	return std::chrono::duration_cast<T>(now).count();
}

}

#endif
