#ifndef UTYL_INCLUDED
#define UTYL_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <map>
#include <vector>
#include <string>

struct EDTHeader {
	int init_enm;
	int tex_init;
};

struct rect {
	uint16_t x0;
	uint16_t y0;
	uint16_t x1;
	uint16_t y1;
};

inline void* _fastcall GetDataFromFile(const char* name, size_t& ret) {
	if (FILE* fp = fopen(name, "rb")) {
		size_t size = -1;
		void* data = nullptr;
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		rewind(fp);

		data = malloc(size);
		fread(data, size, 1, fp);
		ret = size;
		fclose(fp);
		return data;
	}
	ret = -1;
	return nullptr;
}


typedef unsigned char command;
typedef signed int address;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef char* string;

enum ECL_DATATYPE {
	NONE = -1,
	COMMAND,
	ADDRESS,
	U8,
	U16,
	U32,
	I8,
	I16,
	I32,
	STRING
};

struct ECLParamData {
	ECL_DATATYPE datatype;
	//Diferent datatypes
	union {
		command cmd;
		address ads;
		u8 byte;
		u16 word;
		u32 dword;
		u8 sbyte;
		u16 sword;
		u32 sdword;
		string stringdata;
	};
	int adv = 0;
};


struct ECLInstructionDefine {
	const char* name;
	std::vector<ECL_DATATYPE> paramdatatype;
};

enum TEXINIT_INSTRUCTION : u8 {
	TEXINITEND = 0x80,
	LOAD,
	RECT,
	TICMD_83,
	TICMD_84,
	ANIME,
	
	TICMD_90 = 0x90,
	TICMD_91,
	TICMD_92,
	TICMD_93,

	HITRECT = 0xA0,
	HITRECT2,
};

enum ECL_INSTRUCTION : u8 {
	PARENT = 0x0, //This doesn't really exist, is just for converting

	EXIT = 0x30,

	// 標準命令(必ずサポートしている必要がある) //
	PUSHR,	// [後] スタックにレジスタの内容を積む
	POPR,	// [後] レジスタにスタックの内容をコピー
	MOVC,	// [中] レジスタに定数の代入を行う(MOVC REG CONST)

	PUSHC,	// [前] 定数をスタックにＰＵＳＨする
	TJMP,	// [前] POP して真ならばジャンプする
	FJMP,	// [前] POP して偽ならばジャンプする
	JMP,	// [前] 無条件ジャンプ
	OJMP,	// [前] POP して真ならPUSH(TRUE),  JMP
	AJMP,	// [前] POP して偽ならPUSH(FALSE), JMP
	LPOP,	// [前] 演算スタックトップ->JMP スタックトップ
	LJMP,	// [前] ０ならジャンプ、真ならデクリメント

	LCLEAR, //Maybe is ret

	ADD,	// [前] Push(Pop(1) + Pop(0))
	SUB,	// [前] Push(Pop(1) - Pop(0))
	MUL,	// [前] Push(Pop(1) * Pop(0))
	DIV,	// [前] Push(Pop(1) / Pop(0))
	MOD,	// [前] Push(Pop(1) % Pop(0))
	NEG,	// [前] Push(-Pop(0))

	SINL,	// [前] Push(sinl(Pop(1), Pop(0))
	COSL,	// [前] Push(cosl(Pop(1), Pop(0))
	RND ,	// [前] Push(rnd() % Pop(0))
	ATAN,	// [前] Push(atan(Pop(1), Pop(0))

	EQUAL,	// [前] Push(Pop(1) == Pop(0))
	NOTEQ,	// [前] Push(Pop(1) != Pop(0))
	ABOVE,	// [前] Push(Pop(1) >  Pop(0))
	LESS,	// [前] Push(Pop(1) <  Pop(0))
	ABOVEEQ,	// [前] Push(Pop(1) >= Pop(0))
	LESSEQ,	// [前] Push(Pop(1) <= Pop(0))

	MAX,	// [前] Push( max(Pop(0), Pop(1)) )
	MIN,	// [前] Push( min(Pop(0), Pop(1)) )

	NOT,
	SHL,
	SHR,
	BITAND,
	BITOR,
	BITNOT,
	ABS,
	CMD_56,
	RGBMAKE,
	RGBAMAKE, 
	CMD_59,
	CMD_5A,
	CMD_5B,

