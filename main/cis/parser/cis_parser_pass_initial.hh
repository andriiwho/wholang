#pragma once

#include "cis_ast_nodes.hh"

[[nodiscard]]
AST_NODE_TRANSLATION_UNIT::PTR parse_type_tree(const TOKEN_STREAM& tokens);