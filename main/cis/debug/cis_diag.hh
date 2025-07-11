#pragma once

#include "console_utils.hh"
#include "cis/lexer/cis_lexer_forward.hh"

#include <format>
#include <vector>
#include <string>
#include <string_view>

namespace DIAGNOSTICS
{
	enum class LEVEL
	{
		ERROR,
		WARNING,
		NOTE,
	};

	inline constexpr std::string_view to_string(LEVEL level)
	{
		if (level == LEVEL::ERROR)
		{
			return "error";
		}
		else if (level == LEVEL::WARNING)
		{
			return "warning";
		}
		else
			return "note";
	}

	struct DIAGNOSTIC
	{
		LEVEL level;
		std::string_view file;
		int line;
		int column;
		std::string message;
		std::string_view line_text;
	};

	namespace Detail
	{
		extern std::vector<DIAGNOSTIC> SDiag;
	} // namespace Detail

	void print_all();

	template <typename... Args>
	void add_diag(
		LEVEL level,
		std::string_view fileName,
		int line,
		int column,
		std::string_view lineText,
		std::format_string<Args...> format,
		Args&&... args)
	{
		DIAGNOSTIC d{
			.level = level,
			.file = fileName,
			.line = line,
			.column = column,
			.message = std::format(format, std::forward<Args>(args)...),
			.line_text = lineText,
		};
		Detail::SDiag.push_back(std::move(d));

		if (level == LEVEL::ERROR)
		{
			print_all();
		}
	}

	template <typename... Args>
	void note(
		std::string_view fileName,
		int line,
		int column,
		std::string_view lineText,
		std::format_string<Args...> format,
		Args&&... args)
	{
		add_diag(
			LEVEL::NOTE,
			fileName,
			line,
			column,
			lineText,
			format,
			std::forward<Args>(args)...);
	}

	template <typename... Args>
	void warn(
		std::string_view fileName,
		int line,
		int column,
		std::string_view lineText,
		std::format_string<Args...> format,
		Args&&... args)
	{
		add_diag(
			LEVEL::WARNING,
			fileName,
			line,
			column,
			lineText,
			format,
			std::forward<Args>(args)...);
	}


	template <typename... Args>
	[[noreturn]]
	void error(
		std::string_view fileName,
		int line,
		int column,
		std::string_view lineText,
		std::format_string<Args...> format,
		Args&&... args)
	{
		add_diag(
			LEVEL::ERROR,
			fileName,
			line,
			column,
			lineText,
			format,
			std::forward<Args>(args)...);
		throw std::runtime_error("Exception. See errors above.");
	}

} // namespace DIAGNOSTICS