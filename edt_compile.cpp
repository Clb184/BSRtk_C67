#include "edt_compile.hpp"
#include "Calc2Token.hpp"

static int g_IfCnt = 0;
static int g_WhileCnt = 0;
static int g_LoopCnt = 0;

inline bool IsValidKeyword(const std::string& word, int* pint) {
    if (g_Keystr2Tok.find(word) != g_Keystr2Tok.end()) {
        *pint = g_Keystr2Tok[word];
        return true;
    }
    return false;
}

inline bool IsValidCommand(const std::string& word, int* pint) {
    if (g_Str2Cmd.find(word) != g_Str2Cmd.end()) {
        *pint = g_Str2Cmd[word];
        return true;
    }
    else if (g_Str2TICmd.find(word) != g_Str2TICmd.end()) {
        *pint = g_Str2TICmd[word];
        return true;
    }
    return false;
}
/*
+ - * / % operation
+= -= *= /= %= operation and assign
== <= >= < > !=
&&-> min ||-> max

(stuff) && stuff && stuff && stuff
(((1 1)min 1)min 0)min
1 1 min 0 min
1 0 min
0
false


if not (40 > 60){
    ...
       block
    ...
}

    40
    60
    great
    tjmp ifblock
    ...
        block
    ...
ifblock0e:
    
*/
constexpr inline bool IsValidBlockControl(char c) {
    return c == '{' || c == '}';
}

constexpr inline bool IsValidArithmeticControl(char c) {
    return c == '(' || c == ')';
}

constexpr inline bool IsValidArithmeticSymbol(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '=' || c == '>' || c == '<' || c == '&' || c == '|' || c == '!';
}

constexpr inline bool IsValidCharacter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == ',' || c == ';' || c == '\"' || c == '-' || c == ':'
        || IsValidArithmeticSymbol(c) || IsValidArithmeticControl(c) || IsValidBlockControl(c);
}

