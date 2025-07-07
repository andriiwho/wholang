#include "Lexer.hh"
#include "ConsoleUtils.hh"
#include "CIS/Debug/Diagnostics.hh"

#include <stdexcept>

struct LEXER_CONTEXT
{
	std::string fileContents;
	TOKEN_STREAM stream;
	const char** cursor{};
	int line{1};
	int column{};

	std::string filePath{};
	std::string_view lineText{};
};
static thread_local LEXER_CONTEXT TSCtx;

[[nodiscard]] static TOKEN NextToken();
[[nodiscard]] static char Peek();
[[nodiscard]] static char PeekNext();
static void Advance();
static void SkipWhitespaceAndComments();
[[nodiscard]] static bool IsIdentifierStart(char c);
[[nodiscard]] static bool IsIdentifierChar(char c);

TOKEN_STREAM LexEvaluateSource(const std::string& filePath, const std::string& fileContents)
{
	TSCtx = {};
	TSCtx.filePath = filePath;
	TSCtx.stream.filePath = filePath;

	const char* cursor = fileContents.data();
	TSCtx.cursor = &cursor;
	{
		TOKEN token = NextToken();
		while (token.type != TOKEN_TYPE::END_OF_FILE)
		{
			TSCtx.stream.tokens.push_back(token);
			token = NextToken();
		}

		if (token.type == TOKEN_TYPE::END_OF_FILE)
		{
			TSCtx.stream.tokens.push_back(token);
		}
	}
	TSCtx.cursor = nullptr;
	return TSCtx.stream;
}

void LexDumpTokens(const TOKEN_STREAM& inStream)
{
	for (const auto& token : inStream.tokens)
	{
		switch (token.type)
		{
			case TOKEN_TYPE::IDENTIFIER:
			case TOKEN_TYPE::STRING_LITERAL:
			case TOKEN_TYPE::INTEGRAL_LITERAL:
			case TOKEN_TYPE::FLOAT_LITERAL:
				Console::PrintLine("{} ('{}')", TOKEN_TYPE::ToString(token.type), *token.lexeme);
				break;
			default:
				Console::PrintLine("{}", TOKEN_TYPE::ToString(token.type));
				break;
		}
		Console::Flush(Console::PRINT_MODE::NORMAL);
	}
}

TOKEN ParseStringLikeLiterals(char literalStart, TOKEN_TYPE::TYPE type)
{
	const char* start = *TSCtx.cursor;
	Advance();
	while (Peek() && Peek() != literalStart)
	{
		Advance();
	}

	if (Peek() == literalStart)
	{
		Advance();
	}

	const std::string_view lexeme = std::string_view(start, *TSCtx.cursor - start);
	return TOKEN{
		.type = type,
		.lexeme = std::string(lexeme),
		.line = TSCtx.line,
		.column = TSCtx.column,
	};
}

