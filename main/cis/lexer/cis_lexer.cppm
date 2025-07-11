module;

#include <string>
#include <optional>

export module wholang.lexer;

import wholang.console;
import wholang.lexer.forward;
import wholang.diag;

struct LexerContext
{
	std::string file_contents;
	TokenStream stream;
	const char** cursor{};
	int line{1};
	int column{};

	std::string file_path{};
	std::string_view line_text{};
};
static thread_local LexerContext ts_ctx;

[[nodiscard]] static Token next_token();
[[nodiscard]] static char peek();
[[nodiscard]] static char peek_next();
static void advance();
static void skip_whitespaces_and_comments();
[[nodiscard]] static bool is_identifier_start(char c);
[[nodiscard]] static bool is_identifier_char(char c);

export TokenStream lex_evaluate_source(const std::string& file_path, const std::string& file_contents)
{
	ts_ctx = {};
	ts_ctx.file_path = file_path;
	ts_ctx.stream.file_path = file_path;

	const char* cursor = file_contents.data();
	ts_ctx.cursor = &cursor;
	{
		Token token = next_token();
		while (token.type != TokenType::eEOF)
		{
			ts_ctx.stream.tokens.push_back(token);
			token = next_token();
		}

		if (token.type == TokenType::eEOF)
		{
			ts_ctx.stream.tokens.push_back(token);
		}
	}
	ts_ctx.cursor = nullptr;
	return ts_ctx.stream;
}

export void lex_dump_tokens(const TokenStream& in_stream)
{
	for (const auto& token : in_stream.tokens)
	{
		switch (token.type)
		{
			case TokenType::eIdentifier:
			case TokenType::eStringLiteral:
			case TokenType::eIntLiteral:
			case TokenType::eFloatLiteral:
				console::println("{} ('{}')", TokenType::to_string(token.type), *token.lexeme);
				break;
			default:
				console::println("{}", TokenType::to_string(token.type));
				break;
		}
		console::flush(console::PrintMode::eNormal);
	}
}

Token read_stringlike_literal(char literal_start, TokenType::Type type)
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

	const auto lexeme = std::string_view(start, *ts_ctx.cursor - start);
	return Token{
		.type = type,
		.lexeme = std::string(lexeme),
		.line = ts_ctx.line,
		.column = ts_ctx.column,
	};
}

Token next_token()
{
	skip_whitespaces_and_comments();

	if (peek() == '\0')
	{
		return Token{
			TokenType::eEOF,
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
		const TokenType::Type tokenType = TokenType::from_string(lexeme);
		if (tokenType != TokenType::INVALID_TOKEN)
		{
			return Token{
				.type = tokenType,
				.lexeme = std::string(lexeme),
				.line = ts_ctx.line,
				.column = ts_ctx.column
			};
		}

		return Token{
			.type = TokenType::eIdentifier,
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
			const TokenType::Type TokenType = TokenType::from_string(std::string_view(compound));
			if (TokenType != TokenType::INVALID_TOKEN && std::string_view(compound).length() >= 2)
			{
				advance();
				advance();
				return Token{
					.type = TokenType,
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
			return Token{
				.type = isFloat ? TokenType::eFloatLiteral : TokenType::eIntLiteral,
				.lexeme = std::string(lexeme),
				.line = ts_ctx.line,
				.column = ts_ctx.column,
			};
		}
	}

	// Parse single symbols
	{
		char cursor_symbol_buffer[2] = { c, '\0' };
		const TokenType::Type tokenType = TokenType::from_string(std::string_view(cursor_symbol_buffer));
		if (tokenType != TokenType::INVALID_TOKEN)
		{
			advance();
			return Token{
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
		return read_stringlike_literal('"', TokenType::eStringLiteral);
	}

	if (c == '\'')
	{
		return read_stringlike_literal('\'', TokenType::eCharLiteral);
	}

	diag::error(
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
