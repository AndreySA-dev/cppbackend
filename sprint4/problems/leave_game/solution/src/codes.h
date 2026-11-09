#pragma once

namespace game {
enum class Code { OK, NOT_FOUND, MAP_NOT_FOUND, BAD_REQUEST, ANOTHER_ERROR, UNKNOWN_ACTION };
}

namespace auth {
enum class Code { OK, INVALID_TOKEN_HEADER_MISSING, TOKEN_IS_INCORRECT, PLAYER_NOT_FOUND, MAP_NOT_FOUND };
}