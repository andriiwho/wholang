module;

#include <vector>
#include <string>
#include <optional>
#include <string_view>
#include <unordered_map>

export module wholang.lexer.forward;

#define WHO_BEGIN_TOKENS                       \
	namespace TokenType::Detail               \
	{                                          \
		inline constexpr int START = __LINE__; \
	}

#define WHO_END_TOKENS

#define WHO_END_TOKENS
#define WHO_TOKEN_VALUE(name, value)                                              \
	namespace TokenType                                                          \
	{                                                                             \
		export constexpr Type name = __LINE__ - Detail::START;                    \
		namespace Detail                                                          \
		{                                                                         \
			auto name##_initializer = TokenTypeStaticInit(name, value); \
		}                                                                         \
	}

namespace TokenType
{
	export using Type = int;
	export constexpr int INVALID_TOKEN = -1;
	namespace Detail
	{
		std::unordered_map<Type, std::string_view> TYPE_TO_SV_TABLE;
		std::unordered_map<std::string_view, Type> SV_TO_TYPE_TABLE;

		struct TokenTypeStaticInit
		{
			TokenTypeStaticInit(Type index, std::string_view value)
			{
				TYPE_TO_SV_TABLE[index] = value;
				SV_TO_TYPE_TABLE[value] = index;
			}
		};
	} // namespace Detail

	export std::string_view to_string(const Type type)
	{
		if (const auto iter = Detail::TYPE_TO_SV_TABLE.find(type); iter != Detail::TYPE_TO_SV_TABLE.end())
		{
			return iter->second;
		}

		return "UNKNOWN";
	}

	export Type from_string(const std::string_view value)
	{
		if (const auto iter = Detail::SV_TO_TYPE_TABLE.find(value); iter != Detail::SV_TO_TYPE_TABLE.end())
		{
			return iter->second;
		}

		return -1;
	}
} // namespace TokenType

WHO_BEGIN_TOKENS
WHO_TOKEN_VALUE(eUnknown, "<unknown>")
WHO_TOKEN_VALUE(eEOF, "<eof>")
WHO_TOKEN_VALUE(eIdentifier, "<identifier>");

// Keywords
WHO_TOKEN_VALUE(eKwdPackage, "package")
WHO_TOKEN_VALUE(eKwdImport, "import")
WHO_TOKEN_VALUE(eKwdExport, "export")
WHO_TOKEN_VALUE(eKwdInline, "inline")
WHO_TOKEN_VALUE(eKwdFunc, "func")
WHO_TOKEN_VALUE(eKwdVar, "var")
WHO_TOKEN_VALUE(eKwdConst, "const")
WHO_TOKEN_VALUE(eKwdType, "type")
WHO_TOKEN_VALUE(eKwdSelf, "self")
WHO_TOKEN_VALUE(eKwdDefer, "defer")
WHO_TOKEN_VALUE(eKwdReturn, "return")
WHO_TOKEN_VALUE(eKwdIf, "if")
WHO_TOKEN_VALUE(eKwdElse, "else")
WHO_TOKEN_VALUE(eKwdFor, "for")
WHO_TOKEN_VALUE(eKwdWhile, "while")
WHO_TOKEN_VALUE(eKwdDo, "do")
WHO_TOKEN_VALUE(eKwdTrue, "true")
WHO_TOKEN_VALUE(eKwdFalse, "false")
WHO_TOKEN_VALUE(eKwdIn, "in")
WHO_TOKEN_VALUE(eKwdBreak, "break")
WHO_TOKEN_VALUE(eKwdContinuer, "continue")

// Symbols
WHO_TOKEN_VALUE(eLParen, "(")
WHO_TOKEN_VALUE(eRParen, ")")
WHO_TOKEN_VALUE(eLBrace, "{")
WHO_TOKEN_VALUE(eRBrace, "}")
WHO_TOKEN_VALUE(eLBracket, "[")
WHO_TOKEN_VALUE(eRBracket, "]")
WHO_TOKEN_VALUE(eLAngular, "<")
WHO_TOKEN_VALUE(eRAngular, ">")
WHO_TOKEN_VALUE(eSemicolon, ";")
WHO_TOKEN_VALUE(eColon, ":")
WHO_TOKEN_VALUE(eDot, ".")
WHO_TOKEN_VALUE(eComma, ",")
WHO_TOKEN_VALUE(eAmpersand, "&")
WHO_TOKEN_VALUE(eStart, "*")
WHO_TOKEN_VALUE(ePlus, "+")
WHO_TOKEN_VALUE(eMinus, "-")
WHO_TOKEN_VALUE(eDivide, "/")
WHO_TOKEN_VALUE(ePercent, "%")
WHO_TOKEN_VALUE(eNegate, "!")
WHO_TOKEN_VALUE(eBitOr, "|")
WHO_TOKEN_VALUE(eBackslash, "\\")
WHO_TOKEN_VALUE(eEquals, "=")
WHO_TOKEN_VALUE(eXor, "^")
WHO_TOKEN_VALUE(eBitNeg, "~")

// Literals
WHO_TOKEN_VALUE(eStringLiteral, "<string literal>")
WHO_TOKEN_VALUE(eCharLiteral, "<char literal>")
WHO_TOKEN_VALUE(eIntLiteral, "<int literal>")
WHO_TOKEN_VALUE(eFloatLiteral, "<float literal>")

// Compound tokens
WHO_TOKEN_VALUE(eArrow, "->")
WHO_TOKEN_VALUE(eElvis, "?:")
WHO_TOKEN_VALUE(eNotEquals, "!=")
WHO_TOKEN_VALUE(ePlusEquals, "+=")
WHO_TOKEN_VALUE(eMinusEquals, "-=")
WHO_TOKEN_VALUE(eDivideEquals, "/=")
WHO_TOKEN_VALUE(eMulEquals, "*=")
WHO_TOKEN_VALUE(eBitOrEquals, "|=")
WHO_TOKEN_VALUE(eBitAndEquals, "&=")
WHO_TOKEN_VALUE(eBitXorEquals, "^=")
WHO_TOKEN_VALUE(eBitNegEquals, "~=")
WHO_TOKEN_VALUE(eBitShiftLeft, "<<")
WHO_TOKEN_VALUE(eBitShiftRight, ">>")
WHO_TOKEN_VALUE(eLEqual, "<=")
WHO_TOKEN_VALUE(eGEqual, ">=")
WHO_TOKEN_VALUE(eBoolEqual, "==")
WHO_TOKEN_VALUE(eIncrement, "++")
WHO_TOKEN_VALUE(eDecrement, "--")

WHO_END_TOKENS

#undef WHO_TOKEN_VALUE
#undef WHO_BEGIN_TOKENS
#undef WHO_END_TOKENS

export struct Token
{
	TokenType::Type type{};
	std::optional<std::string> lexeme{};
	int line{};
	int column{};
};

export struct TokenStream
{
	std::vector<Token> tokens;
	std::string file_path;
};
