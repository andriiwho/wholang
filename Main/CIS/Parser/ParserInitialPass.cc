#include "ParserInitialPass.hh"
#include "CIS/Debug/Diagnostics.hh"

#include <optional>
#include <sstream>

/**
 * First pass of the parser analyzes all the types and functions that are used in this module.
 * It doesn't really resolve any types, just plain syntax tree.
 * Second pass will handle and resolve the types.
 */

using OPTIONAL_TOKEN = std::optional<std::reference_wrapper<const TOKEN>>;

static thread_local const TOKEN_STREAM* STREAM = nullptr;
static thread_local int POINTER = 0;

// Valid for structs and code blocks
static thread_local AST_NODE::WEAK_REF CURRENT_SCOPE = {};

using std::nullopt;

[[noreturn]]
static void AssertStreamValid();

[[noreturn]]
static void ThrowUnknownParserError()
{
	DIAGNOSTICS::AddError(
		STREAM->filePath,
		0,
		0,
		{},
		"Unknown parser error");
}

static void AssertStreamValid()
{
	ThrowUnknownParserError();
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

static const TOKEN* Consume(TOKEN_TYPE::TYPE type)
{
	if (Check(type))
	{
		const TOKEN* outToken = Deref(Peek());
		Advance();
		return outToken;
	}

	if (const auto token = Deref(Peek()))
	{
		DIAGNOSTICS::AddError(
			STREAM->filePath,
			token->line,
			token->column,
			{},
			"Unexpected token '{}', expected '{}'",
			token->lexeme ? *token->lexeme : TOKEN_TYPE::ToString(token->type),
			TOKEN_TYPE::ToString(type));
	}

	ThrowUnknownParserError();
}

[[nodiscard]]
static std::string ParseDottedIdentifiers(const TOKEN* firstToken)
{
	std::stringstream dottedName{};
	dottedName << *firstToken->lexeme;

	while (Match(TOKEN_TYPE::DOT))
	{
		firstToken = Consume(TOKEN_TYPE::IDENTIFIER);
		dottedName << '.' << firstToken->lexeme.value();
	}

	return dottedName.str();
}

[[nodiscard]]
static AST_NODE::PTR ParsePackageDefinition()
{
	Consume(TOKEN_TYPE::KWD_PACKAGE);
	const TOKEN* nameToken = Consume(TOKEN_TYPE::IDENTIFIER);
	if (!nameToken)
	{
		ThrowUnknownParserError();
	}

	auto packageDef = std::make_shared<AST_NODE_PACKAGE>();
	packageDef->name = ParseDottedIdentifiers(nameToken);
	Consume(TOKEN_TYPE::SEMICOLON);

	return packageDef;
}

[[nodiscard]]
static AST_NODE::PTR ParseImportDefinition()
{
	Consume(TOKEN_TYPE::KWD_IMPORT);
	const auto packageName = Consume(TOKEN_TYPE::IDENTIFIER);

	auto importDef = std::make_shared<AST_NODE_IMPORT_DEFINITION>();
	importDef->packageName = ParseDottedIdentifiers(packageName);
	Consume(TOKEN_TYPE::SEMICOLON);

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
	Consume(TOKEN_TYPE::COLON);

	const auto firstIdentifier = Consume(TOKEN_TYPE::IDENTIFIER);
	std::string fullType = ParseDottedIdentifiers(firstIdentifier);

	auto typeDef = std::make_shared<AST_NODE_TYPE_ANNOTATION>();
	typeDef->typeNameChain = std::move(fullType);

	return typeDef;
}

static AST_NODE::PTR ParseUnresolvedSymbol()
{
	std::string symbolName = ParseDottedIdentifiers(Consume(TOKEN_TYPE::IDENTIFIER));
	auto outNode = std::make_shared<AST_NODE_UNRESOLVED_SYMBOL>();
	outNode->valueLexeme = symbolName;
	return outNode;

	// LARGE TODO: We need to parse function calls for assignments (as well as expressions)
}

static constexpr AST_NODE_TYPE LiteralTypeToASTNodeType(TOKEN_TYPE::TYPE type)
{
	switch (type)
	{
		case TOKEN_TYPE::INTEGRAL_LITERAL:
			return AST_NODE_TYPE::INT_LITERAL;
		case TOKEN_TYPE::FLOAT_LITERAL:
			return AST_NODE_TYPE::FLOAT_LITERAL;
		case TOKEN_TYPE::STRING_LITERAL:
			return AST_NODE_TYPE::STRING_LITERAL;
		case TOKEN_TYPE::CHAR_LITERAL:
			return AST_NODE_TYPE::CHAR_LITERAL;
		default:
			break;
	}

	ThrowUnknownParserError();
}

static AST_NODE::PTR ParseLiteral()
{
	if (const TOKEN* token = Deref(Peek()))
	{
		auto ptr = std::make_shared<AST_NODE_LITERAL>(LiteralTypeToASTNodeType(token->type));
		ptr->valueLexeme = *token->lexeme;
		Advance();
		return ptr;
	}

	ThrowUnknownParserError();
}

static AST_NODE::PTR ParseAssignment()
{
	Consume(TOKEN_TYPE::EQUALS);

	if (Check(TOKEN_TYPE::IDENTIFIER))
	{
		auto out = ParseUnresolvedSymbol();
		Consume(TOKEN_TYPE::SEMICOLON);
		return out;
	}

	if (Check(TOKEN_TYPE::INTEGRAL_LITERAL) || Check(TOKEN_TYPE::STRING_LITERAL) || Check(TOKEN_TYPE::CHAR_LITERAL) || Check(TOKEN_TYPE::FLOAT_LITERAL))
	{
		auto out = ParseLiteral();
		Consume(TOKEN_TYPE::SEMICOLON);
		return out;
	}

	// TODO: This all should be replaced with return ParseExpr();
	return nullptr;
}

static AST_NODE::PTR ParseExpr()
{
	return ParseAssignment();
}

static AST_NODE::PTR ParseDataDefinition()
{
	auto varDefinition = std::make_shared<AST_NODE_VAR_DEFINITION>();
	varDefinition->scope = CURRENT_SCOPE;

	Advance();

	const auto name = Consume(TOKEN_TYPE::IDENTIFIER);
	varDefinition->baseName = name->lexeme.has_value() ? *name->lexeme : "unknown__";

	if (Check(TOKEN_TYPE::COLON))
	{
		varDefinition->type = ParseTypeAnnotation();
	}

	if (Check(TOKEN_TYPE::EQUALS))
	{
		varDefinition->value = ParseExpr();
	}

	return varDefinition;
}

static AST_NODE::PTR ParseExportDefinition()
{
	Consume(TOKEN_TYPE::KWD_EXPORT);
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
	CURRENT_SCOPE = translationUnit;

	while (!IsAtEnd())
	{
		if (Check(TOKEN_TYPE::KWD_IMPORT))
		{
			translationUnit->children.push_back(ParseImportDefinition());
		}
		else if (Check(TOKEN_TYPE::KWD_EXPORT))
		{
			translationUnit->children.push_back(ParseExportDefinition());
		}
		else
		{
			Advance();
		}
	}

	return translationUnit;
}

std::shared_ptr<AST_NODE_TRANSLATION_UNIT> ParseTypeTree(const TOKEN_STREAM& tokens)
{
	STREAM = &tokens;

	// Translation unit should always start with the package name
	auto translationUnit = ParseTranslationUnit();
	// Start parsing the package

	return translationUnit;
}