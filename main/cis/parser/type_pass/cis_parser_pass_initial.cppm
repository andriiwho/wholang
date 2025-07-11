module;

#include <optional>
#include <sstream>

export module wholang.parser;

import wholang.lexer.forward;
import wholang.lexer;
export import wholang.parser.nodes;
import wholang.diag;

import wholang.parser.initial_pass.thread_local_token_stream;

/**
 * The first pass of the parser analyzes all the types and functions that are used in this module.
 * It doesn't really resolve any types, just a plain syntax tree.
 * The second pass will handle and resolve the types.
 */

#define LOCAL static
#define LOCAL_NODISCARD [[nodiscard]] LOCAL

// -----------------------------------------------------------------------------------------
LOCAL_NODISCARD
std::string parse_dotted_identifiers(const Token* first_token)
{
	std::stringstream dotted_name{};
	dotted_name << *first_token->lexeme;

	while (match(TokenType::eDot))
	{
		first_token = consume(TokenType::eIdentifier);
		dotted_name << '.' << first_token->lexeme.value();
	}

	return dotted_name.str();
}

// -----------------------------------------------------------------------------------------
LOCAL_NODISCARD
ast::TopLevelStmt::Ptr parse_top_level_statement();

// -----------------------------------------------------------------------------------------
LOCAL_NODISCARD ast::PackageDecl::Ptr parse_package_declaration()
{
	consume(TokenType::eKwdPackage);
	const std::string package_name = parse_dotted_identifiers(consume(TokenType::eIdentifier));
	auto out_decl = ast::PackageDecl::make();
	out_decl->package_name = package_name;

	consume(TokenType::eSemicolon);
	return out_decl;
}

// -----------------------------------------------------------------------------------------
LOCAL_NODISCARD
ast::ExportStmt::Ptr parse_export_statement()
{
	consume(TokenType::eKwdExport);

	auto out_ptr = ast::ExportStmt::make();
	out_ptr->statement = parse_top_level_statement();
	return out_ptr;
}

// -----------------------------------------------------------------------------------------
ast::Node::Ptr parse_identifier()
{
	return nullptr;
}

// -----------------------------------------------------------------------------------------
ast::Node::Ptr parse_literal()
{
	if (check(TokenType::eStringLiteral))
	{

	}

	return nullptr;
}

// -----------------------------------------------------------------------------------------
LOCAL_NODISCARD
ast::Expression::Ptr parse_expression(int min_precedence);

LOCAL_NODISCARD
ast::Node::Ptr parse_primary()
{
	const bool is_literal = check(TokenType::eStringLiteral)
		|| check(TokenType::eIntLiteral)
		|| check(TokenType::eFloatLiteral)
		|| check(TokenType::eCharLiteral);

	if (is_literal)
	{
		return parse_literal();
	}

	if (check(TokenType::eIdentifier))
	{
		return parse_identifier();
	}

	if (check(TokenType::eLParen))
	{
		consume(TokenType::eLParen);
		const auto expr = parse_expression(0);
		consume(TokenType::eRParen);
		return expr;
	}
}

// -----------------------------------------------------------------------------------------
ast::Expression::Ptr parse_expression(int min_precedence)
{
	auto lhs = parse_primary();

	return nullptr;
}

// -----------------------------------------------------------------------------------------
LOCAL_NODISCARD
ast::Node::Ptr parse_unresolved_type_annotation()
{
	consume(TokenType::eColon);
	const auto first_id = consume(TokenType::eIdentifier);
	const std::string type_name = parse_dotted_identifiers(first_id);

	const auto out_unresolved_type = ast::UnresolvedType::make();
	out_unresolved_type->name = type_name;
	return out_unresolved_type;
}

// -----------------------------------------------------------------------------------------
LOCAL_NODISCARD
ast::TopLevelVarDefinition::Ptr parse_top_level_var_definition()
{
	const bool is_const = check(TokenType::eKwdConst);
	consume(is_const ? TokenType::eKwdConst : TokenType::eKwdVar);

	const auto var_name = consume(TokenType::eIdentifier);
	if (!check(TokenType::eColon))
	{
		diag::error(
			get_thread_local_token_stream()->file_path,
			var_name->line,
			var_name->column,
			{},
			"Expected ':' after variable name. Type assignment is required for global variables.");
	}

	const auto type_annotation = parse_unresolved_type_annotation();
	if (!check(TokenType::eEquals))
	{
		diag::error(
			get_thread_local_token_stream()->file_path,
			var_name->line,
			var_name->column,
			{},
			"Expected '=' after variable name. Value assignment is required for global variables.");
	}
	consume(TokenType::eEquals);
	const auto assignment_value = parse_expression(0);

	auto out_ptr = ast::TopLevelVarDefinition::make();
	out_ptr->is_const = is_const;
	out_ptr->name = var_name->lexeme.value();
	out_ptr->type_assignment = type_annotation;
	out_ptr->value_assignment = assignment_value;

	return out_ptr;
}

// -----------------------------------------------------------------------------------------
LOCAL_NODISCARD
ast::TopLevelStmt::Ptr parse_top_level_statement()
{
	if (check(TokenType::eKwdPackage))
	{
		return parse_package_declaration();
	}

	if (check(TokenType::eKwdExport))
	{
		return parse_export_statement();
	}

	if (check(TokenType::eKwdVar) || check(TokenType::eKwdConst))
	{
		return parse_top_level_var_definition();
	}

	consume(TokenType::eEOF);
	throw_unknown_parser_error();
}

// -----------------------------------------------------------------------------------------
LOCAL_NODISCARD
ast::TranslationUnit::Ptr parse_translation_unit()
{
	auto out_ptr = ast::TranslationUnit::make();
	set_thread_local_definition_scope(out_ptr);

	while (!is_at_end())
	{
		if (auto tls = parse_top_level_statement())
		{
			out_ptr->children.push_back(tls);
		}
		else
		{
			consume(TokenType::eEOF);
		}
	}

	return out_ptr;
}

// -----------------------------------------------------------------------------------------
export ast::TranslationUnit::Ptr parse_type_tree(const TokenStream& tokens)
{
	set_thread_local_token_stream(&tokens);

	// Translation unit should always start with the package name
	auto translationUnit = parse_translation_unit();
	// Start parsing the package

	return translationUnit;
}