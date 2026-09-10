#include "log.h"

#include <boost/json.hpp>

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/date_time.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>

namespace server_log {

namespace logging = boost::log;
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace json = boost::json;

BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(json_data, "JSONData", json::value)

void MyFormatter(logging::record_view const& rec, logging::formatting_ostream& strm) {
	// чтобы поставить логгеры в равные условия, уберём всё лишнее
	auto ts = rec[timestamp];
	strm << to_iso_extended_string(*ts) << ": ";
	strm << rec[json_data];
	// выводим само сообщение
	strm << rec[expr::smessage];
}

void InitBoostLogFilter() {
	logging::add_common_attributes();
	logging::add_console_log(std::cout, keywords::auto_flush = true, keywords::format = &MyFormatter);
	// logging::add_console_log(std::cout, keywords::auto_flush = true);

	// logging::add_file_log(keywords::file_name = "sample.log", keywords::format = &MyFormatter);
}

} // namespace loggin