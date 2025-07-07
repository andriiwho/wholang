#pragma once

#include "LexFwd.hh"

TOKEN_STREAM LexEvaluateSource(const std::string& filePath, const std::string& fileContents);
void LexDumpTokens(const TOKEN_STREAM& inStream);