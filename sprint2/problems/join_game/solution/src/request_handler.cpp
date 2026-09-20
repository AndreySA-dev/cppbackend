#include "request_handler.h"
#include "authenticator.h"
#include "helper.h"
#include "token.h"

#include <iostream>

namespace http_handler {

using namespace std;
namespace fs = std::filesystem;
using namespace std::literals;
using namespace game_handler;
namespace net = boost::asio;


RequestHandler::RequestHandler(
	model::Game& game, game_handler::GameHandler& game_handler, const std::string& root_path, net::io_context& ctx)
	: game_{game}, game_handler_(game_handler), wwwroot_path_{root_path}, api_strand_{net::make_strand(ctx)} {}


StringResponse RequestHandler::HandleAPIRequest(HTTPRequest req) {

	StringResponse resp;

	auto target = req.target();

	if (target.starts_with(RequestsTexts::API_MAPS)) {
		// API request for MAP ==========================================

		auto [resp_j, code] = game_handler_.HandleAPIMapRequest(target);
		http::status status;
		if (code == game::Code::OK) {
			status = http::status::ok;
		} else if (code == game::Code::NOT_FOUND) {
			status = http::status::not_found;
		} else {
			status = http::status::bad_request;
		}

		resp = GetStringResponse(json::serialize(resp_j), status, ContentType::APP_JSON);

	} else if (target.starts_with(RequestsTexts::API_GAME_JOIN)) {
		// API requst to join to game =============================

		resp = HandleHttpGameJoinRequest(req);

	} else if (target.starts_with(RequestsTexts::API_GAME_PLAYERS)) {
		// API requst get all players =============================

		resp = HandleHttpGetPlayersRequest(req);

	}

	return resp;
}

StringResponse RequestHandler::HandleHttpGameJoinRequest(HTTPRequest req) {

	StringResponse join_resp;

	// allow only post method
	if (req.method() != http::verb::post) {
		join_resp = GetStringResponse(
			ResponseTemplates::INVALID_METHOD_ONLY_POST, http::status::method_not_allowed, ContentType::APP_JSON);
		join_resp.set(http::field::cache_control, "no-cache"sv);
		join_resp.set(http::field::allow, "POST");
		return join_resp;
	}

	std::string_view name;
	std::string_view map_id;

	// check JSON data is correct
	try {

		auto data_j = json::parse(req.body());
		name = data_j.as_object().at("userName").as_string();
		map_id = data_j.as_object().at("mapId").as_string();

	} catch (...) {
		// std::cerr << e.what() << std::endl;
		join_resp = GetStringResponse(
			ResponseTemplates::INVALID_ARGUMENT_PARSE_BODY_ERROR, http::status::bad_request, ContentType::APP_JSON);
		join_resp.set(http::field::cache_control, "no-cache"sv);
		return join_resp;
	}

	if (name.size() == 0) {
		join_resp = GetStringResponse(
			ResponseTemplates::INVALID_ARGUMENT_INVALID_NAME, http::status::bad_request, ContentType::APP_JSON);
		join_resp.set(http::field::cache_control, "no-cache"sv);
		return join_resp;
	}

	auto [result_j, code] = game_handler_.HandleGameJoinRequest(name, map_id);
	if (code == game::Code::OK) {
		join_resp = GetStringResponse(json::serialize(result_j), http::status::ok, ContentType::APP_JSON);
	} else if (code == game::Code::MAP_NOT_FOUND) {
		join_resp = GetStringResponse(ResponseTemplates::MAP_NOT_FOUND, http::status::not_found, ContentType::APP_JSON);
	} else {
		join_resp = GetStringResponse(
			ResponseTemplates::SERVER_INTERNAL_ERROR, http::status::internal_server_error, ContentType::APP_JSON);
	}
	join_resp.set(http::field::cache_control, "no-cache"sv);

	return join_resp;
}

StringResponse RequestHandler::HandleHttpGetPlayersRequest(HTTPRequest req) {

	StringResponse resp;

	// check method GET or HEAD
	if (!(req.method() == http::verb::get || req.method() == http::verb::head)) {
		resp = GetStringResponse(
			ResponseTemplates::INVALID_METHOD, http::status::method_not_allowed, ContentType::APP_JSON);
		resp.set(http::field::cache_control, "no-cache"sv);
		resp.set(http::field::allow, "GET, HEAD");
		return resp;
	}

	// parse and check Authorization string as "Bearer 6516861d89ebfff147bf2eb2b5153ae1"
	auto auth_str = req["Authorization"];
	string_view auth_prefix = auth_str.substr(0, ResponseTemplates::BEARER_FIELD_PREFIX.size());
	string_view token_str;
	if (auth_prefix != ResponseTemplates::BEARER_FIELD_PREFIX &&
		auth_str.size() <= ResponseTemplates::BEARER_FIELD_PREFIX.size()) {
		resp = GetStringResponse(
			ResponseTemplates::INVALID_TOKEN_AUTH_HEADER_MISSING, http::status::unauthorized, ContentType::APP_JSON);
		resp.set(http::field::cache_control, "no-cache"sv);
		return resp;
	}

	// Get player with whit requested token
	token_str = auth_str.substr(ResponseTemplates::BEARER_FIELD_PREFIX.size());
	auto player_result = game_handler_.GetAuthenticator().GetPlayer(token::Token(std::string(token_str)));

	// Token has incorrect format
	if (player_result.second == auth::Code::TOKEN_IS_INCORRECT) {
		resp = GetStringResponse(
			ResponseTemplates::INVALID_TOKEN_AUTH_HEADER_MISSING, http::status::unauthorized, ContentType::APP_JSON);
		resp.set(http::field::cache_control, "no-cache"sv);
		return resp;
	}

	// User with this token has not found
	if (player_result.second == auth::Code::PLAYER_NOT_FOUND) {
		resp = GetStringResponse(
			ResponseTemplates::UNKNOWN_TOKEN_PLAYER_NOT_FOUND, http::status::unauthorized, ContentType::APP_JSON);
		resp.set(http::field::cache_control, "no-cache"sv);
		return resp;
	}

	// get full list of players
	auto [players_j, code] = game_handler_.HandleGetPlayersRequest();
	resp = GetStringResponse(json::serialize(players_j), http::status::ok, ContentType::APP_JSON);
	resp.set(http::field::cache_control, "no-cache"sv);
	return resp;
}


StringResponse RequestHandler::GetStringResponse(
	std::string_view text, http::status status, std::string_view type) {

	StringResponse response(status, 11);
	response.body() = text;
	response.content_length(text.size());
	response.set(http::field::content_type, type);

	return response;
}


std::string_view RequestHandler::GetTypeByExt(std::string_view ext) const {
	if (auto it = MIMEs.find(ext); it != MIMEs.end()) {
		return it->second;
	}
	return ContentType::APP_OCTETSTREAM;
}

CommonResponse RequestHandler::GetFileResponse(std::string target) {

	target = helper::URLDecode(target);

	if (target.size() == 0 || target[0] != '/') {
		// request is empty or not start with '/'
		return {GetStringResponse("Error: incorrect resource in request.", http::status::bad_request,
			ContentType::TEXT_PLAIN /*, keep_alive */)};
	}

	fs::path req_path;
	fs::path root_path(fs::weakly_canonical(wwwroot_path_));

	if (target != "/"sv) {
		// Get static file (not index.html)

		req_path = fs::weakly_canonical(root_path / fs::path(target.substr(1)));

		bool path_in_root = true;
		for (auto r = root_path.begin(), p = req_path.begin(); r != root_path.end(); ++r, ++p) {
			if (p == req_path.end() || *p != *r) {
				path_in_root = false;
			}
		}

		if (!path_in_root) {
			// Request file with incorrect path, for example /directory/../../file.name

			return {GetStringResponse(
				"Error: incorrect resource in request.", http::status::bad_request, ContentType::TEXT_PLAIN)};
				
		}

	} else {
		req_path = root_path / fs::path("index.html");
	}

	if (!fs::exists(req_path)) {
		return {GetStringResponse("Error: File not found."sv, http::status::not_found, ContentType::TEXT_PLAIN)};
	}

	std::string_view content_type = GetTypeByExt(req_path.extension().string());

	FileResponse resp;
	resp.version(11);
	resp.insert(http::field::content_type, content_type);

	http::file_body::value_type file;
	sys::error_code ec;
	file.open(req_path.c_str(), beast::file_mode::read, ec);

	if (ec) {
		return {GetStringResponse(
			"Error: Open file error."sv, http::status::internal_server_error, ContentType::TEXT_PLAIN)};
	}

	resp.body() = std::move(file);
	resp.prepare_payload();

	return {std::move(resp)};
}

} // namespace http_handler
