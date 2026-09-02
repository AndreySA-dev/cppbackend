#include "request_handler.h"

namespace http_handler {


std::string RequestHandler::GetMapList() {

	json::array jmaps;
	for (auto& map : game_.GetMaps()) {
		json::object jo;
		json_loader::MapInfoToJSON(map, jo);
		jmaps.push_back(std::move(jo));
	}

	return json::serialize(jmaps);
}

std::optional<std::string> RequestHandler::GetMap(const std::string& id) {

	const auto* map_ptr = game_.FindMap(model::Map::Id(id));
	if (map_ptr) {
		json::object jmap;
		json_loader::MapToJSON(*map_ptr, jmap);
		return json::serialize(std::move(jmap));
	}

	return std::nullopt;
}

StringResponse RequestHandler::GetResponse(std::string_view text, http::status status, bool keep_alive) {

	StringResponse response(status, 11);
	response.keep_alive(keep_alive);
	response.body() = text;
	response.content_length(text.size());
	response.set(http::field::content_type, ContentType::APP_JSON);

	return response;
}


} // namespace http_handler
