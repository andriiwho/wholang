#pragma once

#include <optional>
#include <string>

struct COMMAND_LINE_ARGS
{
	std::string pathToFile{};
};

namespace App
{
	[[nodiscard]] COMMAND_LINE_ARGS ParseCommandLine(int argc, const char* const* const argv);
} // namespace App
