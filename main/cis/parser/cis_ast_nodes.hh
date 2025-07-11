#pragma once

#include "cis/lexer/cis_lexer_forward.hh"

#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <algorithm>
#include <stdexcept>

enum class AST_NODE_TYPE
{
	UNKNOWN,
	TRANSLATION_UNIT,
	EXPORT_STATEMENT
};

struct AST_NODE : std::enable_shared_from_this<AST_NODE>
{
	AST_NODE_TYPE type;

	explicit AST_NODE(AST_NODE_TYPE type)
		: type(type)
	{
	}

	virtual ~AST_NODE() = default;

	using PTR = std::shared_ptr<AST_NODE>;
	using WEAK_REF = std::weak_ptr<AST_NODE>;

	template <typename T>
	[[nodiscard]]
	inline std::weak_ptr<T> as_weak() const
	{
		return std::weak_ptr<T>(std::static_pointer_cast<T>(shared_from_this()));
	}

	template <typename T>
	[[nodiscard]]
	inline std::shared_ptr<T> as() const
	{
		return std::static_pointer_cast<T>(shared_from_this());
	}
};

#define DECLARE_AST_NODE_POINTERS(nodeType)                             \
	using PTR = std::shared_ptr<nodeType>;                              \
	using WEAK_REF = std::weak_ptr<nodeType>;                           \
	template <typename... Args>                                         \
	static PTR make(Args&&... args)                                     \
	{                                                                   \
		return std::make_shared<nodeType>(std::forward<Args>(args)...); \
	}

struct AST_NODE_TOP_LEVEL_STATEMENT : AST_NODE
{
	DECLARE_AST_NODE_POINTERS(AST_NODE_TOP_LEVEL_STATEMENT);

	inline constexpr explicit AST_NODE_TOP_LEVEL_STATEMENT(AST_NODE_TYPE type)
		: AST_NODE(type)
	{
	}
};

struct AST_NODE_EXPORT_STATEMENT final : AST_NODE_TOP_LEVEL_STATEMENT
{
	DECLARE_AST_NODE_POINTERS(AST_NODE_EXPORT_STATEMENT);

	inline constexpr AST_NODE_EXPORT_STATEMENT()
		: AST_NODE_TOP_LEVEL_STATEMENT(AST_NODE_TYPE::EXPORT_STATEMENT)
	{
	}

	AST_NODE_TOP_LEVEL_STATEMENT::PTR statement;
};

struct AST_NODE_TRANSLATION_UNIT final : AST_NODE
{
	DECLARE_AST_NODE_POINTERS(AST_NODE_TRANSLATION_UNIT)

	inline constexpr AST_NODE_TRANSLATION_UNIT()
		: AST_NODE(AST_NODE_TYPE::TRANSLATION_UNIT)
	{
	}

	AST_NODE::PTR package;
	std::vector<AST_NODE::PTR> children;
};