#pragma once

#include "AstNodes.hh"

[[nodiscard]]
std::shared_ptr<AST_NODE_TRANSLATION_UNIT> ParseTypeTree(const TOKEN_STREAM& tokens);