#ifndef SCLCOMPILE_INCLUDED
#define SCLCOMPILE_INCLUDED
#include "utyl.hpp"
#include <unordered_map>

//I have to apply certain restrictions for commands...
enum TOKEN_KIND {
	TOKEN_IGNORE, //This is ignored by the compiler
	TOKEN_KEYWORD, //Words like PROC, TEXINIT, TSET
	TOKEN_COMMAND, //Words for commans, like SET, CALL, ADD, etc
	TOKEN_IDENTIFIER, //the name of a procedure, label or define
	TOKEN_NUMBER, // Just numbers... Like 5,3,2,1
	TOKEN_STRING, // Words between quotes like "TEX_YUUKA"
	TOKEN_COMMA, // the comma for separating things
	TOKEN_DOTS, // the comma for separating things
	TOKEN_ARITHS, //Arithmetic symbols, like + - / *
	TOKEN_ARITHF, //Arithmetic function names like sinl, cosl, rnd, etc
	TOKEN_REGISTER, // A register like x, y, hp, score, etc
	TOKEN_BEGINPARENTHESIS, // (
	TOKEN_ENDPARENTHESIS, // )
	TOKEN_BEGINBLOCK, // {
	TOKEN_ENDBLOCK, // }

	//Extra tokens to make some stuff faster to process, i think
	TOKEN_PROC, //This holds the name of the procedure
	TOKEN_ENDPROC, //This tells to stop reading a subroutine
	TOKEN_LABEL, //the name of a label
	TOKEN_INCOUNT, //number of instructions to read
	TOKEN_CONST, //has the name of a const
	TOKEN_ARITHBEG, //has the name of a const
	TOKEN_ARITHEND, //has the name of a const

	//TOKEN_COMMENT, // ; as comment, like in a real assembler (Never touched one lol)
};

enum ARITHMETIC_SYMBOL {
	AS_ASSIGN, // var = ...

	AS_ADD, //var + var
	AS_SUB, //var - var
	AS_MUL, //var * var
	AS_DIV, //var / var
	AS_MOD, //var % var

	AS_INC, //var++
	AS_DEC, //var--

	AS_ADDA, //var += var
	AS_SUBA, //var -= var
	AS_MULA, //var *= var
	AS_DIVA, //var /= var
	AS_MODA, //var %= var

	AS_EQU, //var == var
	AS_NOTEQU, //var != var
	AS_LESS, //var < var
	AS_LESSEQ, //var <= var
	AS_GREAT, //var > var
	AS_GREATEQ, //var >= var

	AS_NOT, // !var
	AS_SHL, // var << var
	AS_SHR, // var >> var
	AS_BITAND, // var & var
	AS_BITOR, // var | var
	AS_BITNOT, // ~var

	AS_AND, //var && var
	AS_OR, //var || var

};

enum ARITHMETIC_FUNCTION {
	AF_MAX, //max(var, var)
	AF_MIN, //min(var, var)
	AF_RND, //rnd(var)
	AF_ATAN, //atan(var, var)
	AF_COSL, //cosl(var, var)
	AF_SINL, //sinl(var, var)
	AF_ABS, //abs(var)
};

enum KEYWORD_KIND {
	KEY_GLOBAL,
	KEY_PROC,
	KEY_TEXINIT,
	KEY_ENEMY,
	KEY_BOSS,
	KEY_TSET,
	KEY_EXANM,
	KEY_SET,
	KEY_ENDPROC,

	KEY_IF,
	KEY_WHILE,
	KEY_LOOP,

	KEY_CONST,
	KEY_INCLUDE,
};

//For the tokenizer...
struct Token {
	TOKEN_KIND kind;
	size_t line;
	std::string pStr;
	int number = 0;
	size_t advance;
	const char* source;
};

struct OutputData {
	void* pData;
	size_t size;
};

struct ECLInstructionDataEx : public ECLInstructionData {
	size_t line = -1;
	const char* source = "";
};

struct ProcDataEx2 {
	u8 type;
	address ads;
	std::vector<ECLInstructionDataEx> cmd_data;
	std::map<const std::string, address> label_data;
};

struct SourceInfo {
	const char* pWhereSource;
	const char* pWhatSource;
	size_t line;
};

//Registers used on normal and enemy procedures
static std::map<const std::string, int> g_EnemyRegs = {
	{"x", 0},
	{"y", 1},
	{"vel", 2},
	{"hp", 3},
	{"count", 4},
	{"score", 5},
	{"dir", 6},
	{"flag", 7},
	{"xmid", 18},
	{"acc", 22},
	{"diff", 37},
};

//Registers used on TCL (tama(bullet) control language)
static std::map<const std::string, int> g_AtkRegs = {
	{"x", 0},
	{"y", 1},
};

//Registers used on ExAnm
static std::map<const std::string, int> g_ExAnmRegs = {
	{"x", 0},
	{"y", 1},
};

static std::map<const std::string, ARITHMETIC_FUNCTION> g_ArithFunc = {
	{"cosl",AF_COSL},
	{"sinl",AF_SINL},
	{"atan",AF_ATAN},
	{"min",AF_MIN},
	{"max",AF_MAX},
	{"rnd",AF_RND},
};

