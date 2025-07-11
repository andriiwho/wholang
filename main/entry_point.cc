#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include "console_utils.hh"
#include "command_line.hh"
#include "CIS/Lexer/cis_lexer.hh"
#include "CIS/Parser/cis_parser_pass_initial.hh"

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
	TOKEN_STREAM tokenStream = lex_evaluate_source(cmd.pathToFile, fileSource);

#ifndef NDEBUG
	lex_dump_tokens(tokenStream);
#endif

	AST_NODE::PTR translationUnit = parse_type_tree(tokenStream);

	return 0;
}