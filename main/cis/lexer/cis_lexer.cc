#include "cis_lexer.hh"
#include "console_utils.hh"
#include "cis/debug/cis_diag.hh"

#include <stdexcept>

struct LEXER_CONTEXT
{
	std::string file_contents;
	TOKEN_STREAM stream;
	const char** cursor{};
	int line{1};
	int column{};

	std::string file_path{};
	std::string_view line_text{};
};
static thread_local LEXER_CONTEXT ts_ctx;

[[nodiscard]] static TOKEN next_token();
[[nodiscard]] static char peek();
[[nodiscard]] static char peek_next();
static void advance();
static void skip_whitespaces_and_comments();
[[nodiscard]] static bool is_identifier_start(char c);
[[nodiscard]] static bool is_identifier_char(char c);

TOKEN_STREAM lex_evaluate_source(const std::string& file_path, const std::string& file_contents)
{
	ts_ctx = {};
	ts_ctx.file_path = file_path;
	ts_ctx.stream.file_path = file_path;

	const char* cursor = file_contents.data();
	ts_ctx.cursor = &cursor;
	{
		TOKEN token = next_token();
		while (token.type != TOKEN_TYPE::END_OF_FILE)
		{
			ts_ctx.stream.tokens.push_back(token);
			token = next_token();
		}

		if (token.type == TOKEN_TYPE::END_OF_FILE)
		{
			ts_ctx.stream.tokens.push_back(token);
		}
	}
	ts_ctx.cursor = nullptr;
	return ts_ctx.stream;
}

void lex_dump_tokens(const TOKEN_STREAM& in_stream)
{
	for (const auto& token : in_stream.tokens)
	{
		switch (token.type)
		{
			case TOKEN_TYPE::IDENTIFIER:
			case TOKEN_TYPE::STRING_LITERAL:
			case TOKEN_TYPE::INTEGRAL_LITERAL:
			case TOKEN_TYPE::FLOAT_LITERAL:
				Console::PrintLine("{} ('{}')", TOKEN_TYPE::to_string(token.type), *token.lexeme);
				break;
			default:
				Console::PrintLine("{}", TOKEN_TYPE::to_string(token.type));
				break;
		}
		Console::Flush(Console::PRINT_MODE::NORMAL);
	}
}

TOKEN read_stringlike_literal(char literal_start, TOKEN_TYPE::TYPE type)
{
	const char* start = *ts_ctx.cursor;
	advance();
	while (peek() && peek() != literal_start)
	{
		advance();
	}

	if (peek() == literal_start)
	{
		advance();
	}

	const std::string_view lexeme = std::string_view(start, *ts_ctx.cursor - start);
	return TOKEN{
		.type = type,
		.lexeme = std::string(lexeme),
		.line = ts_ctx.line,
		.column = ts_ctx.column,
	};
}