TOKEN NextToken()
{
	SkipWhitespaceAndComments();

	if (Peek() == '\0')
	{
		return TOKEN{
			TOKEN_TYPE::END_OF_FILE,
			std::nullopt,
			TSCtx.line,
			TSCtx.column
		};
	}

	const char c = Peek();

	// Parse identifier
	if (IsIdentifierStart(c))
	{
		const char* start = *TSCtx.cursor;
		while (IsIdentifierChar(Peek()))
		{
			Advance();
		}

		// Parse keywords
		const std::string_view lexeme = std::string_view(start, *TSCtx.cursor - start);
		const TOKEN_TYPE::TYPE tokenType = TOKEN_TYPE::FromString(lexeme);
		if (tokenType != TOKEN_TYPE::INVALID_TOKEN)
		{
			return TOKEN{
				.type = tokenType,
				.lexeme = std::string(lexeme),
				.line = TSCtx.line,
				.column = TSCtx.column
			};
		}

		return TOKEN{
			.type = TOKEN_TYPE::IDENTIFIER,
			.lexeme = std::string(lexeme),
			.line = TSCtx.line,
			.column = TSCtx.column
		};
	}

	// Try parse compound symbols
	{
		const char compound[] = { Peek(), PeekNext(), '\0' };
		if (std::string_view(compound).length() >= 2)
		{
			const TOKEN_TYPE::TYPE tokenType = TOKEN_TYPE::FromString(std::string_view(compound));
			if (tokenType != TOKEN_TYPE::INVALID_TOKEN && std::string_view(compound).length() >= 2)
			{
				Advance();
				Advance();
				return TOKEN{
					.type = tokenType,
					.lexeme = std::nullopt,
					.line = TSCtx.line,
					.column = TSCtx.column
				};
			}
		}
	}

	// Try parse numeric literals
	{
		if (isdigit(Peek()))
		{
			bool isFloat = false;
			const char* start = *TSCtx.cursor;

			while (isdigit(Peek()))
			{
				Advance();
			}

			if (Peek() == '.' && isdigit(PeekNext()))
			{
				isFloat = true;
				Advance();
				while (isdigit(Peek()))
				{
					Advance();
				}
			}

			if (Peek() == 'e' || Peek() == 'E')
			{
				isFloat = true;
				Advance();
				if (Peek() == '+' || Peek() == '-')
				{
					Advance();
				}

				while (isdigit(Peek()))
				{
					Advance();
				}

				if (Peek() == 'f')
				{
					isFloat = true;
					Advance();
				}
			}

			const std::string_view lexeme = std::string_view(start, *TSCtx.cursor - start);
			return TOKEN{
				.type = isFloat ? TOKEN_TYPE::FLOAT_LITERAL : TOKEN_TYPE::INTEGRAL_LITERAL,
				.lexeme = std::string(lexeme),
				.line = TSCtx.line,
				.column = TSCtx.column,
			};
		}
	}

	// Parse single symbols
	{
		char cursorSymBuffer[2] = { c, '\0' };
		const TOKEN_TYPE::TYPE tokenType = TOKEN_TYPE::FromString(std::string_view(cursorSymBuffer));
		if (tokenType != TOKEN_TYPE::INVALID_TOKEN)
		{
			Advance();
			return TOKEN{
				.type = tokenType,
				.lexeme = std::nullopt,
				.line = TSCtx.line,
				.column = TSCtx.column
			};
		}
	}

	// Parse string literals
	if (c == '"')
	{
		return ParseStringLikeLiterals('"', TOKEN_TYPE::STRING_LITERAL);
	}

	if (c == '\'')
	{
		return ParseStringLikeLiterals('\'', TOKEN_TYPE::CHAR_LITERAL);
	}

	DIAGNOSTICS::AddError(
		TSCtx.filePath,
		TSCtx.line,
		TSCtx.column,
		TSCtx.lineText,
		"Undefined token '{}'",
		c);
}

void Advance()
{
	if (Peek() == '\n')
	{
		TSCtx.line++;
		TSCtx.column = 0;
	}

	(*TSCtx.cursor)++;
	TSCtx.column++;
}

char Peek()
{
	return **TSCtx.cursor;
}

char PeekNext()
{
	if (Peek() != '\0')
		return (*TSCtx.cursor)[1];

	return 0;
}

bool IsIdentifierStart(char c)
{
	return isalpha(c) || c == '_';
}

void SkipWhitespaceAndComments()
{
	const auto skipSpaces = [] {
		while (isspace(Peek()))
		{
			Advance();
		}
	};
	skipSpaces();

	while (true)
	{
		if (Peek() == '/')
		{
			if (const char next = PeekNext())
			{
				// Check for single line comments
				if (next == '/')
				{
					Advance();
					Advance();

					while (Peek() != '\n' && Peek() != '\0')
					{
						Advance();
					}
				}

				// Multi line comments
				if (next == '*')
				{
					Advance();
					Advance();
					while (true)
					{
						if (Peek() == '\0')
						{
							break;
						}

						if (Peek() == '*' && PeekNext() == '/')
						{
							Advance();
							Advance();
							break;
						}

						Advance();
					}
					continue;
				}
			}
		}
		break;
	}

	// And also at the end
	skipSpaces();

	if (Peek() == '/' && (PeekNext() == '/' || PeekNext() == '*'))
	{
		SkipWhitespaceAndComments();
	}
}

bool IsIdentifierChar(char c)
{
	return isalnum(c) || c == '_';
}
