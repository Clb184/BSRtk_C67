#include "edt_dump.hpp"
#include "edt_compile.hpp"

int main(int argc, char** argv) {
	if (argc < 2) return -1;
	switch (argc) {
	case 2:	DumpEDT(argv[1]); break;
	case 3: {
		EDTCompiler compiler;
		if (!compiler.CompileEDT(argv[1], argv[2])) 
			printf("Failed compiling file.\n");
	}
	}
}