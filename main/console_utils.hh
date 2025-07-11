#pragma once

#include <format>
#include <iostream>
#include <string>

namespace Console
{
	enum class PRINT_MODE
	{
		NORMAL,
		ERROR,
	};

	inline void Flush(PRINT_MODE mode)
	{
		if (mode == PRINT_MODE::NORMAL)
		{
			std::cout.flush();
		}
		else if(mode == PRINT_MODE::ERROR)
		{
			std::cerr.flush();
		}
	}

	namespace Detail
	{
		inline void PrintHelper(PRINT_MODE mode, std::string&& message, bool newLine = false)
		{
			std::ostream& stream = [mode]() -> std::ostream& {
				switch (mode)
				{
					case PRINT_MODE::NORMAL:
						return std::cout;
					case PRINT_MODE::ERROR:
						return std::cerr;
				}

				throw std::runtime_error("Unknown print mode");
			}();

			stream << message;
			if (newLine)
			{
				stream << '\n';
			}
		}
	} // namespace Detail

#define WHO_FORMAT_ARGS() std::format(format, std::forward<Args>(args)...)
#define WHO_ADD_PRINTER(name, err_level, newl)                           \
	template <typename... Args>                                          \
	inline void name(std::format_string<Args...> format, Args&&... args) \
	{                                                                    \
		Detail::PrintHelper(err_level, WHO_FORMAT_ARGS(), newl);         \
	}

	WHO_ADD_PRINTER(Print, PRINT_MODE::NORMAL, false)
	WHO_ADD_PRINTER(PrintLine, PRINT_MODE::NORMAL, true)
	WHO_ADD_PRINTER(PrintError, PRINT_MODE::ERROR, false)
	WHO_ADD_PRINTER(PrintErrorLine, PRINT_MODE::ERROR, true)
} // namespace Console

#undef WHO_ADD_PRINTER
#undef WHO_FORMAT_ARGS