constexpr inline bool IsValidFirstIdentifier(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

constexpr inline bool IsValidIdentifier(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

constexpr inline bool IsValidFirstNumberCharacter(char c, bool& sign) {
    if (c == '-') sign = true;
    return (c >= '0' && c <= '9') || c == '-';
}

constexpr inline bool IsValidNumberCharacter(char c) {
    return (c >= '0' && c <= '9');
}

constexpr inline bool IsSeparator(char c) {
    return c == ',' || c == ' ' || c == '\t' || c == ';' || c == '\n' || c == ':' || c == 0x0d || IsValidArithmeticSymbol(c) || IsValidBlockControl(c) || IsValidArithmeticSymbol(c);
}

bool GetInteger(char* pData, size_t bufSize, size_t& buf_idx, int* pResult) {
    bool success = true;
    int ret = 0;
    while (buf_idx < bufSize) {
        char c = pData[buf_idx];
        if (IsValidNumberCharacter(c)) {
            ret = ret * 10 + (c - '0');
        }
        else if (IsSeparator(c)) break;
        else if(IsValidIdentifier(c)) {
            success = false;
            break;
        }
        else break;
        buf_idx++;
    }
    *pResult = ret;
    return success;
}

bool GetIdentifier(char* pData, size_t bufSize, size_t& buf_idx, std::string& result) {
    bool success = true;
    while (buf_idx < bufSize) {
        char c = pData[buf_idx];
        if (IsValidIdentifier(c)) {
            result.push_back(c);
        }
        else {
            break;
        }
        buf_idx++;
    }
    return success;
}

bool GetArithSign(char* pData, size_t bufSize, size_t& buf_idx, ARITHMETIC_SYMBOL& result) {
    bool success = true;
    std::string res = "";
    while (buf_idx < bufSize) {
        char c = pData[buf_idx];
        if (IsValidArithmeticSymbol(c)) {
            res.push_back(c);
        }
        else {
            break;
        }
        buf_idx++;
    }
    if (res.length() >= 1) {
        char base = res.at(0);
        switch(res.length()) {
        default: success = false;
        case 1:
            switch (base) {
            case '+': result = AS_ADD; break;
            case '-': result = AS_SUB; break;
            case '*': result = AS_MUL; break;
            case '/': result = AS_DIV; break;
            case '%': result = AS_MOD; break;

            case '<': result = AS_LESS; break;
            case '>': result = AS_GREAT; break;
            case '=': result = AS_ASSIGN; break;
            default: success = false; break;
            }
            break;
        case 2: {
            char next = res.at(1);
            if (next == base) {
                switch (next) {
                case '&': result = AS_AND; break;
                case '|': result = AS_OR; break;
                case '=': result = AS_EQU; break;
                default: success = false; break;
                }
            }
            else if (next == '=') {
                switch (base) {
                case '+': result = AS_ADDA; break;
                case '-': result = AS_SUBA; break;
                case '*': result = AS_MULA; break;
                case '/': result = AS_DIVA; break;
                case '%': result = AS_MODA; break;

                case '<': result = AS_LESSEQ; break;
                case '>': result = AS_GREATEQ; break;
                case '!': result = AS_NOTEQU; break;
                default: success = false; break;
                }
            }
            else success = false;
            }break;
        }
    }
    else
        success = false;
    return success;
}

EDTCompiler::EDTCompiler() :
    m_FileSize(0),
    m_Header({0,0})
{

}

bool EDTCompiler::CompileEDT(
    const char* Name,
    const char* OutputName
)
{
    //Something went wrong or not
    bool success = false;

    //First we need to separate everything, is not that relevant though
    std::vector<Token> tok_data;
    InitializeString2Command();
    if (
        TokenizeInput(Name, tok_data) &&
        VerifySyntaxAndParse(tok_data) &&
        CalculateAddresses() &&
        PopulateAddresses() &&
        ProcessHeader()
        )
    {

        void* pData = JoinData();
        if (FILE* out = fopen(OutputName, "wb")) {
            fwrite(pData, m_FileSize, 1, out);
            fclose(out);
            free(pData);
            success = true;
        }
    }
    return success;
}

bool EDTCompiler::TokenizeInput(
    const char* pSourceFile,
    std::vector<Token>& output_raw
) 
{
    bool success = true;
    bool on_open = false;
    FILE* fp;
    try {
        fp = fopen(pSourceFile, "rb");
        on_open = fp != nullptr;
    }
    catch (...) {
        on_open = false;
    }

    if (on_open) {
        size_t size;
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        rewind(fp);
        char* pSourceData = (char*)malloc(size);
        fread(pSourceData, size, 1, fp);
        fclose(fp);

        size_t idx = 0;
        size_t line = 1;
        bool is_comment = false;
        while (idx < size) {
            char c = pSourceData[idx];
            try {
                Token tok;
                tok.kind = TOKEN_IGNORE;
                tok.line = line;
                tok.source = pSourceFile;
                if (c == ' ' || c == '\t' || c == 0x0d) {
                    idx++;
                }
                else if (c == '\n') {
                    idx++;
                    line++;
                }
                else if (IsValidCharacter(c)) {
                    bool sign = false;
                    if (c == ',') {
                        tok.kind = TOKEN_COMMA;
                        idx++;
                    }
                    else if (c == ':') {
                        tok.kind = TOKEN_DOTS;
                        idx++;
                    }
                    else if (c == ';') {
                        while (idx < size && c != '\n') {
                            idx++;
                            c = pSourceData[idx];
                        }
                    }
                    //Get string, if no quote is found before new line, throw an error
                    else if (c == '\"') {
                        bool find_quote = false;
                        idx++;
                        if (idx < size) {
                            c = pSourceData[idx];
                            while ((idx < size && c != '\"')) {
                                tok.pStr.push_back(c);
                                idx++;
                                c = pSourceData[idx];
                            }
                            if (c == '\"') {
                                find_quote = true;
                                tok.kind = TOKEN_STRING;
                                idx++;
                            }
                        }
                        if (!find_quote) throw 2;
                    }
                    else if (IsValidNumberCharacter(c)) {
                        if (GetInteger(pSourceData, size, idx, &tok.number)) {
                            tok.kind = TOKEN_NUMBER;
                        }
                        else throw 1;
                    }
                    else if (IsValidFirstIdentifier(c)) {
                        if (GetIdentifier(pSourceData, size, idx, tok.pStr)) {
                            tok.kind =
                                (IsValidKeyword(tok.pStr, &tok.number)) ? TOKEN_KEYWORD :
                                (IsValidCommand(tok.pStr, &tok.number)) ? TOKEN_COMMAND :
                                TOKEN_IDENTIFIER;
                            if (g_ArithFunc.find(tok.pStr) != g_ArithFunc.end()) {
                                tok.kind = TOKEN_ARITHF;
                                tok.number = g_ArithFunc[tok.pStr];
                            }
                        }
                        else throw 0;
                    }
                    else if (IsValidArithmeticSymbol(c)) {
                        ARITHMETIC_SYMBOL sym;
                        if (GetArithSign(pSourceData, size, idx, sym)) {
                            tok.kind = TOKEN_ARITHS;
                            tok.number = sym;
                        }
                    }
                    else if (IsValidArithmeticControl(c)) {
                        tok.kind = (c == '(') ? TOKEN_BEGINPARENTHESIS : TOKEN_ENDPARENTHESIS;
                        idx++;
                    }
                    else if (IsValidBlockControl(c)) {
                        tok.kind = (c == '{') ? TOKEN_BEGINBLOCK : TOKEN_ENDBLOCK;
                        idx++;
                    }
                    else throw c;

                }
                else throw c;
                if (tok.kind != TOKEN_IGNORE)
                    output_raw.emplace_back(tok);
            }
            //Handle errors
            catch (int i) {
                const char* err = "";
                switch (i) {
                case 0: err = "Invalid identifier"; break;
                case 1: err = "Invalid combination with integer"; break;
                case 2: err = "String quote not found"; break;
                }
                printf("%s > Error line %d: %s\n", pSourceFile, line, err);
                success = false;
                break;
            }
            catch (char c) {
                printf("%s > Error line %d: Unrecognized character %c\n", pSourceFile, line, c);
                success = false;
                break;
            }
            catch (...) {
                printf("%s > Error line %d: Unspecified error\n", pSourceFile, line);
                success = false;
                break;
            }
        }

        free(pSourceData);
    }
    else {
        printf("Can't open source %s\n", pSourceFile);
        success = false;
    }
    return success;
}

bool EDTCompiler::IncludeSourceFile(
    const char* pSourceFile,
    std::vector<Token>& source, 
    size_t index
)
{
    std::vector<Token> source_2;
    bool success = false;
    if (this->TokenizeInput(pSourceFile, source_2)) {
        source.insert(source.begin() + index, source_2.begin(), source_2.end());
        success = true;
    }
    return success;
}

bool EDTCompiler::VerifySyntaxAndParse(
    std::vector<Token>& tokens
)
{
    //Something went wrong or not
    bool raise_error = false;
    //Activated by ANIME which needs an array
    bool anime = false;

    //Constants got by the const keyword
    constant_map const_map;
    Token ProcName { TOKEN_PROC, 0, ""};
    Token LabName { TOKEN_PROC, 0, ""};
    Token* pAnimP = nullptr;
    int ident_size = 0;
    size_t size = tokens.size();
    size_t i = 0;
    try {
        for (i = 0; i < size; ) {
            auto& t = tokens[i];
                if (t.kind == TOKEN_KEYWORD) {
                    switch (t.number) {
                    case KEY_TEXINIT:
                    case KEY_PROC:
                    case KEY_ENEMY:
                    case KEY_BOSS:
                    case KEY_TSET:
                    case KEY_EXANM:
                    case KEY_SET:
                        if (i + 2 < tokens.size() && tokens[i + 1].kind == TOKEN_IDENTIFIER) {
                            i++;
                            Token tmp = { TOKEN_PROC, t.number, tokens[i].pStr };
                            tmp.number = t.number;
                            m_ProcessedTokens.emplace_back(tmp);
                            if (tokens[i + 1].kind == TOKEN_BEGINBLOCK) {
                                i += 2;
                                bool parse_success = false;
                                switch (t.number) {
                                case KEY_TEXINIT: parse_success = ProcessTexInitBlock(tokens, &m_ProcessedTokens, const_map, i); break;
                                case KEY_PROC: parse_success = ProcessEnemyBlock(tokens, &m_ProcessedTokens, const_map, i, 2); break;
                                case KEY_ENEMY: parse_success = ProcessEnemyBlock(tokens, &m_ProcessedTokens, const_map, i, 0); break;
                                case KEY_BOSS: parse_success = ProcessEnemyBlock(tokens, &m_ProcessedTokens, const_map, i, 0); break;
                                case KEY_TSET: parse_success = ProcessEnemyBlock(tokens, &m_ProcessedTokens, const_map, i, 0); break;
                                case KEY_EXANM: parse_success = ProcessEnemyBlock(tokens, &m_ProcessedTokens, const_map, i, 0); break;
                                case KEY_SET: parse_success = ProcessEnemyBlock(tokens, &m_ProcessedTokens, const_map, i, 0); break;
                                }
                                if (!parse_success) throw 1;
                            }
                            else throw tokens[i + 1];
                        }
                        else throw 0;
                        break;
                    case KEY_CONST:
                        if (i + 2 < tokens.size() && tokens[i + 1].kind == TOKEN_IDENTIFIER) {
                            i++;
                            Token tmp = { TOKEN_PROC, tokens[i + 1].line };
                            const std::string& str = tokens[i].pStr;
                            i++;
                            switch (tokens[i].kind) {
                            case TOKEN_STRING: tmp.kind = TOKEN_STRING; tmp.pStr = tokens[i].pStr; i++; break;
                            default: tmp.kind = TOKEN_NUMBER; if (!CalcConvertConst(tokens, &m_ProcessedTokens, i, KEY_GLOBAL, const_map, tmp.number)) throw t; break;
                            }
                            const_map.insert({ str, tmp });
                        }
                        else throw 0;
                        break;
                    case KEY_INCLUDE:
                        if (i + 1 < tokens.size() && tokens[i + 1].kind == TOKEN_STRING) {
                            std::string src = tokens[i + 1].pStr;
                            i += 2;
                            if (!IncludeSourceFile(src.c_str(), tokens, i)) throw SourceInfo{ t.source, src.c_str(),  t.line };
                            size = tokens.size();
                        }
                        else throw 0;
                    }

                }
                else throw t;
        }
    }
    catch (const Token& t) {
        printf("Unexpected token in line %d @ %s\n", t.line, t.source);
        raise_error = true;
    }
    catch (const SourceInfo& src) {
        printf("%s - %d: Failed including source file %s\n", src.pWhereSource, src.line, src.pWhatSource);
        raise_error = true;
    }
    catch (const int code) {
        const char* msg = "";
        switch (code) {
        case 0: msg = "Unexpected End of File"; break;
        case 1: msg = "Failed processing procedure block"; break;
        case 2: msg = "Failed processing const"; break;
        }
        raise_error = true;
        printf("L: %d Error while parsing: %s\n", tokens[i].line, msg);
    }
    catch (...) {
        raise_error = true;
    }
    return !raise_error;
}

bool ProcessCommonCmdData(
    std::vector<Token>& tokens, 
    std::vector<Token>* pProcessedData, 
    constant_map& const_map,
    ECL_INSTRUCTION command, 
    ECL_INSTRUCTION& last_cmd, 
    KEYWORD_KIND proc_type,
    size_t& idx
)
{
    bool success = true;
    size_t size = tokens.size();
    try {
        //Used to look at reference for arguments
        ECLInstructionDefine def = g_InstructionSize[command];

        //Instruction and how many parameters it has
        Token cmd;
        cmd.kind = TOKEN_COMMAND;
        cmd.number = command;
        last_cmd = command;

        int num_args = def.paramdatatype.size() - 1;
        Token cnt;
        cnt.kind = TOKEN_INCOUNT;
        cnt.number = num_args;

        pProcessedData->emplace_back(cmd);
        pProcessedData->emplace_back(cnt);
        idx++;
        if (idx + (num_args) * 2 - 1 < size) {
            for (int x = num_args, y = 0; x > 0;) {
                Token rg;
                rg.advance = 0;
                const Token& rf = tokens[idx];
                if (command == PUSHR || command == POPR || command == MOVC) {
                    int reg = def.paramdatatype[0];
                    def.paramdatatype[0] = COMMAND;
                    def.paramdatatype[1] = U8;
                }

                rg.line = rf.line;
                rg.source = rf.source;

                switch (def.paramdatatype[1 + y]) {
                case U32: case I32:
                    rg.advance += 2;
                case U16: case I16:
                    rg.advance++;
                case U8: case I8:
                    rg.advance++;
                    {
                        rg.kind = TOKEN_NUMBER;
                        if (!CalcConvertConst(tokens, pProcessedData, idx, proc_type, const_map, rg.number)) throw 3;
                    }    break;
                case ADDRESS:
                    rg.kind = TOKEN_IDENTIFIER;
                    rg.advance = 4;
                    rg.pStr = rf.pStr;
                    idx++;
                    break;
                case STRING :
                    if (rf.kind == TOKEN_STRING) {
                        rg.kind = TOKEN_STRING;
                        rg.pStr = rf.pStr;
                        rg.advance = rf.pStr.length() + 1;
                    }
                    else if (rf.kind == TOKEN_IDENTIFIER) {
                        rg.kind = TOKEN_STRING;
                        if (const_map.find(rf.pStr) != const_map.end()) {
                            rg.pStr = const_map[rf.pStr].pStr;
                            rg.advance = rg.pStr.length() + 1;
                        }
                        else throw 4;
                    }
                    else throw 4;
                    idx++;
                    break;
                default:
                    throw 2;
                }
                pProcessedData->emplace_back(rg);
                x--;
                if (x > 0 && tokens[idx].kind == TOKEN_COMMA) {
                    idx++;
                    y++;
                }
            }
        }
        else throw 0;
    }
    catch (...) {
        success = false;
    }

    return success;
}

bool ProcessTexInitData(
    std::vector<Token>& tokens,
    std::vector<Token>* pProcessedData,
    constant_map& const_map,
    TEXINIT_INSTRUCTION command,
    TEXINIT_INSTRUCTION& last_cmd,
    size_t& idx
)
{
    bool success = true;
    size_t size = tokens.size();
    std::vector<Token> frames;
    try {
        //Used to look at reference for arguments
        ECLInstructionDefine def = g_TexInitSize[command];

        //Instruction and how many parameters it has
        Token cmd;
        cmd.kind = TOKEN_COMMAND;
        cmd.number = command;
        last_cmd = command;

        int num_args = def.paramdatatype.size() - 1;
        Token cnt;
        cnt.kind = TOKEN_INCOUNT;
        cnt.number = num_args;

        pProcessedData->emplace_back(cmd);
        if(command != ANIME)
            pProcessedData->emplace_back(cnt);

        idx++;
        if (idx + (num_args) * 2 - 1 < size) {
            for (int x = num_args, y = 0; x > 0;) {
                Token rg;
                rg.advance = 0;
                const Token& rf = tokens[idx];

                rg.line = rf.line;
                rg.source = rf.source;

                switch (def.paramdatatype[1 + y]) {
                case U32: case I32:
                    rg.advance += 2;
                case U16: case I16:
                    rg.advance++;
                case U8: case I8:
                    rg.advance++;
                    {
                        rg.kind = TOKEN_NUMBER;
                        if (!CalcConvertConst(tokens, pProcessedData, idx, KEY_TEXINIT, const_map, rg.number)) throw 3;
                    }    break;
                case ADDRESS:
                    rg.kind = TOKEN_IDENTIFIER;
                    rg.advance = 4;
                    rg.pStr = rf.pStr;
                    idx++;
                    break;
                case STRING:
                    if (rf.kind == TOKEN_STRING) {
                        rg.kind = TOKEN_STRING;
                        rg.pStr = rf.pStr;
                        rg.advance = rf.pStr.size() + 1;
                    }
                    else if (rf.kind == TOKEN_IDENTIFIER) {
                        rg.kind = TOKEN_STRING;
                        if (const_map.find(rf.pStr) != const_map.end()) {
                            rg.pStr = const_map[rf.pStr].pStr;
                            rg.advance = rg.pStr.size() + 1;
                        }
                        else throw 4;
                    }
                    else throw 4;
                    idx++;
                    break;
                default:
                    throw 2;
                }
                if (command != ANIME)
                    pProcessedData->emplace_back(rg);
                else
                    frames.emplace_back(rg);
                x--;
                if (x == 0 && command == ANIME && tokens[idx].kind == TOKEN_COMMA) {
                    x++; 
                    def.paramdatatype.emplace_back(U8);
                    idx++;
                    y++;
                }
                else if (x > 0 && tokens[idx].kind == TOKEN_COMMA) {
                    idx++;
                    y++;
                }
            }
            if (command == ANIME) {
                cnt.number = 1 + frames.size();
                pProcessedData->emplace_back(cnt);
                pProcessedData->insert(pProcessedData->begin() + pProcessedData->size(), frames.begin(), frames.end());
                Token rg;
                rg.advance = 1;
                rg.kind = TOKEN_NUMBER;
                rg.number = 0xff;
                rg.advance = 1;
                rg.line = -1;
                pProcessedData->emplace_back(rg);
            }
        }
        else throw 0;
    }
    catch (...) {
        success = false;
    }

    return success;
}

bool  ProcessConditionOrLoopBlock(
    std::vector<Token>& tokens,
    std::vector<Token>* pProcessedData,
    constant_map& const_map,
    KEYWORD_KIND proc_kind,
    KEYWORD_KIND type,
    bool (*BlockProcessFn)(std::vector<Token>&, std::vector<Token>*, constant_map&, size_t&, int),
    size_t& idx
)
{
    bool success = true;
    size_t size = tokens.size();
    try {
        switch (type) {
        case KEY_IF:
            if (idx + 3 < size) {
                idx++;
                if (tokens[idx].kind == TOKEN_BEGINPARENTHESIS) {
                    if (CalcConvert(tokens, pProcessedData, idx, proc_kind, const_map, false)) {
                        if (tokens[idx].kind == TOKEN_BEGINBLOCK) {
                            idx++;
                            std::string lab_if = "@if_" + std::to_string(g_IfCnt);
                            g_IfCnt++;
                            Token if_cmd;
                            if_cmd.kind = TOKEN_COMMAND;
                            if_cmd.number = FJMP;
                            pProcessedData->emplace_back(if_cmd);
                            Token if_cnt;
                            if_cnt.kind = TOKEN_INCOUNT;
                            if_cnt.number = 1;
                            pProcessedData->emplace_back(if_cnt);
                            Token if_label;
                            if_label.kind = TOKEN_IDENTIFIER;
                            if_label.advance = 4;
                            if_label.pStr = lab_if;
                            pProcessedData->emplace_back(if_label);
                            if (!BlockProcessFn(tokens, pProcessedData, const_map, idx, 1)) throw 0;

                            Token tmp{ TOKEN_LABEL, 0, lab_if };
                            pProcessedData->emplace_back(tmp);
                        }
                    }
                    else throw 0;
                }


            }
            else throw 0;

        case KEY_WHILE:
            if (idx + 3 < size) {
                idx++;
                if (tokens[idx].kind == TOKEN_BEGINPARENTHESIS) {
                    std::string w_cond = "@whilec_" + std::to_string(g_WhileCnt);
                    std::string w_end = "@whilee_" + std::to_string(g_WhileCnt);
                    g_WhileCnt++;
                    Token tmp1{ TOKEN_LABEL, 0, w_cond };
                    pProcessedData->emplace_back(tmp1);
                    if (CalcConvert(tokens, pProcessedData, idx, proc_kind, const_map, false)) {
                        Token w_cmd1;
                        w_cmd1.kind = TOKEN_COMMAND;
                        w_cmd1.number = FJMP;
                        pProcessedData->emplace_back(w_cmd1);
                        Token w_cnt1;
                        w_cnt1.kind = TOKEN_INCOUNT;
                        w_cnt1.number = 1;
                        pProcessedData->emplace_back(w_cnt1);
                        Token w_label1;
                        w_label1.kind = TOKEN_IDENTIFIER;
                        w_label1.advance = 4;
                        w_label1.pStr = w_end;
                        pProcessedData->emplace_back(w_label1);
                        if (tokens[idx].kind == TOKEN_BEGINBLOCK) {
                            idx++;
                            if (!BlockProcessFn(tokens, pProcessedData, const_map, idx, 1)) throw 0;

                            Token w_cmd2;
                            w_cmd2.kind = TOKEN_COMMAND;
                            w_cmd2.number = JMP;
                            pProcessedData->emplace_back(w_cmd2);
                            Token w_cnt2;
                            w_cnt2.kind = TOKEN_INCOUNT;
                            w_cnt2.number = 1;
                            pProcessedData->emplace_back(w_cnt2);
                            Token w_label2;
                            w_label2.kind = TOKEN_IDENTIFIER;
                            w_label2.advance = 4;
                            w_label2.pStr = w_cond;
                            pProcessedData->emplace_back(w_label2);

                            Token tmp2{ TOKEN_LABEL, 0, w_end };
                            pProcessedData->emplace_back(tmp2);
                        }
                    }
                    else throw 0;
                }
            }
            else throw 0;
            break;


        case KEY_LOOP:
            if (idx + 3 < size) {
                idx++;
                if (tokens[idx].kind == TOKEN_BEGINPARENTHESIS) {
                    if (CalcConvert(tokens, pProcessedData, idx, proc_kind, const_map, false)) {
                        if (tokens[idx].kind == TOKEN_BEGINBLOCK) {
                            idx++;
                            std::string w_cond = "@loopb_" + std::to_string(g_LoopCnt);
                            std::string w_end = "@loope_" + std::to_string(g_LoopCnt);
                            g_LoopCnt++;

                            Token lpop_cmd;
                            lpop_cmd.kind = TOKEN_COMMAND;
                            lpop_cmd.number = LPOP;
                            pProcessedData->emplace_back(lpop_cmd);
                            Token lpop_cnt;
                            lpop_cnt.kind = TOKEN_INCOUNT;
                            lpop_cnt.number = 0;
                            pProcessedData->emplace_back(lpop_cnt);

                            Token tmp1{ TOKEN_LABEL, 0, w_cond };
                            pProcessedData->emplace_back(tmp1);

                            Token w_cmd1;
                            w_cmd1.kind = TOKEN_COMMAND;
                            w_cmd1.number = LJMP;
                            pProcessedData->emplace_back(w_cmd1);
                            Token w_cnt1;
                            w_cnt1.kind = TOKEN_INCOUNT;
                            w_cnt1.number = 1;
                            pProcessedData->emplace_back(w_cnt1);
                            Token w_label1;
                            w_label1.kind = TOKEN_IDENTIFIER;
                            w_label1.advance = 4;
                            w_label1.pStr = w_end;
                            pProcessedData->emplace_back(w_label1);
                            if (!BlockProcessFn(tokens, pProcessedData, const_map, idx, 1)) throw 0;

                            Token w_cmd2;
                            w_cmd2.kind = TOKEN_COMMAND;
                            w_cmd2.number = JMP;
                            pProcessedData->emplace_back(w_cmd2);
                            Token w_cnt2;
                            w_cnt2.kind = TOKEN_INCOUNT;
                            w_cnt2.number = 1;
                            pProcessedData->emplace_back(w_cnt2);
                            Token w_label2;
                            w_label2.kind = TOKEN_IDENTIFIER;
                            w_label2.advance = 4;
                            w_label2.pStr = w_cond;
                            pProcessedData->emplace_back(w_label2);

                            Token tmp2{ TOKEN_LABEL, 0, w_end };
                            pProcessedData->emplace_back(tmp2);
                        }
                    }
                    else throw 0;
                }
            }
            else throw 0;
            break;
        }
    }
    catch (...) {
        success = false;
    }
    return success;
}

bool ProcessTexInitBlock(
    std::vector<Token>& tokens, 
    std::vector<Token>* pProcessedData, 
    constant_map& const_map, 
    size_t& idx
)
{
    bool success = true;
    TEXINIT_INSTRUCTION last_cmd = TEXINIT_INSTRUCTION::LOAD;
    const size_t size = tokens.size();

    try {
        bool on_exit = false;
        for (; idx < size; ) {
            const Token& t = tokens[idx];
            switch (t.kind) {
                //Just comands
            case TOKEN_COMMAND: {
                bool found_command = false;
                for (auto c : g_TexInitCheck) if (c == t.number) found_command = true;
                if (found_command) {
                    if (!ProcessTexInitData(tokens, pProcessedData, const_map, (TEXINIT_INSTRUCTION)t.number, last_cmd, idx)) throw 0;
                }
                else throw t;
            }    break;
                              //Const for local constants
            case TOKEN_KEYWORD:
                switch (t.number) {
                case KEY_CONST:
                    if (idx + 2 < tokens.size() && tokens[idx + 1].kind == TOKEN_IDENTIFIER) {
                        idx++;
                        Token tmp = { TOKEN_PROC, tokens[idx + 1].line };
                        const std::string& str = tokens[idx].pStr;
                        idx++;
                        switch (tokens[idx].kind) {
                        case TOKEN_STRING: tmp.kind = TOKEN_STRING; tmp.pStr = tokens[idx].pStr; break;
                        default: tmp.kind = TOKEN_NUMBER; if (!CalcConvertConst(tokens, pProcessedData, idx, KEY_GLOBAL, const_map, tmp.number)) throw 2; break;
                        }
                        const_map.insert({ str, tmp });
                    }
                    else throw 0;
                    break;
                default:
                    throw 0;
                }
                break;
            case TOKEN_ENDBLOCK:
            {
                idx++;
                if (last_cmd != TEXINITEND) {
                    Token cmd;
                    cmd.kind = TOKEN_COMMAND;
                    cmd.number = TEXINITEND;

                    Token cnt;
                    cnt.kind = TOKEN_INCOUNT;
                    cnt.number = 0;
                    pProcessedData->emplace_back(cmd);
                    pProcessedData->emplace_back(cnt);
                }
                Token end;
                end.kind = TOKEN_ENDPROC;
                pProcessedData->emplace_back(end);

                on_exit = true;
            }
            break;
            default:
                throw 0;
                break;
            }
            if (on_exit) break;
        }
    }
    catch (...) {
        success = false;
    }
    return success;
}

bool ProcessEnemyBlock(
    std::vector<Token>& tokens, 
    std::vector<Token>* pProcessedData, 
    constant_map& const_map,
    size_t& idx,
    int exit_type
)
{
    bool success = true;
    ECL_INSTRUCTION last_cmd = NOP;
    const size_t size = tokens.size();

    try {
        bool on_exit = false;
        for (; idx < size; ) {
            const Token& t = tokens[idx];
            switch (t.kind) {
            //Just comands
            case TOKEN_COMMAND: {
                bool found_command = true;
                if (found_command) {
                    if(!ProcessCommonCmdData(tokens, pProcessedData, const_map, (ECL_INSTRUCTION)t.number, last_cmd, KEY_ENEMY, idx)) throw 0;
                }
                else throw t;
            }    break;
            //For normal math expressions
            case TOKEN_IDENTIFIER: {
                if (idx + 1 < size && tokens[idx + 1].kind == TOKEN_DOTS) {
                    Token tmp{ TOKEN_LABEL, t.line, t.pStr };
                    pProcessedData->emplace_back(tmp);
                    idx += 2;
                }
                else if (!CalcConvert(tokens, pProcessedData, idx, KEY_ENEMY, const_map, true)) throw 0;
            } break;
            //Const for local constants
            case TOKEN_KEYWORD:
                if (t.number == KEY_IF || t.number == KEY_WHILE || t.number == KEY_LOOP) {
                    if(!ProcessConditionOrLoopBlock(tokens, pProcessedData, const_map, KEY_ENEMY, (KEYWORD_KIND)t.number, ProcessEnemyBlock,idx)) throw 0;
                }
                else {
                    switch (t.number) {
                    case KEY_CONST:
                        if (idx + 2 < tokens.size() && tokens[idx + 1].kind == TOKEN_IDENTIFIER) {
                            idx++;
                            Token tmp = { TOKEN_PROC, tokens[idx + 1].line };
                            const std::string& str = tokens[idx].pStr;
                            idx++;
                            switch (tokens[idx].kind) {
                            case TOKEN_STRING: tmp.kind = TOKEN_STRING; tmp.pStr = tokens[idx].pStr; break;
                            default: tmp.kind = TOKEN_NUMBER; if (!CalcConvertConst(tokens, pProcessedData, idx, KEY_GLOBAL, const_map, tmp.number)) throw 2; break;
                            }
                            const_map.insert({ str, tmp });
                        }
                        else throw 0;
                        break;
                    default:
                        throw 0;
                    }
                }
                break;
            case TOKEN_ENDBLOCK:
                idx++;
                if (exit_type != 1) {
                    /*if (last_cmd != EXIT) {
                        Token cmd;
                        cmd.kind = TOKEN_COMMAND;
                        cmd.number = (exit_type == 0) ? EXIT : RET;

                        Token cnt;
                        cnt.kind = TOKEN_INCOUNT;
                        cnt.number = !(exit_type == 0);
                        pProcessedData->emplace_back(cmd);
                        pProcessedData->emplace_back(cnt);
                        if (exit_type == 2) {
                            Token rt;
                            rt.kind = TOKEN_NUMBER;
                            rt.number = 0;
                            rt.advance = 1;
                            pProcessedData->emplace_back(rt);
                        }
                    }*/
                    Token end;
                    end.kind = TOKEN_ENDPROC;
                    pProcessedData->emplace_back(end);
                    
                }
                on_exit = true;
                break;
            default:
                break;
            }
            if (on_exit) break;
        }
    }
    catch(...){
        success = false;
    }
    return success;
}

bool ProcessTSetBlock(
    std::vector<Token>& tokens,
    std::vector<Token>* pProcessedData, 
    constant_map& const_map, 
    size_t& idx,
    int exit_type
)
{
    bool success = true;
    ECL_INSTRUCTION last_cmd = NOP;
    const size_t size = tokens.size();

    try {
        bool on_exit = false;
        for (; idx < size; ) {
            const Token& t = tokens[idx];
            switch (t.kind) {
                //Just comands
            case TOKEN_COMMAND: {
                bool found_command = false;
                for (auto c : g_TSetCheck) if (c == t.number) found_command = true;
                if(!found_command)for (auto c : g_ControlFlow) if (c == t.number) found_command = true;
                if (found_command) {
                    if (!ProcessCommonCmdData(tokens, pProcessedData, const_map, (ECL_INSTRUCTION)t.number, last_cmd, KEY_TSET, idx)) throw 0;
                }
                else throw t;
            }    break;
                              //For normal math expressions
            case TOKEN_IDENTIFIER: {
                if (idx + 1 < size && tokens[idx + 1].kind == TOKEN_DOTS) {
                    Token tmp{ TOKEN_LABEL, t.line, t.pStr };
                    pProcessedData->emplace_back(tmp);
                    idx += 2;
                }
                else if (!CalcConvert(tokens, pProcessedData, idx, KEY_TSET, const_map, true)) throw 0;
            } break;
                                 //Const for local constants
            case TOKEN_KEYWORD:
                if (t.number == KEY_IF || t.number == KEY_WHILE || t.number == KEY_LOOP) {
                    if (!ProcessConditionOrLoopBlock(tokens, pProcessedData, const_map, KEY_TSET, (KEYWORD_KIND)t.number, ProcessTSetBlock, idx)) throw 0;
                }
                else {
                    switch (t.number) {
                    case KEY_CONST:
                        if (idx + 2 < tokens.size() && tokens[idx + 1].kind == TOKEN_IDENTIFIER) {
                            idx++;
                            Token tmp = { TOKEN_PROC, tokens[idx + 1].line };
                            const std::string& str = tokens[idx].pStr;
                            idx++;
                            switch (tokens[idx].kind) {
                            case TOKEN_STRING: tmp.kind = TOKEN_STRING; tmp.pStr = tokens[idx].pStr; break;
                            default: tmp.kind = TOKEN_NUMBER; if (!CalcConvertConst(tokens, pProcessedData, idx, KEY_GLOBAL, const_map, tmp.number)) throw 2; break;
                            }
                            const_map.insert({ str, tmp });
                        }
                        else throw 0;
                        break;
                    default:
                        throw 0;
                    }
                }
                break;
            case TOKEN_ENDBLOCK:
                idx++;
                if(exit_type != 1){
                    if (last_cmd != EXIT) {
                        Token cmd;
                        cmd.kind = TOKEN_COMMAND;
                        cmd.number = EXIT;

                        Token cnt;
                        cnt.kind = TOKEN_INCOUNT;
                        cnt.number = 0;
                        pProcessedData->emplace_back(cmd);
                        pProcessedData->emplace_back(cnt);
                    }
                    Token end;
                    end.kind = TOKEN_ENDPROC;
                    pProcessedData->emplace_back(end);

                }
                on_exit = true;
                break;
            default:
                break;
            }
            if (on_exit) break;
        }
    }
    catch (...) {
        success = false;
    }
    return success;
}

bool ProcessExAnmBlock(
    std::vector<Token>& tokens, 
    std::vector<Token>* pProcessedData,
    constant_map& const_map, 
    size_t& idx,
    int exit_type
)
{
    bool success = true;
    ECL_INSTRUCTION last_cmd = NOP;
    const size_t size = tokens.size();

    try {
        bool on_exit = false;
        for (; idx < size; ) {
            const Token& t = tokens[idx];
            switch (t.kind) {
                //Just comands
            case TOKEN_COMMAND: {
                bool found_command = false;
                for (auto c : g_ExAnmCheck) if (c == t.number) found_command = true;
                if(!found_command) for (auto c : g_ControlFlow) if (c == t.number) found_command = true;
                if (found_command) {
                    if (!ProcessCommonCmdData(tokens, pProcessedData, const_map, (ECL_INSTRUCTION)t.number, last_cmd, KEY_EXANM, idx)) throw 0;
                }
                else throw t;
            }    break;
                              //For normal math expressions
            case TOKEN_IDENTIFIER: {
                if (idx + 1 < size && tokens[idx + 1].kind == TOKEN_DOTS) {
                    Token tmp{ TOKEN_LABEL, t.line, t.pStr };
                    pProcessedData->emplace_back(tmp);
                    idx += 2;
                }
                else if (!CalcConvert(tokens, pProcessedData, idx, KEY_EXANM, const_map, true)) throw 0;
            } break;
                                 //Const for local constants
            case TOKEN_KEYWORD:
                if (t.number == KEY_IF || t.number == KEY_WHILE || t.number == KEY_LOOP) {
                    if (!ProcessConditionOrLoopBlock(tokens, pProcessedData, const_map, KEY_EXANM, (KEYWORD_KIND)t.number, ProcessExAnmBlock, idx)) throw 0;
                }
                else {
                    switch (t.number) {
                    case KEY_CONST:
                        if (idx + 2 < tokens.size() && tokens[idx + 1].kind == TOKEN_IDENTIFIER) {
                            idx++;
                            Token tmp = { TOKEN_PROC, tokens[idx + 1].line };
                            const std::string& str = tokens[idx].pStr;
                            idx++;
                            switch (tokens[idx].kind) {
                            case TOKEN_STRING: tmp.kind = TOKEN_STRING; tmp.pStr = tokens[idx].pStr; break;
                            default: tmp.kind = TOKEN_NUMBER; if (!CalcConvertConst(tokens, pProcessedData, idx, KEY_GLOBAL, const_map, tmp.number)) throw 2; break;
                            }
                            const_map.insert({ str, tmp });
                        }
                        else throw 0;
                        break;
                    default:
                        throw 0;
                    }
                }
                break;
            case TOKEN_ENDBLOCK:
                idx++; 
                if (exit_type != 1) {
                    if (last_cmd != EXIT) {
                        Token cmd;
                        cmd.kind = TOKEN_COMMAND;
                        cmd.number = EXIT;

                        Token cnt;
                        cnt.kind = TOKEN_INCOUNT;
                        cnt.number = 0;
                        pProcessedData->emplace_back(cmd);
                        pProcessedData->emplace_back(cnt);

                    }

                    Token end;
                    end.kind = TOKEN_ENDPROC;
                    pProcessedData->emplace_back(end);
                }
                on_exit = true;
                break;
            default:
                break;
            }
            if (on_exit) break;
        }
    }
    catch (...) {
        success = false;
    }
    return success;
}

bool ProcessSetBlock(
    std::vector<Token>& tokens,
    std::vector<Token>* pProcessedData,
    constant_map& const_map,
    size_t& idx
)
{
    bool success = true;
    ECL_INSTRUCTION last_cmd = NOP;
    const size_t size = tokens.size();

    try {
        bool on_exit = false;
        for (; idx < size; ) {
            const Token& t = tokens[idx];
            switch (t.kind) {
                //Just comands
            case TOKEN_COMMAND: {
                bool found_command = false;
                for (auto c : g_SetCheck) if (c == t.number) found_command = true;
                if (found_command) {
                    if (!ProcessCommonCmdData(tokens, pProcessedData, const_map, (ECL_INSTRUCTION)t.number, last_cmd, KEY_SET, idx)) throw 0;
                }
                else throw t;
            }    break;
                              //Const for local constants
            case TOKEN_KEYWORD:
                switch (t.number) {
                case KEY_CONST:
                    if (idx + 2 < tokens.size() && tokens[idx + 1].kind == TOKEN_IDENTIFIER) {
                        idx++;
                        Token tmp = { TOKEN_PROC, tokens[idx + 1].line };
                        const std::string& str = tokens[idx].pStr;
                        idx++;
                        switch (tokens[idx].kind) {
                        case TOKEN_STRING: tmp.kind = TOKEN_STRING; tmp.pStr = tokens[idx].pStr; break;
                        default: tmp.kind = TOKEN_NUMBER; if (!CalcConvertConst(tokens, pProcessedData, idx, KEY_GLOBAL, const_map, tmp.number)) throw 2; break;
                        }
                        const_map.insert({ str, tmp });
                    }
                    else throw 0;
                    break;
                default:
                    throw 0;
                }
                break;
            case TOKEN_ENDBLOCK:
            {
                idx++;
                if (last_cmd != EXIT) {
                    Token cmd;
                    cmd.kind = TOKEN_COMMAND;
                    cmd.number = EXIT;

                    Token cnt;
                    cnt.kind = TOKEN_INCOUNT;
                    cnt.number = 0;
                    pProcessedData->emplace_back(cmd);
                    pProcessedData->emplace_back(cnt);
                }
                Token end;
                end.kind = TOKEN_ENDPROC;
                pProcessedData->emplace_back(end);

                on_exit = true;
            }
            break;
            default:
                throw 0;
                break;
            }
            if (on_exit) break;
        }
    }
    catch (...) {
        success = false;
    }
    return success;
}

bool EDTCompiler::CalculateAddresses() 
{
    bool raise_error = false;
    address offset = sizeof(EDTHeader);
    size_t size = m_ProcessedTokens.size();
    bool found_texproc = false;
    int err_code = -1;
    std::string dupe;
    int proc_kind = 0;
    for (int i = 0; i < size; i++) {
        if (m_ProcessedTokens[i].kind == TOKEN_PROC) {
            ProcDataEx2 chunk;
            chunk.ads = offset;
            const std::string name = m_ProcessedTokens[i].pStr;
            //printf("PROC: %s\n", name.c_str());
            if (m_ProcData.find(name) != m_ProcData.end()) {
                raise_error = true;
                err_code = 1;
                dupe = m_ProcessedTokens[i].pStr;
                break;
            }
            proc_kind = m_ProcessedTokens[i].number;
            chunk.type = proc_kind;
            if (proc_kind == KEY_TEXINIT) {
                if (found_texproc) {
                    raise_error = true;
                    err_code = 0;
                    break;
                }
                else found_texproc = true;
            }
            i++;
            while (m_ProcessedTokens[i].kind != TOKEN_ENDPROC) {
                switch (m_ProcessedTokens[i].kind){
                    case TOKEN_COMMAND: {
                        //if (proc_kind == KEY_TEXINIT && m_ProcessedTokens[i].number == ANIME) __debugbreak();
                        ECLInstructionDataEx data;
                        data.line = m_ProcessedTokens[i].line;
                        data.source = m_ProcessedTokens[i].source;
                        data.cmd = m_ProcessedTokens[i].number;
                        i++;
                        if (m_ProcessedTokens[i].kind == TOKEN_INCOUNT) {
                            size_t pc = m_ProcessedTokens[i].number;
                            data.cnt = pc;
                            offset++;
                            i++;
                            for (int j = 0; j < pc; j++, i++) {
                                ECLParamData param;
                                int adv = m_ProcessedTokens[i].advance;
                                int kind = m_ProcessedTokens[i].kind;
                                if (kind == TOKEN_STRING || kind == TOKEN_IDENTIFIER) {
                                    param.stringdata = (char*)m_ProcessedTokens[i].pStr.data();
                                    if (kind == TOKEN_STRING) {
                                        param.datatype = STRING;
                                        //printf("%s\n", param.stringdata);
                                    }
                                    else param.datatype = ADDRESS;
                                }
                                else if (m_ProcessedTokens[i].kind == TOKEN_NUMBER) {
                                    param.sdword = m_ProcessedTokens[i].number;
                                    switch (adv) {
                                    case 1: param.datatype = U8; break;
                                    case 2: param.datatype = U16; break;
                                    case 4: param.datatype = U32; break;
                                    }
                                }
                                param.adv = adv;
                                data.param.emplace_back(param);
                                if (m_ProcessedTokens[i].advance > 100 || m_ProcessedTokens[i].advance < 0) {
                                    __debugbreak();
                                    auto& debb = m_ProcessedTokens[i];
                                    while (1) {
                                        ;
                                    }
                                }
                                offset += adv;
                            }
                        }
                        chunk.cmd_data.emplace_back(std::move(data));
                    } break;
                    case TOKEN_LABEL: {
                        if (chunk.label_data.find(m_ProcessedTokens[i].pStr) != chunk.label_data.end()) {
                            raise_error = true;
                            err_code = 2;
                            dupe = m_ProcessedTokens[i].pStr;
                            break;
                        }
                        chunk.label_data.insert({ m_ProcessedTokens[i].pStr, offset});
                        i++;
                    }break;
                }
                if (raise_error)
                    break;
            }
            m_ProcData.insert({name, chunk});
            m_ProcNames.emplace_back(name);
        }
        if (raise_error)
            break;
    }
    if (!raise_error)
        m_FileSize = offset;
    else {
        const char* msg = "";
        switch (err_code) {
        case 0: printf("Error: There can only be one TexInit Procedure."); break;
        case 1: printf("Duplicate procedure name: %s\n", dupe.c_str()); break;
        case 2: printf("Duplicate label name: %s\n", dupe.c_str()); break;
        }
    }
    return !raise_error;
}

bool EDTCompiler::PopulateAddresses()
{
    //Something went wrong or not
    bool on_success = true;

    try {
        for (auto& p : m_ProcData) {
            ProcDataEx2& chunk = p.second;
            if (chunk.type == KEY_TEXINIT) continue;
            for (auto& c : chunk.cmd_data) {
                switch (c.cmd) {
                case BEGINCALLBACK:
                case ENDCALLBACK:
                case TJMP:
                case FJMP:
                case JMP:
                case OJMP:
                case AJMP:
                case LJMP:
                {
                    const string& str = c.param[0].stringdata;
                    if (chunk.label_data.find(str) != chunk.label_data.end())
                        c.param[0].ads = chunk.label_data[str];
                    else throw SourceInfo{"", str, c.line};
                }break;
                case CALL:
                case ENEMYEX:
                case CMD_6A:
                case CMD_6B:
                case CMD_6C:
                case HPINT:
                {
                    const string& str = c.param[0].stringdata;
                    if (m_ProcData.find(str) != m_ProcData.end())
                        c.param[0].ads = m_ProcData[str].ads;
                    else throw SourceInfo{ "", str, c.line};
                }break;
                case ENEMY:
                case ENEMYCIRCLE:
                case ENEMY2:
                case ENEMY3:
                case ATK:
                case ANMTASK:
                {
                    const string& str = c.param[2].stringdata;
                    if (m_ProcData.find(str) != m_ProcData.end())
                        c.param[2].ads = m_ProcData[str].ads;
                    else throw SourceInfo{ "", str, c.line};
                }break;
                case BOSS:
                case CMD_68:
                case CMD_70:
                case CHILD:
                case CMD_72:
                case CMD_73:
                {
                    const string& str = c.param[3].stringdata;
                    if (m_ProcData.find(str) != m_ProcData.end())
                        c.param[3].ads = m_ProcData[str].ads;
                    else throw SourceInfo{ "", str, c.line};
                }break;
                }
            }
        }
    }
    catch (const SourceInfo& inf) {
        printf("%s : %d > Procedure or label \"%s\" doesn't exist.\n", inf.pWhereSource, inf.line, inf.pWhatSource);
        on_success = false;
    }
    return on_success;
}

bool EDTCompiler::ProcessHeader() 
{
    bool raise_error = false;
    memset(&m_Header, 0x00, sizeof(EDTHeader));
    if (m_ProcData.find("main") != m_ProcData.end()) {
        m_Header.init_enm = m_ProcData["main"].ads;
        if (m_ProcData.find("tex_init") != m_ProcData.end()) {
            m_Header.tex_init = m_ProcData["tex_init"].ads;
        }
        else raise_error = true;
    }
    else raise_error = true;
    return !raise_error;
}

void* EDTCompiler::JoinData()
{
    char* pData = (char*)malloc(m_FileSize);
    size_t idx = sizeof(EDTHeader);
    memset(pData, 0x00, m_FileSize);
    memcpy(pData, &m_Header, sizeof(EDTHeader));
    for (const auto& chunk_name : m_ProcNames) {
        const ins_data& ref_ins = m_ProcData.at(chunk_name).cmd_data;
        for (const auto& data : ref_ins) {
            switch (data.cmd) {
            case PARENT:
                pData[idx] = char(data.param[0].sdword % 0x100);
                pData[idx + 1] = data.cmd;
                pData[idx + 2] = char(data.param[1].sdword % 0x100);
                idx += 2;
            case PUSHR:
            case POPR:
            case MOVC:
                pData[idx] = char(data.param[0].sdword % 0x100);
                pData[idx + 1] = data.cmd;
                idx += 2;
                if (data.cmd == MOVC) {
                    *(int*)((char*)pData + idx) = data.param[1].sdword;
                    idx += 4;
                }
                break;
            default: {
                pData[idx] = data.cmd;
                idx++;
                for (auto& t : data.param) {
                    switch (t.datatype) {
                    case U8: {
                        char bytedata = (char)(t.sdword % 0x100);
                        pData[idx] = bytedata;
                        idx++;
                        break;
                    }
                    case U16: {
                        short shortdata = (short)(t.sdword % 0x10000);
                        *(short*)(pData + idx) = shortdata;
                        idx += 2;
                        break;
                    }
                    case ADDRESS:
                    case U32: {
                        int intdata = (t.sdword);
                        *(int*)(pData + idx) = intdata;
                        idx += 4;
                        break;
                    }
                    case STRING: {
                        memcpy(pData + idx, t.stringdata, t.adv);
                        idx += t.adv;
                    }
                    }
                }
            }
                   break;
            }
        }
    }
    return pData;
}
