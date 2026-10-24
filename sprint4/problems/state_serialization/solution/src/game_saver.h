#pragma once
#include "game_serialization.h"
#include "game_handler.h"
#include <filesystem>

namespace fs = std::filesystem;

void SaveState(game_handler::GameHandler game_h, fs::path file_path);