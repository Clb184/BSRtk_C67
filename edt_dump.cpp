#include "edt_dump.hpp"

auto processData = [&](ECL_INSTRUCTION inst, uint8_t*& pSCLData, int& idx) {
	ECLInstructionDefine def = g_InstructionSize[inst];
	if (inst == PUSHR && *(pSCLData) == 0x2a)
		def.paramdatatype.emplace_back(U8);
	else if (inst == EXIT) {
		ProcData pdata;
		const std::string& address = std::to_string(idx + 1);
		pdata.name = "PROC" + address;
		g_ProcData.insert({idx + 1, std::move(pdata)});
	}

	ECLInstructionData data;
	data.cnt = def.paramdatatype.size();
	data.add = idx;
	int offset = 0;

	for (int i = 0; i < data.cnt; i++) {

		ECLParamData paramdata;
		paramdata.datatype = def.paramdatatype[i];
		switch (paramdata.datatype) {
		case ADDRESS: {
			address ads = *(address*)pSCLData;
			bool is_proc = true;
			paramdata.ads = ads;
			const std::string& address = std::to_string(ads);
			ProcData pdata;
			switch (data.cmd) {
			case ATK:
				pdata.type = PROC_ATK;
				break;
			case ENEMYCIRCLE:
			case CHILD:
				pdata.type = PROC_ENEMY;
				break;
			case BOSS:
				pdata.type = PROC_BOSS;
				break;
			case ANMTASK:
				pdata.type = PROC_EXANM;
				break;
			case BEGINCALLBACK:
			case ENDCALLBACK:
			case TJMP:
			case FJMP:
			case JMP:
			case OJMP:
			case AJMP:
			case LJMP:
				is_proc = false;
				pdata.type = PROC_LABEL;
				break;
			default:
				pdata.type = PROC_DEF;
				break;
			}
			pdata.name = (is_proc ? "PROC" : "ADDRESS") + std::to_string(ads);

			if (!is_proc)g_LabelData.insert({ ads, std::move(pdata) });
			else if (g_ProcData.find(ads) == g_ProcData.end()) {
				g_ProcData.insert({ ads, std::move(pdata) });
			}
			else {
				g_ProcData[ads] = std::move(pdata);
			}
			pSCLData += 4;
			offset += 4;
		}	break;
		case U32:
			paramdata.sdword = *(u32*)pSCLData;
			pSCLData += 4;
			offset += 4;
			break;
		case I32:
			paramdata.dword = *(i32*)pSCLData;
			pSCLData += 4;
			offset += 4;
			break;
		case U16:
			paramdata.word = *(u16*)pSCLData;
			pSCLData += 2;
			offset += 2;
			break;
		case I16:
			paramdata.sword = *(i16*)pSCLData;
			pSCLData += 2;
			offset += 2;
			break;
		case COMMAND:
			data.cmd = *(ECL_INSTRUCTION*)pSCLData;
			pSCLData++;
			offset++;
			break;
		case U8:
			paramdata.byte = *(u8*)pSCLData;
			pSCLData++;
			offset++;
			break;
		case I8:
			paramdata.sbyte = *(i8*)pSCLData;
			pSCLData++;
			offset++;
			break;
		case STRING:
			paramdata.stringdata = (char*)pSCLData;
			while (*pSCLData++) offset++;
			offset++;
			break;
		}
		data.param.emplace_back(paramdata);
	}
	idx += offset;
	return data;
	};

auto printinst = [](const ECLInstructionData& data) {
	int mx = data.cnt - 1;
	bool is_register_based = (data.cmd == PUSHR || data.cmd == POPR || data.cmd == MOVC);
	if (is_register_based && data.cmd == PUSHR && data.param[0].byte == 0x2a) {
		printf("PARENT %d", data.param[2].byte);
		return;
	}
	printf("%s ", g_InstructionSize[(ECL_INSTRUCTION)data.cmd].name);
	u8 reg = 0;
	if (is_register_based) {
		switch (data.cmd) {
		case PUSHR:
		case POPR: printf("%d", data.param[0].byte); break;
		case MOVC: printf("%d, %d", data.param[0].sbyte, data.param[2].sdword); break;
		}
		return;
	}
	for (int i = 0; i <= mx; i++) {
		ECL_DATATYPE datatype = data.param[i].datatype;
		switch (datatype) {
		case ADDRESS:
			if (data.cmd >= TJMP && data.cmd <= LJMP || data.cmd == BEGINCALLBACK || data.cmd == ENDCALLBACK) {
				address ads = data.param[i].ads;
				if (g_LabelData.find(ads) != g_LabelData.end())
					printf("%s", g_LabelData[ads].name.c_str());
			}
			else {
				address ads = data.param[i].ads;
				if (g_ProcData.find(ads) != g_ProcData.end())
					printf("%s", g_ProcData[ads].name.c_str());
				else
					printf("0x%x", data.param[i].ads);
			}
			break;
		case U32:
			printf("%d", data.param[i].sdword);
			break;
		case I32:
			printf("%d", data.param[i].dword);
			break;
		case U16:
			printf("%d", data.param[i].word);
			break;
		case I16:
			printf("%d", (short)data.param[i].sword);
			break;
		case COMMAND:
			break;
		case U8:
			printf("%d", data.param[i].byte);
			break;
		case I8:
			printf("%d", (char)data.param[i].sbyte);
			break;
		case STRING:
			printf("\"%s\"", data.param[i].stringdata);
			break;
		}
		if (i != mx && datatype != COMMAND) {
				printf(", ");
		}
	}
	};

