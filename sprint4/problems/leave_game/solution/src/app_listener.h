#pragma once
#include <chrono>

namespace listener {

class IApplicationListener {
  public:
	virtual void OnTick(std::chrono::milliseconds duration) = 0;
};

} // namespace listener