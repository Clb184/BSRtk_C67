#ifndef EDT_DUMP_INCLUDED
#define EDT_DUMP_INCLUDED
#include "utyl.hpp"

typedef std::map<address, ProcData> address_map;

static address_map g_ProcData;
static address_map g_LabelData;
static std::vector<ECLInstructionData> g_InstructionData;

bool DumpEDT(const char* name);

bool ParseHeader(uint8_t* data, const EDTHeader& header, size_t);
bool ParseData(uint8_t* data, size_t size, size_t& ind, int tex_init);
bool ParseTexInit(unsigned char* data, size_t& idx);
bool PrintEDT();


#endif // !EDT_DUMP_INCLUDED