bool DumpEDT(const char* name) {
	bool success = true;
	size_t size = 0;
	if (char* data = (char*)GetDataFromFile(name, size)) {
		EDTHeader header;
		memcpy(&header, data, sizeof(EDTHeader));
		ParseHeader((uint8_t*)data, header, size);
		free(data);
	}

	return success;
}

bool ParseHeader(uint8_t* data, const EDTHeader& header, size_t size) {
//	auto t = std::vector<ECL_DATATYPE>(0, );
	auto test = g_InstructionSize[MUSIC];
	u8* pBuf = data; 
	data += sizeof(EDTHeader);
	ProcData pdata;
	pdata.name = "main";
	pdata.type = PROC_SET;
	g_ProcData.insert({ header.init_enm, std::move(pdata)});

	size_t idx = sizeof(EDTHeader);

	if (header.tex_init == sizeof(EDTHeader)) {
		ParseTexInit(pBuf + header.tex_init, idx);
		ParseData(pBuf, size, idx, header.tex_init);
	}
	else {
		ParseData(pBuf, size, idx, header.tex_init);
		ParseTexInit(pBuf + header.tex_init, idx);
		ParseData(pBuf, size, idx, header.tex_init);
	}
	return true;
}

bool ParseData(uint8_t* data, size_t size, size_t& ind, int tex_init) {
	u8* pBuf = data + ind;
	int idx = ind;
	g_InstructionData.clear();
	if (idx >= size) return true;
	while (idx < size) {
		u8 cmd = *pBuf;
		if (cmd < 0x30) cmd = *(pBuf + 1);
		if (g_InstructionSize.find((ECL_INSTRUCTION)cmd) == g_InstructionSize.end()) {
			printf("unknown ins: 0x%x at 0x%x\n", cmd, idx);
			__debugbreak();
		}
		auto a = processData((ECL_INSTRUCTION)cmd, pBuf, idx);
		g_InstructionData.emplace_back(a);
		if (idx == tex_init) break;
	}
	PrintEDT();
	ind = idx;
	return true;
}

bool ParseTexInit(unsigned char* data, size_t& ind) {
	int rc_cnt = 0;
	int hb_cnt = 0;
	int anm_cnt = 0;
	printf("TexInit tex_init {\n");
	size_t idx = ind;
	bool is_process = true;
	while (is_process) {
		uint8_t cmd = *(char*)data;
		data++;
		idx++;
		char buf[256] = "";
		printf("\t");
		switch (cmd) {
		case 0x80:
			printf("TEXINITEND\n}\n");
			is_process = false;
			break;
		case 0x81: //Load texture
			for (int i = 0; *data; i++) {
				buf[i] = *data;
				data++;
				idx++;
			}
			data++;
			idx++;
			printf("LOAD \"%s\"", buf);
			break;
		case 0x82:
			{
				rect rc;
				memcpy(&rc, data, sizeof(rc));
				data += sizeof(rc);
				idx += sizeof(rc);
				printf("RECT %d, %d, %d, %d", rc.x0, rc.y0, rc.x1, rc.y1);
				rc_cnt++;
			}
			break;
		case 0x83:
		case 0x84:
			printf("TICMD_%x", cmd);
			break;
		case 0x85:
			{
				short x = *(uint16_t*)data;
				data += 2;
				idx += 2;
				short y = *(uint16_t*)data;
				data += 2;
				idx += 2;
				printf("ANIME %d, %d, ", (short)x, (short)y);
				anm_cnt++;
				while (*data != 0xff) {
					printf("%d", *data);
					data++;
					idx++;
					if (*data != 0xff) {
						printf(", ");
					}
				}
				data++;
				idx++;
			}
			break;
		case 0x90:
		case 0x91:
		case 0x92:
		case 0x93:
			printf("TICMD_%x", cmd);
			break;
		case 0xa0:
			{
				rect rc;
				memcpy(&rc, data, sizeof(rc));
				data += sizeof(rc);
				idx += sizeof(rc);
				printf("HITRECT %d, %d, %d, %d", (short)rc.x0, (short)rc.y0, rc.x1, rc.y1);
				hb_cnt++;
			}
			break;
		case 0xa1:
			{
				short w = *(short*)data;
				data += 2;
				idx += 2;
				short h = *(short*)data;
				data += 2;
				idx += 2;
				printf("HITRECT2 %d, %d", w, h);
				hb_cnt++;
			}
		}
		printf("\n");
	}
	ind = idx;
	return true;
}

bool PrintEDT() {
	address_map::iterator it = g_ProcData.begin();
	address_map::iterator it2 = g_LabelData.begin();
	int proc_cnt = 0;
	for (auto& i : g_InstructionData) {
		if (it != g_ProcData.end() && it->first <= i.add) {
			const char* proc_name = "Proc";
			switch (it->second.type) {
			case PROC_EXANM: proc_name = "ExAnm"; break;
			case PROC_BOSS: proc_name = "Boss"; break;
			case PROC_ENEMY: proc_name = "Enemy"; break;
			case PROC_ATK: proc_name = "TSet"; break;
			case PROC_SET: proc_name = "Set"; break;
			}

			if (proc_cnt > 0)
				printf("}\n\n");
			printf("%s %s {\n", proc_name, it->second.name.c_str());
			it++;
			proc_cnt++;
		}
		if (it2 != g_LabelData.end() && it2->first <= i.add) {
			printf("%s:\n",it2->second.name.c_str());
			it2++;
		}
		printf("\t"); printinst(i); printf("\n");
	}
	printf("}\n\n");
	return true;
}
