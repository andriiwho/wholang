module;

#include <format>
#include <iostream>
#include <string>

export module wholang.console;

namespace console
{
	export enum class PrintMode
	{
		eNormal,
		eError,
	};

	export void flush(const PrintMode mode)
	{
		if (mode == PrintMode::eNormal)
		{
			std::cout.flush();
		}
		else if (mode == PrintMode::eError)
		{
			std::cerr.flush();
		}
	}

	namespace detail
	{
		inline void print_helper(PrintMode mode, std::string&& message, bool newLine = false)
		{
			std::ostream& stream = [mode]() -> std::ostream& {
				switch (mode)
				{
					case PrintMode::eNormal:
						return std::cout;
					case PrintMode::eError:
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
	} // namespace detail

#define WHO_FORMAT_ARGS() std::format(format, std::forward<Args>(args)...)
#define WHO_ADD_PRINTER(name, err_level, newl)                    \
	export template <typename... Args>                            \
	void name(std::format_string<Args...> format, Args&&... args) \
	{                                                             \
		detail::print_helper(err_level, WHO_FORMAT_ARGS(), newl); \
	}

	WHO_ADD_PRINTER(print, PrintMode::eNormal, false)
	WHO_ADD_PRINTER(println, PrintMode::eNormal, true)
	WHO_ADD_PRINTER(printerr, PrintMode::eError, false)
	WHO_ADD_PRINTER(printerrln, PrintMode::eError, true)
} // namespace console

#undef WHO_ADD_PRINTER
#undef WHO_FORMAT_ARGS