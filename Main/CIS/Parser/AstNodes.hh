#pragma once

#include "CIS/Lexer/LexFwd.hh"

#include <memory>
#include <string>
#include <vector>
#include <algorithm>

enum class AST_NODE_TYPE
{
	UNKNOWN,
	TRANSLATION_UNIT,
	PACKAGE,
	IMPORT_DEFINITION,
	EXPORT_DEFINITION,
	VAR_DEFINITION,
	CONSTANT_DEFINITION,
	FUNC_DEFINITION,
	TYPE_DEFINITION,
	TYPE_ANNOTATION,

	TERM,
	FACTOR,
	EXPR,
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
	using WEAK_REF = std::unique_ptr<AST_NODE>;

	template <typename T>
	[[nodiscard]]
	inline std::weak_ptr<T> AsWeak() const
	{
		return std::weak_ptr<T>(std::static_pointer_cast<T>(shared_from_this()));
	}

	template <typename T>
	[[nodiscard]]
	inline std::shared_ptr<T> As() const
	{
		return std::static_pointer_cast<T>(shared_from_this());
	}
};

struct AST_NODE_TRANSLATION_UNIT final : AST_NODE
{
	inline AST_NODE_TRANSLATION_UNIT()
		: AST_NODE(AST_NODE_TYPE::TRANSLATION_UNIT)
	{
	}

	AST_NODE::PTR package;
	std::vector<AST_NODE::PTR> children;
};

struct AST_NODE_PACKAGE final : AST_NODE
{
	inline AST_NODE_PACKAGE()
		: AST_NODE(AST_NODE_TYPE::PACKAGE)
	{
	}

	std::string name;
};

struct AST_NODE_IMPORT_DEFINITION final : AST_NODE
{
	inline AST_NODE_IMPORT_DEFINITION()
		: AST_NODE(AST_NODE_TYPE::IMPORT_DEFINITION)
	{
	}

	std::string packageName{};
};

struct AST_NODE_EXPORT_DEFINITION final : AST_NODE
{
	inline AST_NODE_EXPORT_DEFINITION()
		: AST_NODE(AST_NODE_TYPE::EXPORT_DEFINITION)
	{
	}

	AST_NODE::PTR definition;
};

struct AST_NODE_VAR_DEFINITION final : AST_NODE
{
	inline AST_NODE_VAR_DEFINITION()
		: AST_NODE(AST_NODE_TYPE::VAR_DEFINITION)
	{}

	std::string baseName;
	AST_NODE::PTR type;
	AST_NODE::PTR value;
};

struct AST_NODE_TYPE_ANNOTATION final : AST_NODE
{
	inline AST_NODE_TYPE_ANNOTATION()
		: AST_NODE(AST_NODE_TYPE::TYPE_ANNOTATION)
	{}

	std::string typeNameChain{};
};

struct AST_NODE_TERM final : AST_NODE
{
	inline AST_NODE_TERM()
		: AST_NODE(AST_NODE_TYPE::TERM)
	{}


};