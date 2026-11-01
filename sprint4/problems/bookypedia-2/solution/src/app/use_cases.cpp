#include "use_cases.h"

namespace app {
	
UseCasesGuard::UseCasesGuard(UseCases& use_cases) : use_cases_(use_cases) {
	use_cases_.Start();
}

UseCasesGuard::~UseCasesGuard() {
	use_cases_.Interrupt();
}

} // namespace app