#include "edt_dump.hpp"
#include "edt_compile.hpp"
#include "sounddef.hpp"

void PrintUsage(const char* exe) {
	printf("Usage:\n"
	"	%s ed file.edt (Dump)\n"
	"	%s ec source output.edt <- in pairs (Compile)\n",exe,exe);
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
	}
	else if (op == "ec" && !((argc-2) % 2)) {
		EDTCompiler compiler;
		int num_comp = (argc - 2) / 2;
		for (int i = 0; i < num_comp; i++) {
			compiler.Reset();
			if (!compiler.CompileEDT(argv[2 + i * 2], argv[2 + i * 2 + 1]))
				printf("Failed compiling %s.\n", argv[2 + i * 2]);
		}
	}
	else if (op == "sd") {
		DumpSoundDef(argv[2]);
	}
	else if (op == "sc") {
	}
	else {
		PrintUsage(argv[0]);
	}
}