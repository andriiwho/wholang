#pragma once

#include "cis_lexer_forward.hh"

TOKEN_STREAM lex_evaluate_source(const std::string& file_path, const std::string& file_contents);

void lex_dump_tokens(const TOKEN_STREAM& in_stream);