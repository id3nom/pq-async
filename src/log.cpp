/*
	This file is part of libpq-async++
	Copyright (C) 2011-2018 Michel Denommee (and other contributing authors)
	
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <fstream>
#include <sstream>
#include <chrono>
#include <unistd.h>
#include <regex>
#include <iostream>
#include <iomanip>
#include <limits.h>

#include "log.h"

namespace pq_async{

static __thread char log_line[PQ_ASYNC_MAX_LOG_LENGTH];
std::string log::s_app_name;
log_level log::s_level = log_level::ll_info;
bool log::s_source = false;
bool log::s_color = false;

char s_hostname[HOST_NAME_MAX];
pid_t s_pid;
std::string s_machine_id;

log::log(){}
log::~log(){}

void log::init(const std::string& app_name, int level, bool enable_source)
{
	// set log level
	switch(level){
		case 0:
			pq_async::log::set_level(log_level::ll_fatal);
			break;
		case 1:
			pq_async::log::set_level(log_level::ll_error);
			break;
		case 2:
			pq_async::log::set_level(log_level::ll_warning);
			break;
		case 3:
			pq_async::log::set_level(log_level::ll_info);
			break;
		case 4:
			pq_async::log::set_level(log_level::ll_debug);
			break;
		default:
			pq_async::log::set_level(log_level::ll_trace);
			level = (int)log_level::ll_trace;
			break;
	}
	
	// get hostname
	memset(s_hostname, 0, HOST_NAME_MAX);
	if(gethostname(s_hostname, HOST_NAME_MAX) != 0)
		throw pq_async::exception("unable to get computer hostname!");
	
	// get process pid
	s_pid = getpid();
	
	// read machine id from /var/lib/dbus/machine-id
	std::ifstream ifs("/var/lib/dbus/machine-id");
	s_machine_id.assign(
		(std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>())
		);
	pq_async::trim(s_machine_id);
	
	s_app_name = app_name;
	s_source = enable_source;
	
	pq_async_log_debug(
		"logging class is initialized\n"
		"Minimum verbosity level was set to %s\nshow source is %s\n", 
		log::level_to_str((log_level)level).c_str(),
		s_source ? "enabled" : "disabled"
		);
}

const std::string& log::machine_id()
{
	return s_machine_id;
}

std::string get_timestamp(){
	const auto currentTime = std::chrono::system_clock::now();
	time_t time = std::chrono::system_clock::to_time_t(currentTime);
	auto currentTimeRounded = std::chrono::system_clock::from_time_t(time);
	if( currentTimeRounded > currentTime ) {
		--time;
		currentTimeRounded -= std::chrono::seconds( 1 );
	}
	//const tm *values = localtime( &time );
	struct tm t;
	localtime_r(&time, &t);
	int hours = t.tm_hour;
	int minutes = t.tm_min;
	int seconds = t.tm_sec;
	// etc.
	int milliseconds = std::chrono::duration_cast<std::chrono::duration<int,std::milli> >( currentTime - currentTimeRounded ).count( );
	
	std::stringstream ss;
	ss << std::setfill('0') << std::setw(2) << hours << ":" <<
		std::setfill('0') << std::setw(2) << minutes << ":" <<
		std::setfill('0') << std::setw(2) << seconds << ":" <<
		std::setfill('0') << std::setw(3) << milliseconds;
	// ss << boost::format("%|02|")%hours << ":" << 
	// 	boost::format("%|02|")%minutes << ":" << 
	// 	boost::format("%|02|")%seconds << "." << 
	// 	boost::format("%|03|")%milliseconds;
	return ss.str();
}

std::string log::level_to_str(log_level level)
{
	switch(level){
		case log_level::ll_fatal: return std::string("FATAL");
		case log_level::ll_error: return std::string("ERROR");
		case log_level::ll_warning: return std::string("WARNING");
		case log_level::ll_info: return std::string("INFO");
		case log_level::ll_debug: return std::string("DEBUG");
		case log_level::ll_trace: return std::string("TRACE");
		default: return std::string("N/A");
	}
}

std::string prepare_header(const char* color, const char* hint, const char* ts, bool is_core)
{
	memset(log_line, 0, PQ_ASYNC_MAX_LOG_LENGTH);
	if(log::is_color_enabled())
		snprintf(log_line, PQ_ASYNC_MAX_LOG_LENGTH -1, "\x1b[%sm%s%s-%s:\x1b[0m", color, is_core ? "CORE-" : "", hint, ts);
	else
		snprintf(log_line, PQ_ASYNC_MAX_LOG_LENGTH -1, "%s%s-%s:", is_core ? "CORE-" : "", hint, ts);
	return std::string(log_line);
}

std::string prepare_footer(const char* color)
{
	memset(log_line, 0, PQ_ASYNC_MAX_LOG_LENGTH);
	if(log::is_color_enabled())
		snprintf(log_line, PQ_ASYNC_MAX_LOG_LENGTH -1, "\n\x1b[%sm¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\x1b[0m", color);
	else
		snprintf(log_line, PQ_ASYNC_MAX_LOG_LENGTH -1, "\n¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯");
	return std::string(log_line);	
}

std::string exec(const char* cmd) {
	char buffer[128];
	std::string result = "";
	FILE* pipe = popen(cmd, "r");
	if (!pipe)
		throw std::runtime_error("popen() failed!");
	
	try {
		while (!feof(pipe)) {
			if (fgets(buffer, 128, pipe) != NULL)
				result += buffer;
		}
	} catch (...) {
		pclose(pipe);
		throw;
	}
	
	pclose(pipe);
	return result;
}

void log::write(log_level level, const char* log_name, const char* file, int line, const char* fn, const char* msg, va_list arg)
{
	std::stringstream output;
	std::string ts = get_timestamp();
	
	bool is_core = false;
	
	switch(level){
		case log_level::ll_fatal:
			output << prepare_header("31;1", "FATAL", ts.c_str(), is_core);
			break;
		case log_level::ll_error:
			output << prepare_header("31", "ERROR", ts.c_str(), is_core);
			break;
		case log_level::ll_warning:
			output << prepare_header("33;1", "WARN", ts.c_str(), is_core);
			break;
		case log_level::ll_info:
			output << prepare_header("34;1", "INFO", ts.c_str(), is_core);
			break;
		case log_level::ll_debug:
			output << prepare_header("36", "DEBUG", ts.c_str(), is_core);
			break;
		case log_level::ll_trace:
			output << prepare_header("37", "TRACE", ts.c_str(), is_core);
			break;
		default:
			output << prepare_header("37", "N/A", ts.c_str(), is_core);
			break;
	}
	
	//std::string label("LogName@hostname:pid:machineId");
	if(s_color)
		output << "[\x1b[35m" << log_name << "@" << s_hostname << ":" << s_pid << ":" << s_machine_id << "\x1b[0m] ";
	else
		output << "[" << log_name << "@" << s_hostname << ":" << s_pid << ":" << s_machine_id << "] ";
		
	if(s_source){
		if(s_color)
			output << "\n \x1b[35m" << fn << "\x1b[0m";
		else
			output << "\n " << fn;
		
		if(s_color)
			output << "\n \x1b[35mfile://" << file << ":" << line << "\x1b[0m";
		else
			output << "\n file://" << file << ":" << line;
	}	
	
	memset(log_line, 0, PQ_ASYNC_MAX_LOG_LENGTH);
	vsnprintf(log_line, PQ_ASYNC_MAX_LOG_LENGTH -1, msg, arg);
	
	if(strnlen(log_line, PQ_ASYNC_MAX_LOG_LENGTH) > 0){
		std::string result = 
			std::regex_replace(
				std::string(log_line), 
				std::regex("\n"),
				std::string("\n  "), 
				std::regex_constants::match_any
				);
		output << "\n" << result;
	}
	
	switch(level){
		case log_level::ll_fatal:
			output << prepare_footer("31;1");
			break;
		case log_level::ll_error:
			output << prepare_footer("31");
			break;
		case log_level::ll_warning:
			output << prepare_footer("33;1");
			break;
		case log_level::ll_info:
			output << prepare_footer("34;1");
			break;
		case log_level::ll_debug:
			output << prepare_footer("36");
			break;
		case log_level::ll_trace:
			output << prepare_footer("37");
			break;
		default:
			output << prepare_footer("37");
			break;
	}
	
	if(level > log_level::ll_error)
		std::cout << output.str() << std::endl;
	else
		std::cerr << output.str() << std::endl;
	
	if(level == log_level::ll_fatal)
		std::abort();
}




} //namespace pq_async