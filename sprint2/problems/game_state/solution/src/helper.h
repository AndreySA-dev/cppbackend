#pragma once

#include <random>
#include <string>

namespace helper {

std::string URLDecode(const std::string_view encoded);

int random_int(int min, int max);

} // namespace helper