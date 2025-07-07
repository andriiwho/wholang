#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include "ConsoleUtils.hh"
#include "CommandLine.hh"
#include "CIS/Lexer/Lexer.hh"
#include "CIS/Parser/ParserInitialPass.hh"

static std::string ReadFileToString(std::string_view path)
{
	std::ifstream	  file_stream(path.data());
	std::stringstream sstream;
	sstream << file_stream.rdbuf();
	return sstream.str();
}

int main(const int argc, const char* const* const argv)
{
	// Parse command line
	COMMAND_LINE_ARGS cmd;
	try
	{
		cmd = App::ParseCommandLine(argc, argv);
	}
	catch(const std::exception& exc)
	{
		Console::PrintErrorLine("Failed to parse command line: {}", exc.what());
		return 1;
	}

	// Read the file to string
	std::string fileSource;
	try
	{
		fileSource = ReadFileToString(cmd.pathToFile);
	}
	catch(const std::exception& exc)
	{
		Console::PrintErrorLine("Failed to read file '{}'", exc.what());
		return 1;
	}

	// Tokenize the file
	TOKEN_STREAM tokenStream = LexEvaluateSource(cmd.pathToFile, fileSource);

#ifndef NDEBUG
	LexDumpTokens(tokenStream);
#endif

	AST_NODE::PTR translationUnit = ParseTypeTree(tokenStream);

	return 0;
}