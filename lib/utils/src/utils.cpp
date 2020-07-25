#include "utils/utils.hpp"

// For Debugging
#define ANSI_COLOUR_BLACK "\x1b[1;30m"
#define ANSI_COLOUR_RED "\x1b[1;31m"
#define ANSI_COLOUR_GREEN "\x1b[1;32m"
#define ANSI_COLOUR_YELLOW "\x1b[1;33m"
#define ANSI_COLOUR_BLUE "\x1b[1;34m"
#define ANSI_COLOUR_MAGENTA "\x1b[1;35m"
#define ANSI_COLOUR_CYAN "\x1b[1;36m"
#define ANSI_COLOUR_WHITE "\x1b[1;37m"
#define ANSI_COLOUR_RESET "\x1b[1;0m"

std::mutex display_mutex;

utils_log::LogDetailed::~LogDetailed()
{
	if(_log_level >= _min_level){
		std::lock_guard<std::mutex> lock(display_mutex);
#ifndef SIMPLE
		switch (_log_level){
		case LogLevel::Debug:
			set_display_colour(Colour::GREEN);
			break;
		case LogLevel::Info:
			set_display_colour(Colour::BLUE);
			break;
		case LogLevel::Warn:
			set_display_colour(Colour::YELLOW);
			break;
		case LogLevel::Err:
			set_display_colour(Colour::RED);
			break;
		case LogLevel::Status:
			set_display_colour(Colour::WHITE);
			break;
		}

		if (_log_level == LogLevel::Status){
			std::cout << _s.str();
			set_display_colour(Colour::RESET);
			std::cout << std:: endl;
		}
		else{
			time_t rawtime;
			time(&rawtime);
			struct tm *timeinfo = localtime(&rawtime);
			char time_buffer[25]{};
			strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d | %H:%M:%S ", timeinfo);
			std::cout << "[" << time_buffer;

			switch(_log_level){
			case LogLevel::Debug:
				std::cout << "| Debug--] ";
				break;
			case LogLevel::Info:
				std::cout << "| Info---] ";
				break;
			case LogLevel::Warn:
				std::cout << "| Warn---] ";
				break;
			case LogLevel::Err:
				std::cout << "| Error--] ";
				break;
			case LogLevel::Status:
				std::cout << "| Status-] ";
				break;
			}
			set_display_colour(Colour::RESET);
			std::cout << _s.str();
			set_display_colour(Colour::CYAN);
			std::cout << " |" << _caller_filename << ":" << _caller_filenumber << "|";
			set_display_colour(Colour::RESET);
			std::cout << std::endl;
		}
#else
	std::cout << _s.str() << std::endl;
#endif
	}
}

void utils_log::set_display_colour(utils_log::Colour colour){
	switch (colour){
	case Colour::BLACK:
		std::cout << ANSI_COLOUR_BLACK;
		break;
	case Colour::RED:
		std::cout << ANSI_COLOUR_RED;
		break;
	case Colour::GREEN:
		std::cout << ANSI_COLOUR_GREEN;
		break;
	case Colour::YELLOW:
		std::cout << ANSI_COLOUR_YELLOW;
		break;
	case Colour::BLUE:
		std::cout << ANSI_COLOUR_BLUE;
		break;
	case Colour::MAGENTA:
		std::cout << ANSI_COLOUR_MAGENTA;
		break;
	case Colour::CYAN:
		std::cout << ANSI_COLOUR_CYAN;
		break;
	case Colour::WHITE:
		std::cout << ANSI_COLOUR_WHITE;
		break;
	case Colour::RESET:
		std::cout << ANSI_COLOUR_RESET;
		break;
	}
}

utils_log::LogLevel utils_log::LogDetailed::_min_level = utils_log::LogLevel::Debug;

void utils_log::LogDetailed::setLogLevel(LogLevel level){
	_min_level = level;
}
