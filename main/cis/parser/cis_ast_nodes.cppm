module;

#include <memory>
#include <vector>
#include <string>

export module wholang.parser.nodes;

namespace ast
{
	export enum class NodeType
	{
		eUnknown,
		eTranslationUnit,
		eExportStmt,
		ePackageDecl,
		eVarDefinition,
		eExpression,

		eUnresolvedType,
		eLiteral,
		eIdentifier,
	};

	export struct Node : std::enable_shared_from_this<Node>
	{
		NodeType type;

		explicit Node(NodeType type)
			: type(type)
		{
		}

		virtual ~Node() = default;

		using Ptr = std::shared_ptr<Node>;
		using WeakRef = std::weak_ptr<Node>;

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
	using Ptr = std::shared_ptr<nodeType>;                              \
	using WeakRef = std::weak_ptr<nodeType>;                            \
	template <typename... Args>                                         \
	static Ptr make(Args&&... args)                                     \
	{                                                                   \
		return std::make_shared<nodeType>(std::forward<Args>(args)...); \
	}

	export struct TopLevelStmt : Node
	{
		DECLARE_AST_NODE_POINTERS(TopLevelStmt);

		explicit TopLevelStmt(NodeType type)
			: Node(type)
		{
		}
	};

	export struct PackageDecl final : TopLevelStmt
	{
		DECLARE_AST_NODE_POINTERS(PackageDecl);

		explicit PackageDecl()
			: TopLevelStmt(NodeType::ePackageDecl)
		{
		}

		std::string package_name{};
	};

	export struct ExportStmt final : TopLevelStmt
	{
		DECLARE_AST_NODE_POINTERS(ExportStmt);

		inline ExportStmt()
			: TopLevelStmt(NodeType::eExportStmt)
		{
		}

		TopLevelStmt::Ptr statement;
	};

	export struct TopLevelVarDefinition final : TopLevelStmt
	{
		DECLARE_AST_NODE_POINTERS(TopLevelVarDefinition);

		inline TopLevelVarDefinition()
			: TopLevelStmt(NodeType::eVarDefinition)
			, is_const(false)
		{
		}

		bool is_const;
		std::string name;
		Node::Ptr type_assignment;
		Node::Ptr value_assignment;
	};

	export struct Literal final : Node
	{
		DECLARE_AST_NODE_POINTERS(Literal);

		inline Literal()
			: Node(NodeType::eLiteral)
		{
		}

		std::string value;
		Node::Ptr literal_type;
	};

	export struct Identifier final : Node
	{
		DECLARE_AST_NODE_POINTERS(Identifier);

		inline Identifier()
			: Node(NodeType::eIdentifier)
		{
		}

		std::string name;
		Node::Ptr identifier_type;
	};

	export struct Expression final : Node
	{
		DECLARE_AST_NODE_POINTERS(Expression);

		inline Expression()
			: Node(NodeType::eExpression)
		{
		}
	};

	export struct UnresolvedType final : Node
	{
		DECLARE_AST_NODE_POINTERS(UnresolvedType);

		inline UnresolvedType()
			: Node(NodeType::eUnresolvedType)
		{
		}

		std::string name;
	};

	export struct TranslationUnit final : Node
	{
		DECLARE_AST_NODE_POINTERS(TranslationUnit);

		inline TranslationUnit()
			: Node(NodeType::eTranslationUnit)
		{
		}

		Node::Ptr package;
		std::vector<Node::Ptr> children;
	};
} // namespace ast