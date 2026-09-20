#pragma once

#include "authenticator.h"
#include "game_handler.h"
#include "http_server.h"
#include "json_loader.h"
#include "log.h"
#include "model.h"
#include "perf_timer.h"

#include <filesystem>
#include <optional>
#include <unordered_map>
#include <variant>

namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
namespace fs = std::filesystem;
using namespace std::literals;
namespace net = boost::asio;


// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;
using FileResponse = http::response<http::file_body>;

using CommonResponse = std::variant<StringResponse, FileResponse>;


const std::unordered_map<std::string_view, std::string_view> MIMEs = {
	{".htm"sv, "text/html"sv},
	{".html"sv, "text/html"sv},
	{".css"sv, "text/css"sv},
	{".txt"sv, "text/plain"sv},
	{".js"sv, "text/javascript"sv},
	{".json"sv, "application/json"sv},
	{".xml"sv, "application/xml"sv},
	{".png"sv, "image/png"sv},
	{".jpg"sv, "image/jpeg"sv},
	{".jpe"sv, "image/jpeg"sv},
	{".jpeg"sv, "image/jpeg"sv},
	{".gif"sv, "image/gif"sv},
	{".bmp"sv, "image/bmp"sv},
	{".ico"sv, "image/vnd.microsoft.icon"sv},
	{".tiff"sv, "tif: image/tiff"sv},
	{".tif"sv, "image/tiff"sv},
	{".svg"sv, "image/svg+xml"sv},
	{".svgz"sv, "image/svg+xml"sv},
	{".mp3"sv, "audio/mpeg"sv},
};


struct ContentType {
	ContentType() = delete;
	constexpr static std::string_view TEXT_HTML = "text/html"sv;
	constexpr static std::string_view TEXT_PLAIN = "text/plain"sv;
	constexpr static std::string_view APP_JSON = "application/json"sv;
	constexpr static std::string_view APP_OCTETSTREAM = "application/octet-stream"sv;
};


struct RequestsTexts {
	RequestsTexts() = delete;
	constexpr static std::string_view API = "/api/"sv;
	constexpr static std::string_view API_MAPS = "/api/v1/maps"sv;
	constexpr static std::string_view API_ONE_MAP = "/api/v1/maps/"sv;
	constexpr static std::string_view API_GAME = "/api/v1/game"sv;
	constexpr static std::string_view API_GAME_JOIN = "/api/v1/game/join"sv;
	constexpr static std::string_view API_GAME_PLAYERS = "/api/v1/game/players"sv;
};


struct ResponseTemplates {
	ResponseTemplates() = delete;
	constexpr static std::string_view MAP_NOT_FOUND = R"({"code" : "mapNotFound", "message" : "Map not found"})"sv;

	constexpr static std::string_view BAD_REQUEST = R"({"code" : "badRequest", "message" : "Bad request"})"sv;

	constexpr static std::string_view INVALID_ARGUMENT_PARSE_BODY_ERROR =
		R"({"code" : "invalidArgument", "message" : "Join game request parse error"})"sv;


	constexpr static std::string_view INVALID_ARGUMENT_INVALID_NAME =
		R"({"code" : "invalidArgument", "message" : "Invalid name"})"sv;

	constexpr static std::string_view INVALID_METHOD =
		R"({"code" : "invalidMethod", "message" : "Invalid method"})"sv;

	constexpr static std::string_view INVALID_METHOD_ONLY_POST =
		R"({"code" : "invalidMethod", "message" : "Only POST method is expected"})"sv;

	constexpr static std::string_view FILE_NOT_FOUND = "File not found"sv;

	constexpr static std::string_view FILE_INCORRECT_PATH = "Incorrect path to file"sv;

	constexpr static std::string_view SERVER_INTERNAL_ERROR =
		R"({"code" : "serverInternalError", "message" : "Server internal error"})"sv;

	constexpr static std::string_view BEARER_FIELD_PREFIX = R"(Bearer )"sv;

	constexpr static std::string_view INVALID_TOKEN_AUTH_HEADER_MISSING =
		R"({"code" : "invalidToken", "message" : "Authorization header is missing"})"sv;

	constexpr static std::string_view UNKNOWN_TOKEN_PLAYER_NOT_FOUND =
		R"({"code" : "unknownToken", "message" : "Player token has not been found"})"sv;
};


class RequestHandler {
  public:
	explicit RequestHandler(
		model::Game& game, game_handler::GameHandler& game_handler, const std::string& root_path, net::io_context& ctx);

	using HTTPRequest = http::request<http::string_body>;

	RequestHandler(const RequestHandler&) = delete;
	RequestHandler& operator=(const RequestHandler&) = delete;

	template <typename Body, typename Allocator, typename Send>
	http::header<false> operator()(http::request<Body, http::basic_fields<Allocator>>&& req,
		[[maybe_unused]] const tcp::socket& socket, Send&& send) {

		auto target = req.target();
		CommonResponse resp;

		if (target.starts_with(RequestsTexts::API)) {
			// API request

			resp = HandleAPIRequest(req);

		} else {
			// get a file from www directory

			resp = GetFileResponse(std::string(target));

		}

		http::header<false> returned_resp;

		// std::visit([&returned_resp](auto&& result) { returned_resp = result; }, resp);

		std::visit(
			[&req, &send](auto&& result) {
				result.keep_alive(req.keep_alive());
				send(std::forward<decltype(result)>(result));
			},
			resp);

		return returned_resp;
	}


  private:
	StringResponse HandleAPIRequest(HTTPRequest req);
	StringResponse HandleHttpGameJoinRequest(HTTPRequest req);
	StringResponse HandleHttpGetPlayersRequest(HTTPRequest req);

	CommonResponse GetFileResponse(std::string target);

	StringResponse GetStringResponse(
		std::string_view text, http::status status, std::string_view type);

	std::string_view GetTypeByExt(std::string_view ext) const;

	model::Game& game_;
	game_handler::GameHandler& game_handler_;
	std::filesystem::path wwwroot_path_;
	net::strand<net::io_context::executor_type> api_strand_;
};


template <class RealHandler>
class LoggingRequestHandler {


	//  static void LogRequest(const Request& r);
	//  static void LogResponse(const Response& r);
  public:
	LoggingRequestHandler(RealHandler& handler) : decorated_(handler) {};

	template <typename Body, typename Allocator, typename Send>
	void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, const tcp::socket& socket, Send&& send) {


		std::string addr = socket.remote_endpoint().address().to_string();
		srv_log::LogMessage({{"ip", addr}, {"URI", req.target()}, {"method", req.method_string()}, {"address", addr}},
			"request received"sv);

		perf_timer<std::chrono::microseconds> timer;

		auto log_send = [timer, send](auto&& resp) mutable {
			timer.stop();

			std::string contnent_type_str;
			if (const auto it = resp.find("Content-Type"); it != resp.cend()) {
				contnent_type_str = std::string(it->value());
			};

			srv_log::LogMessage({{"response_time", timer.get_duration()}, {"code", resp.result_int()},
									{"content_type", contnent_type_str}},
				"response sent"sv);

			send(std::move(resp));
		};

		decorated_(std::move(req), socket, log_send);

	}

  private:
	RealHandler& decorated_;
};

} // namespace http_handler
