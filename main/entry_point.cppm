module;

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

export module who.entry_point;

import wholang.lexer.forward;
import wholang.lexer;
import wholang.command_line;
import wholang.console;
import wholang.parser;

static std::string read_file_to_string(std::string_view path)
{
	std::ifstream file_stream(path.data());
	std::stringstream sstream;
	sstream << file_stream.rdbuf();
	return sstream.str();
}

export int main(const int argc, const char* const* const argv)
{
	// Parse command line
	CommandLineArgs cmd;
	try
	{
		cmd = app::parse_command_line(argc, argv);
	}
	catch (const std::exception& exc)
	{
		console::printerrln("Failed to parse command line: {}", exc.what());
		return 1;
	}

	// Read the file to string
	std::string fileSource;
	try
	{
		fileSource = read_file_to_string(cmd.path_to_file);
	}
	catch (const std::exception& exc)
	{
		console::printerrln("Failed to read file '{}'", exc.what());
		return 1;
	}

	// Tokenize the file
	const TokenStream token_stream = lex_evaluate_source(cmd.path_to_file, fileSource);

#if !defined NDEBUG && 0
	lex_dump_tokens(token_stream);
#endif

	try
	{
		ast::Node::Ptr translationUnit = parse_type_tree(token_stream);
	}
	catch (const std::exception& exc)
	{
		console::printerrln("Parsing failed: {}", exc.what());
		return 1;
	}

	return 0;
}