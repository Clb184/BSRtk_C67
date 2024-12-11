#ifndef PTN_INCLUDED
#define PTN_INCLUDED

#include <fstream>
#include "json.hpp"
#include "utyl.hpp"
#include <locale>

//Normal PTN
bool DumpPTN(const char* name);
bool CompilePTN(const char* input, const char* output);

//Player defs for PTN
bool DumpPlayerPTN(const char* name);
bool CompilePlayerPTN(const char* input, const char* output);

#endif