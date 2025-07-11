#include "command_line.hh"
#include "console_utils.hh"

#include <utility>
#include <unordered_map>

// Valid options
#define WHO_ONLY_FILE_PATH 1

struct COMMAND_LINE_PARSE_CONTEXT
{
	int						 argc;
	const char* const* const argv;
	int						 argIndex;
	COMMAND_LINE_ARGS*		 inOutArgs;
};

using COMMAND_LINE_ROUTER = void (*)(COMMAND_LINE_PARSE_CONTEXT);

#define DECLARE_ROUTER(name) static void name(COMMAND_LINE_PARSE_CONTEXT ctx);
#define DEFINE_ROUTER(name) void name(COMMAND_LINE_PARSE_CONTEXT ctx)

DECLARE_ROUTER(Router_FilePath);

static std::unordered_map<std::string, COMMAND_LINE_ROUTER> GRouters{};

#define WHO_MAKE_PARSE_CONTEXT(argIndex, outPtr) COMMAND_LINE_PARSE_CONTEXT{argc, argv, argIndex, outPtr}
COMMAND_LINE_ARGS App::ParseCommandLine(int argc, const char* const* const argv)
{
	if (argc <= 1)
	{
		Console::PrintErrorLine("Usage: who <path-to-file>");
		throw std::runtime_error("No file specified");
	}

	// For now, we don't really need to parse other args except file path
	// Possible command options
	// -flag
	// -option="value"

	COMMAND_LINE_ARGS outArgs{};
#if WHO_ONLY_FILE_PATH
	outArgs.pathToFile = argv[1];
#else
	for (int index = 0; index < argc; ++index)
	{
		if(auto iter = GRouters.find(argv[index]); iter != GRouters.end())
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
