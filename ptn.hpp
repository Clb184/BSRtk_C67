#ifndef PTN_INCLUDED
#define PTN_INCLUDED

#include <fstream>
#include "json.hpp"
#include "utyl.hpp"

bool DumpPTN(const char* name);
bool CompilePTN(const char* input, const char* output);

#endif