TOKEN next_token()
{
	skip_whitespaces_and_comments();

	if (peek() == '\0')
	{
		return TOKEN{
			TOKEN_TYPE::END_OF_FILE,
			std::nullopt,
			ts_ctx.line,
			ts_ctx.column
		};
	}

	const char c = peek();

	// Parse identifier
	if (is_identifier_start(c))
	{
		const char* start = *ts_ctx.cursor;
		while (is_identifier_char(peek()))
		{
			advance();
		}

		// Parse keywords
		const std::string_view lexeme = std::string_view(start, *ts_ctx.cursor - start);
		const TOKEN_TYPE::TYPE tokenType = TOKEN_TYPE::FromString(lexeme);
		if (tokenType != TOKEN_TYPE::INVALID_TOKEN)
		{
			return TOKEN{
				.type = tokenType,
				.lexeme = std::string(lexeme),
				.line = ts_ctx.line,
				.column = ts_ctx.column
			};
		}

		return TOKEN{
			.type = TOKEN_TYPE::IDENTIFIER,
			.lexeme = std::string(lexeme),
			.line = ts_ctx.line,
			.column = ts_ctx.column
		};
	}

	// Try parse compound symbols
	{
		const char compound[] = { peek(), peek_next(), '\0' };
		if (std::string_view(compound).length() >= 2)
		{
			const TOKEN_TYPE::TYPE token_type = TOKEN_TYPE::FromString(std::string_view(compound));
			if (token_type != TOKEN_TYPE::INVALID_TOKEN && std::string_view(compound).length() >= 2)
			{
				advance();
				advance();
				return TOKEN{
					.type = token_type,
					.lexeme = std::nullopt,
					.line = ts_ctx.line,
					.column = ts_ctx.column
				};
			}
		}
	}

	// Try parse numeric literals
	{
		if (isdigit(peek()))
		{
			bool isFloat = false;
			const char* start = *ts_ctx.cursor;

			while (isdigit(peek()))
			{
				advance();
			}

			if (peek() == '.' && isdigit(peek_next()))
			{
				isFloat = true;
				advance();
				while (isdigit(peek()))
				{
					advance();
				}
			}

			if (peek() == 'e' || peek() == 'E')
			{
				isFloat = true;
				advance();
				if (peek() == '+' || peek() == '-')
				{
					advance();
				}

				while (isdigit(peek()))
				{
					advance();
				}

				if (peek() == 'f')
				{
					isFloat = true;
					advance();
				}
			}

			const std::string_view lexeme = std::string_view(start, *ts_ctx.cursor - start);
			return TOKEN{
				.type = isFloat ? TOKEN_TYPE::FLOAT_LITERAL : TOKEN_TYPE::INTEGRAL_LITERAL,
				.lexeme = std::string(lexeme),
				.line = ts_ctx.line,
				.column = ts_ctx.column,
			};
		}
	}

	// Parse single symbols
	{
		char cursor_symbol_buffer[2] = { c, '\0' };
		const TOKEN_TYPE::TYPE tokenType = TOKEN_TYPE::FromString(std::string_view(cursor_symbol_buffer));
		if (tokenType != TOKEN_TYPE::INVALID_TOKEN)
		{
			advance();
			return TOKEN{
				.type = tokenType,
				.lexeme = std::nullopt,
				.line = ts_ctx.line,
				.column = ts_ctx.column
			};
		}
	}

	// Parse string literals
	if (c == '"')
	{
		return read_stringlike_literal('"', TOKEN_TYPE::STRING_LITERAL);
	}

	if (c == '\'')
	{
		return read_stringlike_literal('\'', TOKEN_TYPE::CHAR_LITERAL);
	}

	DIAGNOSTICS::error(
		ts_ctx.file_path,
		ts_ctx.line,
		ts_ctx.column,
		ts_ctx.line_text,
		"Undefined token '{}'",
		c);
}

void advance()
{
	if (peek() == '\n')
	{
		ts_ctx.line++;
		ts_ctx.column = 0;
	}

	(*ts_ctx.cursor)++;
	ts_ctx.column++;
}

char peek()
{
	return **ts_ctx.cursor;
}

char peek_next()
{
	if (peek() != '\0')
		return (*ts_ctx.cursor)[1];

	return 0;
}

bool is_identifier_start(char c)
{
	return isalpha(c) || c == '_';
}

void skip_whitespaces_and_comments()
{
	const auto skipSpaces = [] {
		while (isspace(peek()))
		{
			advance();
		}
	};
	skipSpaces();

	while (true)
	{
		if (peek() == '/')
		{
			if (const char next = peek_next())
			{
				// check for single line comments
				if (next == '/')
				{
					advance();
					advance();

					while (peek() != '\n' && peek() != '\0')
					{
						advance();
					}
				}

				// Multi line comments
				if (next == '*')
				{
					advance();
					advance();
					while (true)
					{
						if (peek() == '\0')
						{
							break;
						}

						if (peek() == '*' && peek_next() == '/')
						{
							advance();
							advance();
							break;
						}

						advance();
					}
					continue;
				}
			}
		}
		break;
	}

	// And also at the end
	skipSpaces();

	if (peek() == '/' && (peek_next() == '/' || peek_next() == '*'))
	{
		skip_whitespaces_and_comments();
	}
}

bool is_identifier_char(char c)
{
	return isalnum(c) || c == '_';
}
