module;

#include <optional>

export module wholang.parser.initial_pass.thread_local_token_stream;

import wholang.lexer.forward;
import wholang.lexer;
import wholang.parser.nodes;
import wholang.diag;

#define EXPORT export
#define EXPORT_NODISCARD export [[nodiscard]]

using OPTIONAL_TOKEN = std::optional<std::reference_wrapper<const Token>>;

// Token Stream.
static thread_local const TokenStream* s_thread_local_token_stream = nullptr;

EXPORT
void set_thread_local_token_stream(const TokenStream* in_stream)
{
	s_thread_local_token_stream = in_stream;
}

EXPORT_NODISCARD
const TokenStream* get_thread_local_token_stream()
{
	return s_thread_local_token_stream;
}

// Pointer
static thread_local int s_thread_local_pointer = 0;
EXPORT_NODISCARD
int get_thread_local_pointer()
{
	return s_thread_local_pointer;
}

// Valid for structs and code blocks
static thread_local ast::Node::WeakRef s_thread_local_definition_scope = {};

EXPORT_NODISCARD
ast::Node::WeakRef get_thread_local_definition_scope()
{
	return s_thread_local_definition_scope;
}

EXPORT
void set_thread_local_definition_scope(const ast::Node::WeakRef& in_scope)
{
	s_thread_local_definition_scope = in_scope;
}

using std::nullopt;

export [[noreturn]]
void throw_unknown_parser_error()
{
	diag::error(
		s_thread_local_token_stream->file_path,
		0,
		0,
		{},
		"Unknown parser error");
}

EXPORT
void assert_stream_valid()
{
	throw_unknown_parser_error();
}

EXPORT
void advance()
{
	s_thread_local_pointer++;
}

EXPORT_NODISCARD
const Token* deref(const OPTIONAL_TOKEN token)
{
	if (!token)
		return nullptr;

	return &token.value().get();
}

EXPORT_NODISCARD
OPTIONAL_TOKEN peek()
{
	if (!s_thread_local_token_stream)
	{
		assert_stream_valid();
	}

	if (s_thread_local_pointer < s_thread_local_token_stream->tokens.size())
	{
		return s_thread_local_token_stream->tokens.at(s_thread_local_pointer);
	}

	return nullopt;
}

EXPORT_NODISCARD
bool is_at_end()
{
	const OPTIONAL_TOKEN peek_val = peek();
	if (peek_val && peek_val.value().get().type == TokenType::eEOF)
	{
		return true;
	}

	return false;
}

EXPORT_NODISCARD
OPTIONAL_TOKEN peek_next()
{
	if (!s_thread_local_token_stream)
		assert_stream_valid();

	if ((s_thread_local_pointer + 1) < s_thread_local_token_stream->tokens.size())
	{
		return s_thread_local_token_stream->tokens.at(s_thread_local_pointer);
	}

	return nullopt;
}

EXPORT_NODISCARD
bool check(TokenType::Type type)
{
	return peek().has_value() && deref(peek())->type == type;
}

EXPORT_NODISCARD
bool match(const TokenType::Type type)
{
	if (check(type))
	{
		advance();
		return true;
	}

	return false;
}

EXPORT
const Token* consume(const TokenType::Type type)
{
	if (check(type))
	{
		const Token* out_token = deref(peek());
		advance();
		return out_token;
	}

	if (const auto token = deref(peek()))
	{
		diag::error(
			s_thread_local_token_stream->file_path,
			token->line,
			token->column,
			{},
			"Unexpected token '{}', expected '{}'",
			token->lexeme ? *token->lexeme : TokenType::to_string(token->type),
			TokenType::to_string(type));
	}

	throw_unknown_parser_error();
}