static std::map<const std::string, KEYWORD_KIND> g_Keystr2Tok = {
	//Procedure types
	{"Proc", KEY_PROC},
	{"TexInit", KEY_TEXINIT},
	{"Enemy", KEY_ENEMY},
	{"Boss", KEY_BOSS},
	{"TSet", KEY_TSET},
	{"ExAnm", KEY_EXANM},
	{"Set", KEY_SET},

	//Conditions and loops (lol)
	{"if", KEY_IF},
	{"while", KEY_WHILE},
	{"loop", KEY_LOOP},

	//Other Keywords
	{"ENDPROC", KEY_ENDPROC},
	{"const", KEY_CONST},
	{"include", KEY_INCLUDE}
};

typedef std::vector<u8> valid_instruction_set;
typedef std::vector<ECLInstructionDataEx> ins_data;
typedef std::unordered_map<std::string, ProcDataEx2> address_map_ex;
typedef std::map<const std::string, Token> constant_map;

static valid_instruction_set g_ControlFlow = {
	//Control flow
	TJMP,
	FJMP,
	JMP,
	OJMP,
	AJMP,
	LPOP,
	LJMP,

	//Arithmetic
	PUSHR,
	POPR,
	MOVC,
	PUSHC,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	NEG,
	SINL,
	COSL,
	RND,
	ATAN,
	MAX,
	MIN,
	EQUAL,
	NOTEQ,
	ABOVE,
	LESS,
	ABOVEEQ,
	LESSEQ
};

static valid_instruction_set g_TexInitCheck = {
	TEXINITEND,
	LOAD,
	RECT,
	TICMD_83,
	TICMD_84,
	ANIME,
	TICMD_90,
	TICMD_91,
	TICMD_92,
	TICMD_93,

	HITRECT,
	HITRECT2,
};

//Also add Arithmetic
static valid_instruction_set g_EnemyCheck = {

};

//Add also flow control
static valid_instruction_set g_TSetCheck = {

};

//Add also flow control
static valid_instruction_set g_ExAnmCheck = {

};

static valid_instruction_set g_SetCheck = {

};

class EDTCompiler {
public:
	//Basic constructor for the EDT Compiler
	EDTCompiler();

	//Main function
	bool CompileEDT(const char* name, const char* output);

	//Delete all subroutine data and clean all
	void Reset();

private: //Private funcs

	//Tokenize everything
	bool TokenizeInput(
		const char* pSourceFile,
		std::vector<Token>& processed_data
	);

	//Verify the syntax and try to make an easier to understand array to process data
	bool VerifySyntaxAndParse(
		std::vector<Token>& raw_data
	);

	//Calculate addresses and parse command data
	bool CalculateAddresses();

	//Fill the corresponding addresses
	bool PopulateAddresses();

	//Process header data and set addresses
	bool ProcessHeader();

	//Finally, join both data
	void* JoinData();


	//Include a source file
	bool IncludeSourceFile(
		const char* pSourceFile,
		std::vector<Token>& source,
		size_t index
	);

private: //Private vars
	//We need a set of rules to separate tokens


	std::vector<Token> m_ProcessedTokens; //Tokens after being processed and ready to pass to calculate and populate addresses

	//Join both to form the final file:
	EDTHeader m_Header; //Header
	address_map_ex m_ProcData; //Procedure data (label, comands)
	size_t m_FileSize; //Final output file size

	//Defined constants and local constants
	constant_map m_GlobalConstMap;
	constant_map m_LocalConstMap;

	//Using this to order all procedures according to the file
	std::vector<std::string> m_ProcNames;
};



//Parse common stuff for comand data
bool ProcessCommonCmdData(
	std::vector<Token>& tokens,
	std::vector<Token>* pProcessedData,
	constant_map& const_map,
	ECL_INSTRUCTION cmd,
	ECL_INSTRUCTION& last_cmd,
	KEYWORD_KIND proc_type,
	size_t& idx
);

//Specific for TexInit
bool ProcessTexInitData(
	std::vector<Token>& tokens,
	std::vector<Token>* pProcessedData,
	constant_map& const_map,
	TEXINIT_INSTRUCTION cmd,
	TEXINIT_INSTRUCTION& last_cmd,
	size_t& idx
);

//Process if statemets, while loops and normal loops
bool ProcessConditionOrLoopBlock(
	std::vector<Token>& tokens,
	std::vector<Token>* pProcessedData,
	constant_map& const_map,
	KEYWORD_KIND proc_kind,
	KEYWORD_KIND type,
	bool (*BlockProcessFn)(std::vector<Token>&, std::vector<Token>*, constant_map&, size_t&, int),
	size_t& idx
);

//Process different kinds of blocks
bool ProcessTexInitBlock(
	std::vector<Token>& tokens,
	std::vector<Token>* pProcessedData,
	constant_map& const_map,
	size_t& idx
);


bool ProcessEnemyBlock(
	std::vector<Token>& tokens,
	std::vector<Token>* pProcessedData,
	constant_map& const_map,
	size_t& idx,
	int exit_type
);

bool ProcessTSetBlock(
	std::vector<Token>& tokens,
	std::vector<Token>* pProcessedData,
	constant_map& const_map,
	size_t& idx,
	int exit_type
);

bool ProcessExAnmBlock(
	std::vector<Token>& tokens,
	std::vector<Token>* pProcessedData,
	constant_map& const_map,
	size_t& idx,
	int exit_type
);

bool ProcessSetBlock(
	std::vector<Token>& tokens,
	std::vector<Token>* pProcessedData,
	constant_map& const_map,
	size_t& idx
);

#endif