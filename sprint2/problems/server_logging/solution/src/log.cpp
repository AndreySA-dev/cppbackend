#include "log.h"

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/date_time.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/attributes/value_extraction_fwd.hpp>
#include <boost/log/attributes/value_extraction.hpp>

namespace srv_log {

namespace logging = boost::log;
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace json = boost::json;

// BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(json_data, "JSONData", json::object)

void MyFormatter(logging::record_view const& rec, logging::formatting_ostream& strm) {
	
	auto ts = rec[timestamp];
	std::string ts_str = to_iso_extended_string(*ts);
	json::object resp{{"timestamp", std::move(ts_str)},  {"data", *rec[json_data]}, {"message", *rec[expr::smessage]}};
	strm << resp;

}

void LogMessage(json::object data, const std::string_view message) {
	BOOST_LOG_TRIVIAL(info) <<  logging::add_value(std::move(json_data), std::move(data)) << message;
}

void InitBoostLogFilter() {

	logging::add_common_attributes();
	logging::add_console_log(std::cout, keywords::auto_flush = true, keywords::format = &MyFormatter);
	
}

} // namespace loggin