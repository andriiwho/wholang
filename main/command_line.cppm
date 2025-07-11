module;

#include <utility>
#include <unordered_map>
#include <string>
#include <stdexcept>

export module wholang.command_line;

import wholang.console;

// Valid options
#define WHO_ONLY_FILE_PATH 1

export struct CommandLineArgs
{
	std::string path_to_file{};
};

struct CommandLineParseContext
{
	int argc;
	const char* const* const argv;
	int argIndex;
	CommandLineArgs* inOutArgs;
};

using CommandLineRouter = void (*)(CommandLineParseContext);

#define DECLARE_ROUTER(name) static void name(CommandLineParseContext ctx);
#define DEFINE_ROUTER(name) void name(CommandLineParseContext ctx)

DECLARE_ROUTER(Router_FilePath);

static std::unordered_map<std::string, CommandLineRouter> GRouters{};

#define WHO_MAKE_PARSE_CONTEXT(argIndex, outPtr) \
	CommandLineParseContext                      \
	{                                            \
		argc, argv, argIndex, outPtr             \
	}
namespace app
{
	export CommandLineArgs parse_command_line(int argc, const char* const* const argv)
	{
		if (argc <= 1)
		{
			console::printerrln("Usage: who <path-to-file>");
			throw std::runtime_error("No file specified");
		}

		// For now, we don't really need to parse other args except file path
		// Possible command options
		// -flag
		// -option="value"

		CommandLineArgs outArgs{};
#if WHO_ONLY_FILE_PATH
		outArgs.path_to_file = argv[1];
#else
		for (int index = 0; index < argc; ++index)
		{
			if (auto iter = GRouters.find(argv[index]); iter != GRouters.end())
			{
				iter->second(WHO_MAKE_PARSE_CONTEXT(index, &outArgs));
			}
		}
#endif
		return outArgs;
	}

#if !WHO_ONLY_FILE_PATH
	DEFINE_ROUTER(Router_FilePath)
	{
	}
#endif
} // namespace app