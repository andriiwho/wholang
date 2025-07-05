#pragma once

#include <vector>
#include <string>
#include <optional>
#include <string_view>
#include <unordered_map>

#define WHO_BEGIN_TOKENS                       \
	namespace TOKEN_TYPE::Detail               \
	{                                          \
		inline constexpr int START = __LINE__; \
	}

#define WHO_END_TOKENS

#define WHO_END_TOKENS
#define WHO_TOKEN_VALUE(name, value)                                                     \
	namespace TOKEN_TYPE                                                                 \
	{                                                                                    \
		inline constexpr TYPE name = __LINE__ - Detail::START;                           \
		namespace Detail                                                                 \
		{                                                                                \
			inline auto name##_initializer = TOKEN_TYPE_STATIC_INITIALIZER(name, value); \
		}                                                                                \
	}

namespace TOKEN_TYPE
{
	using TYPE = int;
	inline constexpr int INVALID_TOKEN = -1;
	namespace Detail
	{
		inline std::unordered_map<TYPE, std::string_view> TYPE_TO_SV_TABLE;
		inline std::unordered_map<std::string_view, TYPE> SV_TO_TYPE_TABLE;

		struct TOKEN_TYPE_STATIC_INITIALIZER
		{
			TOKEN_TYPE_STATIC_INITIALIZER(TYPE index, std::string_view value)
			{
				TYPE_TO_SV_TABLE[index] = value;
				SV_TO_TYPE_TABLE[value] = index;
			}
		};
	} // namespace Detail

	inline std::string_view ToString(TYPE type)
	{
		if (const auto iter = Detail::TYPE_TO_SV_TABLE.find(type); iter != Detail::TYPE_TO_SV_TABLE.end())
		{
			return iter->second;
		}

		return "UNKNOWN";
	}

	inline TYPE FromString(std::string_view value)
	{
		if (const auto iter = Detail::SV_TO_TYPE_TABLE.find(value); iter != Detail::SV_TO_TYPE_TABLE.end())
		{
			return iter->second;
		}

		return -1;
	}
} // namespace TOKEN_TYPE

WHO_BEGIN_TOKENS
WHO_TOKEN_VALUE(UNKNOWN, "<unknown>")
WHO_TOKEN_VALUE(END_OF_FILE, "<eof>")
WHO_TOKEN_VALUE(IDENTIFIER, "<identifier>");

// Keywords
WHO_TOKEN_VALUE(KWD_PACKAGE, "package")
WHO_TOKEN_VALUE(KWD_IMPORT, "import")
WHO_TOKEN_VALUE(KWD_EXPORT, "export")
WHO_TOKEN_VALUE(KWD_INLINE, "inline")
WHO_TOKEN_VALUE(KWD_FORCEINLINE, "forceinline")
WHO_TOKEN_VALUE(KWD_COMPILETIME, "compiletime")
WHO_TOKEN_VALUE(KWD_FUNC, "func")
WHO_TOKEN_VALUE(KWD_VAR, "var")
WHO_TOKEN_VALUE(KWD_CONST, "const")
WHO_TOKEN_VALUE(KWD_TYPE, "type")
WHO_TOKEN_VALUE(KWD_SELF, "self")
WHO_TOKEN_VALUE(KWD_DEFER, "defer")
WHO_TOKEN_VALUE(KWD_ASYNC, "async")
WHO_TOKEN_VALUE(KWD_AWAIT, "await")
WHO_TOKEN_VALUE(KWD_RETURN, "return")

// Symbols
WHO_TOKEN_VALUE(LPAREN, "(")
WHO_TOKEN_VALUE(RPAREN, ")")
WHO_TOKEN_VALUE(LBRACE, "{")
WHO_TOKEN_VALUE(RBRACE, "}")
WHO_TOKEN_VALUE(LBRACKET, "[")
WHO_TOKEN_VALUE(RBRACKET, "]")
WHO_TOKEN_VALUE(LANGULAR, "<")
WHO_TOKEN_VALUE(RANGULAR, ">")
WHO_TOKEN_VALUE(SEMICOLON, ";")
WHO_TOKEN_VALUE(COLON, ":")
WHO_TOKEN_VALUE(DOT, ".")
WHO_TOKEN_VALUE(COMMA, ",")
WHO_TOKEN_VALUE(AMPERSAND, "&")
WHO_TOKEN_VALUE(STAR, "*")
WHO_TOKEN_VALUE(PLUS, "+")
WHO_TOKEN_VALUE(MINUS, "-")
WHO_TOKEN_VALUE(DIVIDE, "/")
WHO_TOKEN_VALUE(PERCENT, "%")
WHO_TOKEN_VALUE(NEGATE, "!")
WHO_TOKEN_VALUE(BITOR, "|")
WHO_TOKEN_VALUE(BACKSLASH, "\\")
WHO_TOKEN_VALUE(EQUALS, "=")
WHO_TOKEN_VALUE(XOR, "^")
WHO_TOKEN_VALUE(BITNEG, "~")

// Literals
WHO_TOKEN_VALUE(STRING_LITERAL, "<string literal>")
WHO_TOKEN_VALUE(CHAR_LITERAL, "<char literal>")
WHO_TOKEN_VALUE(INTEGRAL_LITERAL, "<int literal>")
WHO_TOKEN_VALUE(FLOAT_LITERAL, "<float literal>")

// Compound tokens
WHO_TOKEN_VALUE(ARROW, "->")
WHO_TOKEN_VALUE(ELVIS, "?:")
WHO_TOKEN_VALUE(NEQUALS, "!=")
WHO_TOKEN_VALUE(PLUS_EQUALS, "+=")
WHO_TOKEN_VALUE(MINUS_EQUALS, "-=")
WHO_TOKEN_VALUE(DIVIDE_EQUALS, "/=")
WHO_TOKEN_VALUE(MUL_EQUALS, "*=")
WHO_TOKEN_VALUE(BITOR_EQUALS, "|=")
WHO_TOKEN_VALUE(BITAND_EQUALS, "&=")
WHO_TOKEN_VALUE(BITXOR_EQUALS, "^=")
WHO_TOKEN_VALUE(BITNEG_EQUALS, "~=")
WHO_TOKEN_VALUE(BITSHIFT_LEFT, "<<")
WHO_TOKEN_VALUE(BITSHIFT_RIGHT, ">>")
WHO_TOKEN_VALUE(LEQUAL, "<=")
WHO_TOKEN_VALUE(GEQUAL, ">=")
WHO_TOKEN_VALUE(BOOL_EQUAL, "==")

WHO_END_TOKENS

#undef WHO_TOKEN_VALUE
#undef WHO_BEGIN_TOKENS
#undef WHO_END_TOKENS

struct TOKEN
{
	TOKEN_TYPE::TYPE type{};
	std::optional<std::string> lexeme{};
	int line{};
	int column{};
};

struct TOKEN_STREAM
{
	std::vector<TOKEN> tokens;
};
