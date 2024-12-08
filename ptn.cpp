#include "ptn.hpp"

bool DumpPTN(const char* name) {
	bool success = true;
	size_t size;
	if (char* pBuffer = (char*)GetDataFromFile(name, size)) {
		char* idx = pBuffer;
		int num_cutin = 0;
		int num_portraits = 0;
		//cut-ins and portrait count are the first two shorts of this kind of PTN
		num_cutin = *(short*)(idx);
		num_portraits = *(short*)(idx + 2);
		idx += 4;
		printf("{\n");

		//First we get the cut-ins if there are any
		printf("	\"cut_in\" : [\n");
		num_cutin--;
		for (int i = 0; i <= num_cutin; i++) {
			printf("		\"%s\"%c\n", idx, (i < num_cutin) ? ',' : ' ');
			idx += strnlen(idx, size) + 1;
		}
		printf("	],\n\n");
		
		//Then we get the rects if there are any
		int num_rects = *(short*)idx - 1;
		idx += 2;
		printf("	\"rects\" : [\n");
		for (int i = 0; i <= num_rects; i++) {
			printf("		{\n");
			printf("			\"tex\" : %d,\n", *(short*)(idx + 8));
			printf("			\"u0\" : %d,\n", *(short*)(idx + 0));
			printf("			\"v0\" : %d,\n", *(short*)(idx + 2));
			printf("			\"u1\" : %d,\n", *(short*)(idx + 4));
			printf("			\"v1\" : %d\n", *(short*)(idx + 6));
			printf("		}%c\n", (i < num_rects) ? ',' : ' ');
			idx += 2 * 5;
		}
		printf("	],\n\n");

		//Finally we get the portraits if there are any
		printf("	\"portraits\" : [\n");
		num_portraits--;
		for (int i = 0; i <= num_portraits; i++) {
			printf("		\"%s\"%c\n", idx, (i < num_portraits) ? ',' : ' ');
			idx += strnlen(idx, size) + 1;
		}
		printf("	]\n");


		printf("}\n");
		free(pBuffer);
	}
	else {
		printf("Failed opening %s\n", name);
		success = false;
	}

	return success;
}

bool CompilePTN(const char* input, const char* output) {
	bool success = true;
	std::ifstream src(input);
	if (!src.is_open()) return false;
	nlohmann::json json = nlohmann::json::parse(src);

	size_t output_size = 2 + 2 + 2; //First two shorts + num of rects (short)
	short num_cutins = 0;
	short num_portraits = 0;
	short num_rects = 0;

	struct PTNRect {
		u16 u0 = 0;
		u16 v0 = 0;
		u16 u1 = 0;
		u16 v1 = 0;
		u16 tx = 0;
	};
	std::vector<PTNRect> rcs;
	std::vector<std::string> cut;
	std::vector<std::string> prt;
	try {
		const std::vector<std::string>& cut_ins = json["cut_in"];
		for (auto& s : cut_ins) {
			output_size += s.length() + 1;
			cut.emplace_back(s);
		}
		num_cutins = cut_ins.size();

		const auto& rects = json["rects"];
		for (auto& s : rects) {
			rcs.emplace_back(s["u0"], s["v0"], s["u1"], s["v1"], s["tex"]);
		}
		num_rects = rects.size();
		output_size += num_rects * sizeof(PTNRect);

		const std::vector<std::string>& portraits = json["portraits"];
		for (auto& s : portraits) {
			output_size += s.length() + 1;
			prt.emplace_back(s);
		}
		num_portraits = portraits.size();
	}
	catch (const std::exception& e) {
		printf("An error ocurred: %s.\n", e.what());
		success = false;
	}

	if (success) {
		char* pBuffer = (char*)malloc(output_size);
		char* idx = pBuffer;

		memset(pBuffer, 0x00, output_size);

		//Fill count of cut-in and portraits
		*(short*)idx = num_cutins;
		*(short*)(idx + 2) = num_portraits;
		idx += 2 + 2;

		//Fill cut-in names
		for (int i = 0; i < num_cutins; i++) {
			size_t ln = cut[i].length();
			memcpy(idx, cut[i].data(), ln);
			idx += ln + 1;
		}
		
		//Fill count of rects and their data
		*(short*)idx = num_rects;
		idx += 2;
		memcpy(idx, rcs.data(), rcs.size() * sizeof(PTNRect));
		idx += rcs.size() * sizeof(PTNRect);

		//Finally, fill portrait names
		for (int i = 0; i < num_portraits; i++) {
			size_t ln = prt[i].length();
			memcpy(idx, prt[i].data(), ln);
			idx += ln + 1;
		}

		//Write data to file
		if (FILE* fp = fopen(output, "wb")) {
			fwrite(pBuffer, output_size, 1, fp);
			fclose(fp);
		}
		else {
			printf("Failed writing file.\n");
			success = false;
		}
		free(pBuffer);
	}

	return success;
}
