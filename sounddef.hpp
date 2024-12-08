#ifndef SOUNDDEF_INCLUDED
#define SOUNDDEF_INCLUDED

#include "json.hpp"
#include <fstream>
#include "utyl.hpp"

//Put Sound.def here to dump its contents
bool DumpSoundDef(const char* file);

//Compile Sound.def
bool CompileSoundDef(const char* source, const char* output);

#endif