	CALL = 0x60,
	RET,
	ENEMY,
	ENEMYCIRCLE,
	ENEMY2,
	ENEMY3,
	ATK,

	BOSS,
	CMD_68,
	ENEMYEX,
	CMD_6A,
	CMD_6B,
	CMD_6C,

	CMD_70 = 0x70,
	CHILD,
	CMD_72,
	CMD_73,
	CMD_74,
	CMD_75,

	NOP = 0x80,
	NOPMOV,
	CMD_82,
	CMD_83,
	CMD_84,
	CMD_85,
	CMD_86,
	ACC,
	CMD_88,
	CMD_89,
	CMD_8A,
	CMD_8B,
	CMD_8C,
	CMD_8D,
	ROL,

	CMD_90 = 0x90,
	ANM,
	BEGINCALLBACK,
	ENDCALLBACK,
	CMD_94,
	TAMA,
	CMD_96,
	CMD_97,
	CMD_98,
	CMD_99,
	CMD_9A,
	CMD_9B,
	CMD_9C,
	CMD_9D,
	CMD_9E,
	CMD_9F,

	CMD_A0,
	CMD_A1,
	CMD_A2,
	CMD_A3,

	DEGS = 0xb0,
	CMD_B1,
	DEGSDEL,
	CMD_B3,
	HPINT,

	CMD_C0 = 0xc0,

	PSE = 0xd0,
	MUSIC,
	WARNING,
	FOG,
	ETCLEAR,
	CMD_D5,
	SPECIALE,
	CMD_D7,
	WAITBOSS,
	EFC,
	ANMTASK,
	TALKSTART,
	TALKEND,
	NEXTPAGE,
	NEWLINE,
	NEXTSTAGE,

	CMD_E0,
	FADEOUT,
	CMD_E2,
	CMD_E3,
	CMD_E4,

	CMD_F0 = 0xf0,
	CMD_F1,
};

struct ECLInstructionData {
	u8 cmd;
	address add;
	int cnt = 1;
	std::vector<ECLParamData> param;
};

enum PROC_TYPE {
	PROC_LABEL,
	PROC_DEF,
	PROC_TEXINIT,
	PROC_ENEMY,
	PROC_BOSS,
	PROC_ATK,
	PROC_EXANM,
	PROC_SET,
};

struct ProcData {
	PROC_TYPE type;
	std::string name;
};

struct ProcDataEx {
	PROC_TYPE type;
	address name;
};


#define ECL_INS(x, args) {x, {#x, std::vector<ECL_DATATYPE>args} }

static std::map<TEXINIT_INSTRUCTION, ECLInstructionDefine> g_TexInitSize = {
	ECL_INS(TEXINITEND, ({COMMAND})),
	ECL_INS(LOAD, ({COMMAND, STRING})),
	ECL_INS(RECT, ({COMMAND, I16, I16, I16, I16})),
	ECL_INS(TICMD_83, ({COMMAND})),
	ECL_INS(TICMD_84, ({COMMAND})),
	ECL_INS(ANIME, ({COMMAND, I16, I16})),

	ECL_INS(TICMD_90, ({COMMAND})),
	ECL_INS(TICMD_91, ({COMMAND})),
	ECL_INS(TICMD_92, ({COMMAND})),
	ECL_INS(TICMD_93, ({COMMAND})),

	ECL_INS(HITRECT, ({COMMAND, I16, I16, I16, I16})),
	ECL_INS(HITRECT2, ({COMMAND, I16, I16})),
};

