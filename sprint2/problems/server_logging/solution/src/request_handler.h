#pragma once
#include "http_server.h"
#include "json_loader.h"
#include "log.h"
#include "model.h"
#include "perft_timer.h"

#include <filesystem>
// #include <iostream>
#include <optional>
#include <unordered_map>
// #include <utility>
#include <variant>

namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
namespace fs = std::filesystem;
using namespace std::literals;


// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;
using FileResponse = http::response<http::file_body>;

using CommonResponse = std::variant<StringResponse, FileResponse>;

// using ErrorResponse = std::optional<StringResponse>;

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
	constexpr static std::string_view API = "/api"sv;
	constexpr static std::string_view API_MAPS = "/api/v1/maps"sv;
	constexpr static std::string_view API_ONE_MAP = "/api/v1/maps/"sv;
};

struct ResponseTexts {
	ResponseTexts() = delete;
	constexpr static std::string_view MAP_NOT_FOUND = R"({"code" : "mapNotFound", "message" : "Map not found"})"sv;
	constexpr static std::string_view BAD_REQUEST = R"({"code" : "badRequest", "message" : "Bad request"})"sv;
	constexpr static std::string_view FILE_NOT_FOUND = "File not found"sv;
	constexpr static std::string_view FILE_INCORRECT_PATH = "Incorrect path to file"sv;
};


class RequestHandler {
  public:
	explicit RequestHandler(model::Game& game, const std::string& root_path) : game_{game}, wwwroot_path_{root_path} {}

	RequestHandler(const RequestHandler&) = delete;
	RequestHandler& operator=(const RequestHandler&) = delete;

	template <typename Body, typename Allocator, typename Send>
	http::header<false> operator()(
		http::request<Body, http::basic_fields<Allocator>>&& req, const tcp::socket& socket, Send&& send) {

		CommonResponse resp;


		const auto& target = req.target();


		if (target == RequestsTexts::API_MAPS) {
			// target = "/api/v1/maps/" -> get list of all maps

			std::string maps = GetMapList();
			resp = GetStringResponse(maps, http::status::ok, ContentType::APP_JSON, req.keep_alive());

		} else if (target.starts_with(RequestsTexts::API_ONE_MAP) &&
				   target.size() > RequestsTexts::API_ONE_MAP.size()) {
			// target = "/api/v1/maps/...." -> get one map by id

			auto map_id = target.substr(RequestsTexts::API_ONE_MAP.size());
			auto maps = GetMap(std::string(map_id));

			if (maps) {
				resp = GetStringResponse(*maps, http::status::ok, ContentType::APP_JSON, req.keep_alive());
			} else {
				resp = GetStringResponse(
					ResponseTexts::MAP_NOT_FOUND, http::status::not_found, ContentType::APP_JSON, req.keep_alive());
			}

		} else if (target.starts_with(RequestsTexts::API)) {

			resp = GetStringResponse(
				ResponseTexts::BAD_REQUEST, http::status::bad_request, ContentType::APP_JSON, req.keep_alive());

		} else {
			// get file from wwwroot directory

			resp = GetFileResponse(std::string(target), req.keep_alive());

		}

		http::header<false> returned_resp;
		std::visit([&returned_resp](auto&& result) { returned_resp = result; }, resp);

		std::visit([&send](auto&& result) { send(std::forward<decltype(result)>(result)); }, resp);

		return returned_resp;
	}


  private:
	std::string GetMapList();

	std::optional<std::string> GetMap(const std::string& id);

	CommonResponse GetFileResponse(std::string target, bool keep_alive);

	CommonResponse GetStringResponse(
		std::string_view text, http::status status, std::string_view type, bool keep_alive);

	// CommonResponse GetResponse(std::string_view text, http::status status, bool keep_alive);

	std::string_view GetTypeByExt(std::string_view ext) const;

	model::Game& game_;
	std::filesystem::path wwwroot_path_;
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

		perf_timer<std::chrono::milliseconds> timer;
		auto resp = decorated_(std::move(req), socket, send);
		timer.stop();


		json::value jcontent_type;
		int code = -1;


		code = resp.result_int();
		if (const auto it = resp.find("Content-Type"); it != resp.cend()) {
			jcontent_type.emplace_string() = std::string(it->value());
		};
		srv_log::LogMessage({{"response_time", timer.get_duration()}, {"code", code}, {"content_type", jcontent_type}},
			"response sent"sv);
	}

  private:
	RealHandler& decorated_;
};

} // namespace http_handler
