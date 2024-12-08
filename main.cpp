#include "edt_dump.hpp"
#include "edt_compile.hpp"
#include "sounddef.hpp"

void PrintUsage(const char* exe) {
	printf("Usage: %s\n"
	"	ed file.edt (Dump EDT)\n"
	"	ec source output.edt <- in pairs (Compile EDT)\n"
	"	sd sound.def (Dump Sound defs)\n"
	"	sc input.json output.def (Dump Sound defs)\n"
		, exe);
}

int main(int argc, char** argv) {
	if (argc < 3) {
		PrintUsage(argv[0]);
		return -1;
	}
	const std::string& op = argv[1];
	if (op == "ed") {
		int num_dec = (argc - 2);
		DumpEDT(argv[2]);
		return -1;
	}
	else if (op == "ec" && !((argc-2) % 2)) {
		EDTCompiler compiler;
		int num_comp = (argc - 2) / 2;
		for (int i = 0; i < num_comp; i++) {
			compiler.Reset();
			if (!compiler.CompileEDT(argv[2 + i * 2], argv[2 + i * 2 + 1])) {
				printf("Failed compiling EDT %s.\n", argv[2 + i * 2]);
				return -1;
			}
		}
	}
	else if (op == "sd") {
		DumpSoundDef(argv[2]);
	}
	else if (op == "sc" && !((argc - 2) % 2)) {
		int num_comp = (argc - 2) / 2;
		for (int i = 0; i < num_comp; i++) {
			if (!CompileSoundDef(argv[2 + i * 2], argv[2 + i * 2 + 1])) {
				printf("Failed compiling SoundDef %s.\n", argv[2 + i * 2]);
				return -1;
			}
		}
	}
	else {
		PrintUsage(argv[0]);
	}
	return 0;
}