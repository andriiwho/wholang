#include "ParserInitialPass.hh"

#include <optional>
#include <format>
#include <sstream>

/**
 * First pass of the parser analyzes all the types and functions that are used in this module.
 * It doesn't really resolve any types, just plain syntax tree.
 * Second pass will handle and resolve the types.
 */

using OPTIONAL_TOKEN = std::optional<std::reference_wrapper<const TOKEN>>;

static thread_local const TOKEN_STREAM* STREAM = nullptr;
static thread_local int POINTER = 0;

using std::nullopt;

[[noreturn]]
static void AssertStreamValid()
{
	throw std::runtime_error("STREAM is null");
}

static void Advance()
{
	POINTER++;
}

[[nodiscard]]
static const TOKEN* Deref(OPTIONAL_TOKEN token)
{
	if (!token)
		return nullptr;

	return &token.value().get();
}

[[nodiscard]]
static OPTIONAL_TOKEN Peek()
{
	if (!STREAM)
	{
		AssertStreamValid();
	}

	if (POINTER < STREAM->tokens.size())
	{
		return STREAM->tokens.at(POINTER);
	}

	return nullopt;
}

[[nodiscard]]
static bool IsAtEnd()
{
	const OPTIONAL_TOKEN peek = Peek();
	if (peek && peek.value().get().type == TOKEN_TYPE::END_OF_FILE)
	{
		return true;
	}

	return false;
}

[[nodiscard]]
static OPTIONAL_TOKEN PeekNext()
{
	if (!STREAM)
		AssertStreamValid();

	if ((POINTER + 1) < STREAM->tokens.size())
	{
		return STREAM->tokens.at(POINTER);
	}

	return nullopt;
}

[[nodiscard]]
static bool Check(TOKEN_TYPE::TYPE type)
{
	return Peek().has_value() && Deref(Peek())->type == type;
}

[[nodiscard]]
static bool Match(TOKEN_TYPE::TYPE type)
{
	if (Check(type))
	{
		Advance();
		return true;
	}

	return false;
}

template <typename... Args>
static const TOKEN* Consume(TOKEN_TYPE::TYPE type, std::format_string<Args...> format, Args&&... args)
{
	if (Check(type))
	{
		const TOKEN* outToken = Deref(Peek());
		Advance();
		return outToken;
	}

	throw std::runtime_error(std::format(format, std::forward<Args>(args)...));
}

[[nodiscard]]
static std::string ParseDottedIdentifiers(const TOKEN* firstToken)
{
	std::stringstream dottedName{};
	dottedName << *firstToken->lexeme;

	while (Match(TOKEN_TYPE::DOT))
	{
		firstToken = Consume(TOKEN_TYPE::IDENTIFIER, "Expected identifier after .");
		dottedName << '.' << firstToken->lexeme.value();
	}

	return dottedName.str();
}

[[nodiscard]]
static AST_NODE::PTR ParsePackageDefinition()
{
	Consume(TOKEN_TYPE::KWD_PACKAGE, "Translation unit should always start with package definition.");
	const TOKEN* nameToken = Consume(TOKEN_TYPE::IDENTIFIER, "Missing identifier after 'package'");
	if (!nameToken)
	{
		throw std::runtime_error("nameToken is null for some reason. This should not happen.");
	}

	auto packageDef = std::make_shared<AST_NODE_PACKAGE>();
	packageDef->name = ParseDottedIdentifiers(nameToken);
	Consume(TOKEN_TYPE::SEMICOLON, "; expected");

	return packageDef;
}

[[nodiscard]]
static AST_NODE::PTR ParseImportDefinition()
{
	Consume(TOKEN_TYPE::KWD_IMPORT, "Unknown token, expected 'package'");
	const auto packageName = Consume(TOKEN_TYPE::IDENTIFIER, "Unexpected token.");

	auto importDef = std::make_shared<AST_NODE_IMPORT_DEFINITION>();
	importDef->packageName = ParseDottedIdentifiers(packageName);
	Consume(TOKEN_TYPE::SEMICOLON, "Expected ;");

	return importDef;
}

static AST_NODE::PTR ParseTypeDefinition()
{
	return {};
}

static AST_NODE::PTR ParseFuncDefinition()
{
	return {};
}

static AST_NODE::PTR ParseTypeAnnotation()
{
	Consume(TOKEN_TYPE::COLON, ": expected.");

	const auto firstIdentifier = Consume(TOKEN_TYPE::IDENTIFIER, "Unexpected token. Expected identifier.");
	std::string fullType = ParseDottedIdentifiers(firstIdentifier);

	auto typeDef = std::make_shared<AST_NODE_TYPE_ANNOTATION>();
	typeDef->typeNameChain = std::move(fullType);

	return typeDef;
}

static AST_NODE::PTR ParseExpr()
{
	return {};
}

static AST_NODE::PTR ParseDataDefinition()
{
	auto varDefinition = std::make_shared<AST_NODE_VAR_DEFINITION>();

	Advance();

	const auto name = Consume(TOKEN_TYPE::IDENTIFIER, "Unexpected token. Expected identifier");
	varDefinition->baseName = name->lexeme.has_value() ? *name->lexeme : "unknown__";

	if (Check(TOKEN_TYPE::COLON))
	{
		varDefinition->type = ParseTypeAnnotation();
	}

	if(Check(TOKEN_TYPE::EQUALS))
	{
		
	}

	return varDefinition;
}

static AST_NODE::PTR ParseExportDefinition()
{
	Consume(TOKEN_TYPE::KWD_EXPORT, "Export expected");
	auto exportDef = std::make_shared<AST_NODE_EXPORT_DEFINITION>();

	if (Check(TOKEN_TYPE::KWD_TYPE))
	{
		exportDef->definition = ParseTypeDefinition();
	}

	if (Check(TOKEN_TYPE::KWD_FUNC))
	{
		exportDef->definition = ParseFuncDefinition();
	}

	if (Check(TOKEN_TYPE::KWD_VAR) || Check(TOKEN_TYPE::KWD_CONST))
	{
		exportDef->definition = ParseDataDefinition();
	}

	return exportDef;
}

std::shared_ptr<AST_NODE_TRANSLATION_UNIT> ParseTranslationUnit()
{
	auto translationUnit = std::make_shared<AST_NODE_TRANSLATION_UNIT>();
	translationUnit->package = ParsePackageDefinition();

	while (!IsAtEnd())
	{
		if (Check(TOKEN_TYPE::KWD_IMPORT))
		{
			translationUnit->children.push_back(ParseImportDefinition());
		}

		if (Check(TOKEN_TYPE::KWD_EXPORT))
		{
			translationUnit->children.push_back(ParseExportDefinition());
		}

		Advance();
	}

	return translationUnit;
}

std::shared_ptr<AST_NODE_TRANSLATION_UNIT> ParseTypeTree(const TOKEN_STREAM& tokens)
{
	STREAM = &tokens;

	// Translation unit should always start with package name
	auto translationUnit = ParseTranslationUnit();
	// Start parsing the package

	return nullptr;
}