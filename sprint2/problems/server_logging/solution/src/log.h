#pragma once

// #include <string>

#include <boost/json.hpp>



namespace srv_log {

namespace json = boost::json;

void InitBoostLogFilter();
void LogMessage(json::object data, const std::string_view message);



} // namespace logger