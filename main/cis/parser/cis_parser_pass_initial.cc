#include "cis_parser_pass_initial.hh"
#include "cis/debug/cis_diag.hh"

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
static void assert_stream_valid();

[[noreturn]]
static void throw_unknown_parser_error()
{
	DIAGNOSTICS::error(
		STREAM->file_path,
		0,
		0,
		{},
		"Unknown parser error");
}

static void assert_stream_valid()
{
	throw_unknown_parser_error();
}

static void advance()
{
	POINTER++;
}

[[nodiscard]]
static const TOKEN* deref(OPTIONAL_TOKEN token)
{
	if (!token)
		return nullptr;

	return &token.value().get();
}

[[nodiscard]]
static OPTIONAL_TOKEN peek()
{
	if (!STREAM)
	{
		assert_stream_valid();
	}

	if (POINTER < STREAM->tokens.size())
	{
		return STREAM->tokens.at(POINTER);
	}

	return nullopt;
}

[[nodiscard]]
static bool is_at_end()
{
	const OPTIONAL_TOKEN peek_val = peek();
	if (peek_val && peek_val.value().get().type == TOKEN_TYPE::END_OF_FILE)
	{
		return true;
	}

	return false;
}

[[nodiscard]]
static OPTIONAL_TOKEN peek_next()
{
	if (!STREAM)
		assert_stream_valid();

	if ((POINTER + 1) < STREAM->tokens.size())
	{
		return STREAM->tokens.at(POINTER);
	}

	return nullopt;
}

[[nodiscard]]
static bool check(TOKEN_TYPE::TYPE type)
{
	return peek().has_value() && deref(peek())->type == type;
}

[[nodiscard]]
static bool match(TOKEN_TYPE::TYPE type)
{
	if (check(type))
	{
		advance();
		return true;
	}

	return false;
}

static const TOKEN* consume(TOKEN_TYPE::TYPE type)
{
	if (check(type))
	{
		const TOKEN* out_token = deref(peek());
		advance();
		return out_token;
	}

	if (const auto token = deref(peek()))
	{
		DIAGNOSTICS::error(
			STREAM->file_path,
			token->line,
			token->column,
			{},
			"Unexpected token '{}', expected '{}'",
			token->lexeme ? *token->lexeme : TOKEN_TYPE::to_string(token->type),
			TOKEN_TYPE::to_string(type));
	}

	throw_unknown_parser_error();
}

[[nodiscard]]
static std::string parse_dotted_identifiers(const TOKEN* first_token)
{
	std::stringstream dotted_name{};
	dotted_name << *first_token->lexeme;

	while (match(TOKEN_TYPE::DOT))
	{
		first_token = consume(TOKEN_TYPE::IDENTIFIER);
		dotted_name << '.' << first_token->lexeme.value();
	}

	return dotted_name.str();
}

static AST_NODE_TOP_LEVEL_STATEMENT::PTR parse_top_level_statement()
{
	
}

static AST_NODE_TRANSLATION_UNIT::PTR parse_translation_unit()
{
	auto out_ptr = AST_NODE_TRANSLATION_UNIT::make();
	while(!is_at_end())
	{
		out_ptr->children.push_back(parse_top_level_statement());
	}
	return out_ptr;
}

AST_NODE_TRANSLATION_UNIT::PTR parse_type_tree(const TOKEN_STREAM& tokens)
{
	STREAM = &tokens;

	// Translation unit should always start with the package name
	auto translationUnit = parse_translation_unit();
	// Start parsing the package

	return translationUnit;
}