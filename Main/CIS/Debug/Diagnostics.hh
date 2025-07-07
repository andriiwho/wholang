#pragma once

#include "ConsoleUtils.hh"
#include "CIS/Lexer/LexFwd.hh"

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

	inline constexpr std::string_view ToString(LEVEL level)
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
		std::string_view lineText;
	};

	namespace Detail
	{
		extern std::vector<DIAGNOSTIC> SDiag;
	} // namespace Detail

	void PrintAll();

	template <typename... Args>
	void AddDiagnostic(
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
			.lineText = lineText,
		};
		Detail::SDiag.push_back(std::move(d));

		if (level == LEVEL::ERROR)
		{
			PrintAll();
		}
	}

	template <typename... Args>
	void AddNote(
		std::string_view fileName,
		int line,
		int column,
		std::string_view lineText,
		std::format_string<Args...> format,
		Args&&... args)
	{
		AddDiagnostic(
			LEVEL::NOTE,
			fileName,
			line,
			column,
			lineText,
			format,
			std::forward<Args>(args)...);
	}

	template <typename... Args>
	void AddWarning(
		std::string_view fileName,
		int line,
		int column,
		std::string_view lineText,
		std::format_string<Args...> format,
		Args&&... args)
	{
		AddDiagnostic(
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
	void AddError(
		std::string_view fileName,
		int line,
		int column,
		std::string_view lineText,
		std::format_string<Args...> format,
		Args&&... args)
	{
		AddDiagnostic(
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