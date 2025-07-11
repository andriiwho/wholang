module;

#include <string_view>
#include <string>
#include <vector>
#include <format>

export module wholang.diag;

import wholang.console;

namespace diag
{
	export enum class ELevel
	{
		eError,
		eWarning,
		eNote,
	};

	export constexpr std::string_view to_string(const ELevel level)
	{
		if (level == ELevel::eError)
		{
			return "error";
		}
		if (level == ELevel::eWarning)
		{
			return "warning";
		}
		return "note";
	}

	struct Diagnostic
	{
		ELevel level;
		std::string_view file;
		int line;
		int column;
		std::string message;
		std::string_view line_text;
	};

	namespace detail
	{
		std::vector<Diagnostic> SDiag;
	} // namespace detail

	export void print_all()
	{

		for (const Diagnostic& d : detail::SDiag)
		{
			std::string message = std::format("{}: {}:{}: {}: {}",
				d.file,
				d.line,
				d.column,
				to_string(d.level),
				d.message);

			if (!d.line_text.empty())
			{
				message += '\n';
				message += d.line_text;
			}

			console::printerrln("{}", message);
		}

		detail::SDiag.clear();

		console::flush(console::PrintMode::eNormal);
		console::flush(console::PrintMode::eError);
	}

	template <typename... Args>
	void add_diag(
		const ELevel level,
		const std::string_view fileName,
		const int line,
		const int column,
		const std::string_view lineText,
		std::format_string<Args...> format,
		Args&&... args)
	{
		Diagnostic d{
			.level = level,
			.file = fileName,
			.line = line,
			.column = column,
			.message = std::format(format, std::forward<Args>(args)...),
			.line_text = lineText,
		};
		detail::SDiag.push_back(std::move(d));

		if (level == ELevel::eError)
		{
			print_all();
		}
	}

	export template <typename... Args>
	void note(
		std::string_view fileName,
		int line,
		int column,
		std::string_view lineText,
		std::format_string<Args...> format,
		Args&&... args)
	{
		add_diag(
			ELevel::eNote,
			fileName,
			line,
			column,
			lineText,
			format,
			std::forward<Args>(args)...);
	}

	export template <typename... Args>
	void warn(
		std::string_view fileName,
		int line,
		int column,
		std::string_view lineText,
		std::format_string<Args...> format,
		Args&&... args)
	{
		add_diag(
			ELevel::eWarning,
			fileName,
			line,
			column,
			lineText,
			format,
			std::forward<Args>(args)...);
	}

	export template <typename... Args>
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
			ELevel::eError,
			fileName,
			line,
			column,
			lineText,
			format,
			std::forward<Args>(args)...);
		throw std::runtime_error("Exception. See errors above.");
	}
} // namespace diag