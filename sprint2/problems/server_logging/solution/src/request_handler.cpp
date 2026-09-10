#include "request_handler.h"
#include "helper.h"

namespace http_handler {

namespace fs = std::filesystem;

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

StringResponse RequestHandler::GetResponse(
	std::string_view text, http::status status, std::string_view type, bool keep_alive) {

	StringResponse response(status, 11);
	response.keep_alive(keep_alive);
	response.body() = text;
	response.content_length(text.size());
	response.set(http::field::content_type, type);

	return response;
}


StringResponse RequestHandler::GetResponse(std::string_view text, http::status status, bool keep_alive) {

	StringResponse response(status, 11);
	response.keep_alive(keep_alive);
	response.body() = text;
	response.content_length(text.size());
	response.set(http::field::content_type, ContentType::APP_JSON);

	return response;
}


std::string_view RequestHandler::GetTypeByExt(std::string_view ext) const {
	if (auto it = MIMEs.find(ext); it != MIMEs.end()) {
		return it->second;
	}
	return ContentType::APP_OCTETSTREAM;
}

std::pair<std::optional<FileResponse>, ErrorResponse> RequestHandler::GetFile(std::string target, bool keep_alive) {

	target = helper::URLDecode(target);

	if (target.size() == 0 || target[0] != '/') {

		StringResponse err_resp = GetResponse(
			"Error: incorrect resource in request.", http::status::bad_request, ContentType::TEXT_PLAIN, keep_alive);
		return {std::nullopt, std::move(err_resp)};
	}

	fs::path req_path;
	fs::path root_path(fs::weakly_canonical(wwwroot_path_));

	if (target != "/"sv) {

		req_path = fs::weakly_canonical(root_path / fs::path(target.substr(1)));

		bool path_in_root = true;

		for (auto r = root_path.begin(), p = req_path.begin(); r != root_path.end(); ++r, ++p) {
			if (p == req_path.end() || *p != *r) {
				path_in_root = false;
			}
		}

		if (!path_in_root) {
			StringResponse err_resp = GetResponse("Error: incorrect resource in request.", http::status::bad_request,
				ContentType::TEXT_PLAIN, keep_alive);
			return {std::nullopt, std::move(err_resp)};
		}

	} else {
		req_path = root_path / fs::path("index.html");
	}

	if (!fs::exists(req_path)) {
		StringResponse err_resp =
			GetResponse("Error: File not found."sv, http::status::not_found, ContentType::TEXT_PLAIN, keep_alive);
		return {std::nullopt, std::move(err_resp)};
	}

	std::string_view content_type = GetTypeByExt(req_path.extension().string());

	FileResponse resp;
	resp.version(11);
	resp.insert(http::field::content_type, content_type);

	http::file_body::value_type file;
	sys::error_code ec;
	file.open(req_path.c_str(), beast::file_mode::read, ec);

	if (ec) {
		StringResponse err_resp = GetResponse(
			"Error: Open file error."sv, http::status::internal_server_error, ContentType::TEXT_PLAIN, keep_alive);
		return {std::nullopt, std::nullopt};
	}

	resp.body() = std::move(file);
	resp.prepare_payload();

	return {std::move(resp), std::nullopt};
}

} // namespace http_handler