static std::map<ECL_INSTRUCTION, ECLInstructionDefine> g_InstructionSize = {
	ECL_INS(EXIT, ({COMMAND})),

	ECL_INS(PUSHR, ({U8, COMMAND })),
	ECL_INS(POPR,  ({U8, COMMAND})),
	ECL_INS(MOVC,  ({U8, COMMAND, I32})),

	ECL_INS(PUSHC,		({COMMAND, I32})),
	ECL_INS(TJMP,		({COMMAND, ADDRESS})),
	ECL_INS(FJMP,		({COMMAND, ADDRESS})),
	ECL_INS(JMP,		({COMMAND, ADDRESS})),
	ECL_INS(OJMP,		({COMMAND, ADDRESS})),
	ECL_INS(AJMP,		({COMMAND, ADDRESS})),
	ECL_INS(LPOP,		({COMMAND})),
	ECL_INS(LJMP,		({COMMAND, ADDRESS})),
	ECL_INS(LCLEAR,		({COMMAND})),

	ECL_INS(ADD,		({COMMAND})),
	ECL_INS(SUB,		({COMMAND})),
	ECL_INS(MUL,		({COMMAND})),
	ECL_INS(DIV,		({COMMAND})),
	ECL_INS(MOD,		({COMMAND})),
	ECL_INS(NEG,		({COMMAND})),

	ECL_INS(SINL,		({COMMAND})),
	ECL_INS(COSL,		({COMMAND})),
	ECL_INS(RND,		({COMMAND})),
	ECL_INS(ATAN,		({COMMAND})),

	ECL_INS(EQUAL,		({COMMAND})),
	ECL_INS(NOTEQ,		({COMMAND})),
	ECL_INS(ABOVE,		({COMMAND})),
	ECL_INS(LESS,		({COMMAND})),
	ECL_INS(ABOVEEQ,	({COMMAND})),
	ECL_INS(LESSEQ,		({COMMAND})),

	ECL_INS(MAX,		({COMMAND})),
	ECL_INS(MIN,		({COMMAND})),

	ECL_INS(NOT,		({COMMAND})),
	ECL_INS(SHL,		({COMMAND})),
	ECL_INS(SHR,		({COMMAND})),
	ECL_INS(BITAND,		({COMMAND})),
	ECL_INS(BITOR,		({COMMAND})),
	ECL_INS(BITNOT,		({COMMAND})),
	ECL_INS(ABS,		({COMMAND})),

	ECL_INS(CMD_56, ({COMMAND, U8})),

	ECL_INS(RGBMAKE, ({COMMAND})),
	ECL_INS(RGBAMAKE, ({COMMAND})),

	ECL_INS(CMD_59, ({COMMAND})),
	ECL_INS(CMD_5A, ({COMMAND})),
	ECL_INS(CMD_5B, ({COMMAND})),

	ECL_INS(CALL, ({COMMAND, ADDRESS})),
	ECL_INS(RET, ({COMMAND, U8})),
	ECL_INS(ENEMY, ({COMMAND, I16, I16, ADDRESS})),
	ECL_INS(ENEMYCIRCLE, ({COMMAND, I8, I16, ADDRESS})),
	ECL_INS(ENEMY2, ({COMMAND, I16, I16, ADDRESS})),
	ECL_INS(ENEMY3, ({COMMAND, I8, I16, ADDRESS})),
	ECL_INS(ATK, ({COMMAND, I16, I16, ADDRESS})),
	ECL_INS(BOSS, ({COMMAND, I16, I16, I32, ADDRESS})),
	ECL_INS(CMD_68, ({COMMAND, I16, I16, I32, ADDRESS})),
	ECL_INS(ENEMYEX, ({COMMAND, ADDRESS})),
	ECL_INS(CMD_6A, ({COMMAND, ADDRESS})),
	ECL_INS(CMD_6B, ({COMMAND, ADDRESS})),
	ECL_INS(CMD_6C, ({COMMAND, ADDRESS})),

	ECL_INS(CMD_70, ({COMMAND, U16, U16, U8, ADDRESS})),
	ECL_INS(CHILD, ({COMMAND, I8, U16, U8, ADDRESS})),
	ECL_INS(CMD_72, ({COMMAND, U16, U16, U8, ADDRESS})),
	ECL_INS(CMD_73, ({COMMAND, U8, U16, U8, ADDRESS})),
	ECL_INS(CMD_74, ({COMMAND, U8})),
	ECL_INS(CMD_75, ({COMMAND})),

	ECL_INS(NOP,  ({COMMAND, U16})),

	ECL_INS(NOPMOV,  ({COMMAND, U16})), //Move using current speed and angle for x frames
	ECL_INS(CMD_82,  ({COMMAND, U32, U16})), //?
	ECL_INS(CMD_83,  ({COMMAND, U8, U16})),//Move with current speed, using x direction for y frames
	ECL_INS(CMD_84,  ({COMMAND, U8, U32, U16})), //Move adding x angle, at y speed for z frames
	ECL_INS(CMD_85,  ({COMMAND, U16})), //Move with current speed, accelerating by current acceleration for x frames
	ECL_INS(ACC,  ({COMMAND, U32, U32, U16})), //Move with current angle, x as initial speed, accelerating by y for z frames
	ECL_INS(CMD_88,  ({COMMAND, U8, U32, U32, U16})),
	ECL_INS(CMD_89,  ({COMMAND, U16})),
	ECL_INS(CMD_8B,  ({COMMAND, U32, U16})),
	ECL_INS(CMD_8C,  ({COMMAND, U8, U32, U16})),
	ECL_INS(CMD_8D,  ({COMMAND, U16})),
	ECL_INS(ROL,  ({COMMAND, I8, U16})), //Move with current speed, adding x angle for y frames

	ECL_INS(ANM,  ({COMMAND, U8})),
	ECL_INS(BEGINCALLBACK,  ({COMMAND, ADDRESS})),
	ECL_INS(ENDCALLBACK,  ({COMMAND, ADDRESS})),
	ECL_INS(CMD_94,  ({COMMAND})),
	ECL_INS(TAMA,  ({COMMAND})),
	ECL_INS(CMD_96,  ({COMMAND})),
	ECL_INS(CMD_97,  ({COMMAND})),
	ECL_INS(CMD_98,  ({COMMAND})),
	ECL_INS(CMD_99,  ({COMMAND, U16})),

	ECL_INS(CMD_9B,  ({COMMAND, U16, U16})),
	ECL_INS(CMD_9C,  ({COMMAND, U16})),
	ECL_INS(CMD_9D,  ({COMMAND})),
	ECL_INS(CMD_9E,  ({COMMAND})),
	ECL_INS(CMD_9F,  ({COMMAND})),

	ECL_INS(CMD_A0,  ({COMMAND, U16, U16, U16})),
	ECL_INS(CMD_A1,  ({COMMAND, U16, U16, U16})), //is it something like length?
	ECL_INS(CMD_A2,  ({COMMAND})), //is it something like length?
	ECL_INS(CMD_A3,  ({COMMAND})), //is it something like length?

	ECL_INS(DEGS,  ({COMMAND})),
	ECL_INS(CMD_B1,  ({COMMAND})),
	ECL_INS(DEGSDEL,  ({COMMAND, I8})), //Degs but uses x angle to get to you
	ECL_INS(CMD_B3,  ({COMMAND, U16, U16, U16})),
	ECL_INS(HPINT,  ({COMMAND, ADDRESS})),

	ECL_INS(CMD_C0,  ({COMMAND})),

	ECL_INS(PSE,  ({COMMAND, U8})),
	ECL_INS(MUSIC,  ({COMMAND, U8})),
	ECL_INS(WARNING,  ({COMMAND, U8})),
	ECL_INS(FOG,  ({COMMAND, U8, U8, U8, U16})),
	ECL_INS(ETCLEAR,  ({COMMAND})),
	ECL_INS(CMD_D5,  ({COMMAND, I32})),
	ECL_INS(SPECIALE,  ({COMMAND, STRING, U16})),
	ECL_INS(CMD_D7,  ({COMMAND})),
	ECL_INS(WAITBOSS,  ({COMMAND})),
	ECL_INS(EFC,  ({COMMAND, U8})),
	ECL_INS(ANMTASK,  ({COMMAND, I16, I16, ADDRESS})),
	ECL_INS(TALKSTART,  ({COMMAND})),
	ECL_INS(TALKEND,  ({COMMAND})),
	ECL_INS(NEXTPAGE,  ({COMMAND})),
	ECL_INS(NEWLINE,  ({COMMAND, STRING})),
	ECL_INS(NEXTSTAGE,  ({COMMAND})),

	ECL_INS(CMD_E0,  ({COMMAND})),
	ECL_INS(FADEOUT,  ({COMMAND})),
	ECL_INS(CMD_E2,  ({COMMAND})),
	ECL_INS(CMD_E3,  ({COMMAND, U8})),
	ECL_INS(CMD_E4,  ({COMMAND, STRING, U16, U8, U8, U8})),

	//ECL_INS(CMD_F0,  ({COMMAND, STRING, U16, U8, U8, U8})),
};

//I'm not filling this bullshit by hand
static std::map<const std::string, ECL_INSTRUCTION> g_Str2Cmd;
static std::map<const std::string, TEXINIT_INSTRUCTION> g_Str2TICmd;

inline void InitializeString2Command() {
	for (auto& i : g_InstructionSize) {
		g_Str2Cmd.insert({ i.second.name, i.first });
	}
	for (auto& i : g_TexInitSize) {
		g_Str2TICmd.insert({ i.second.name, i.first });
	}
}

#endif // !EDT_DUMP_INCLUDED
