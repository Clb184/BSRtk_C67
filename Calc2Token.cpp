#include "Calc2Token.hpp"

bool CalcConvert(
	std::vector<Token>& from,
	std::vector<Token>* to,
	size_t& idx,
	KEYWORD_KIND proc,
    constant_map& const_map,
    bool save_ret
)
{
    bool success = false;
    std::deque<Token> toks;
    int receiver = -1;
    if (Calc2Token(from, toks, idx, proc, const_map, false, receiver, 1)) {
        if ((save_ret && receiver != -1) || (!save_ret && receiver == -1) && toks.size() > 0) {
            if (PresolveTokens(toks)) {
                if (Token2ProcessedData(to, toks, receiver))
                    success = true;
            }
        }
    }
    return success;
}

bool CalcConvertConst(
    std::vector<Token>& from,
    std::vector<Token>* to,
    size_t& idx,
    KEYWORD_KIND proc,
    constant_map& const_map,
    int& result
)
{
    bool success = false;
    std::deque<Token> toks;
    int receiver = -1;
    if (Calc2Token(from, toks, idx, proc, const_map, true, receiver, 1)) {
        if (receiver == -1) {
            if (PresolveTokens(toks)) {
                if (toks.size() == 1 && toks[0].kind == TOKEN_NUMBER) {
                    result = toks[0].number;
                    success = true;
                }
            }
        }
    }
    return success;
}


bool Calc2Token(
    std::vector<Token>& from, 
    std::deque<Token>& to, 
    size_t& idx, 
    KEYWORD_KIND proc, 
    constant_map& const_map, 
    bool constant_exp,
    int& receiver,
    int num_args
)
{
    bool success = true;
    size_t i = idx;
    int proc_kind = proc;

    //Store the result to analyze later
    int fn = -1;
    std::deque<Token> nums;
    std::stack<Token> syms;
    std::stack<Token> conds;
    std::stack<Token> andor;

    int args = 1;

    TOKEN_KIND expected_tokens[8] = { TOKEN_IDENTIFIER, TOKEN_ARITHF, TOKEN_ARITHS, TOKEN_NUMBER, TOKEN_IGNORE };
    Token dummy_token = { TOKEN_IGNORE };
    Token guide_token = dummy_token;
    int num_expected = 4;
    bool exit_op = false;
    int nnums = 0;
    int nsyms = 0;
    int nconds = 0;
    int nandor = 0;
    bool negate = false;

    try {
        for (; i < from.size(); i++) {
            auto& t = from[i];
            for (int s = 0; s <= num_expected; s++) {
                if (expected_tokens[s] == t.kind) break;
            }
            switch (t.kind) {
            case TOKEN_IDENTIFIER:
                if (guide_token.kind == TOKEN_NUMBER || guide_token.kind == TOKEN_REGISTER) {
                    exit_op = true;
                    break;
                }

            {
                bool found_register = false;
                switch (proc_kind) {
                case KEY_PROC:
                case KEY_ENEMY:
                    if (g_EnemyRegs.find(t.pStr) != g_EnemyRegs.end()) {
                        t.number = g_EnemyRegs[t.pStr];
                        found_register = true;
                    }
                    break;
                case KEY_TSET:
                    if (g_AtkRegs.find(t.pStr) != g_AtkRegs.end()) {
                        t.number = g_AtkRegs[t.pStr];
                        found_register = true;
                    }
                    break;
                case KEY_EXANM:
                    if (g_ExAnmRegs.find(t.pStr) != g_ExAnmRegs.end()) {
                        t.number = g_ExAnmRegs[t.pStr];
                        found_register = true;
                    }
                    break;
                }
                if (!found_register) {
                    if (const_map.find(t.pStr) != const_map.end()) {
                        auto& tkk = const_map[t.pStr];
                        int line = t.line;
                        const char* source = t.source;
                        t = tkk;
                        t.line = line;
                        t.source = source;
                        nums.push_back(t);
                        nnums++;
                        if (negate) {
                            Token ng;
                            ng.kind = TOKEN_COMMAND;
                            ng.number = NEG;
                            nums.push_back(ng);
                            negate = false;
                        }
                        expected_tokens[0] = TOKEN_ARITHS;
                        expected_tokens[1] = TOKEN_COMMA;
                        num_expected = 2;
                        guide_token = t;
                    }
                    else throw t;
                }
                else {
                    if (constant_exp) throw t;
                    t.kind = TOKEN_REGISTER;
                    nums.push_back(t);
                    nnums++;
                    if (negate) {
                        Token ng;
                        ng.kind = TOKEN_COMMAND;
                        ng.number = NEG;
                        nums.push_back(ng);
                        negate = false;
                    }
                    expected_tokens[0] = TOKEN_ARITHS;
                    expected_tokens[1] = TOKEN_COMMA;
                    num_expected = 2;
                    guide_token = t;
                }
            }
            break;
            case TOKEN_ARITHF: {

                    guide_token = t;
                    expected_tokens[0] = TOKEN_BEGINPARENTHESIS;
                    num_expected = 1;
                    guide_token = dummy_token;
                    fn = t.number;
                    switch (t.number) {
                    case AF_COSL:
                    case AF_SINL:
                    case AF_ATAN:
                    case AF_MIN:
                    case AF_MAX:
                        args = 2; break;
                    case AF_RND:
                        args = 1; break;
                    }
            } break;
            case TOKEN_BEGINPARENTHESIS:
                if (Calc2Token(from, nums, ++i, proc, const_map, constant_exp, receiver, args)) {
                        if (from[i].kind != TOKEN_ENDPARENTHESIS) throw from[i];
                        else {
                            if (fn != -1) {
                                Token cmd2;
                                cmd2.kind = TOKEN_COMMAND;
                                switch (fn) {
                                case AF_COSL: cmd2.number = COSL; break;
                                case AF_SINL: cmd2.number = SINL; break;
                                case AF_MIN: cmd2.number = MIN; break;
                                case AF_MAX: cmd2.number = MAX; break;
                                case AF_ATAN: cmd2.number = ATAN; break;
                                case AF_RND: cmd2.number = RND; break;
                                }
                                nums.push_back(cmd2);
                                fn = -1;
                            }
                            nnums++;
                            if (negate) {
                                Token ng;
                                ng.kind = TOKEN_COMMAND;
                                ng.number = NEG;
                                nums.push_back(ng);
                                negate = false;
                            }
                            expected_tokens[0] = TOKEN_ARITHS;
                            expected_tokens[1] = TOKEN_COMMA;
                            num_expected = 2;
                            Token exm;
                            exm.kind = TOKEN_NUMBER;
                            guide_token = exm;
                        }
                    }
                    else throw t;
                break;

            case TOKEN_ENDPARENTHESIS:
                if (num_args != 1) throw t;
                idx = i;
                exit_op = true;
                break;
            case TOKEN_ARITHS:
                if (t.number == AS_ASSIGN) {
                    if (receiver == -1) {
                        receiver = (nums.end() - 1)->number;
                        nums.pop_back();
                        nnums--;
                    }
                    else throw t;
                }
                else {
                    switch (t.number) {
                    case AS_SUB:
                        if (nnums == nsyms) {
                            negate = true;
                            break;
                        }
                    case AS_ADD:
                        while(!syms.empty()) {
                            nums.emplace_back(syms.top());
                            syms.pop();
                        }
                    case AS_MUL:
                    case AS_DIV:
                    case AS_MOD:
                        syms.push(t);
                        nsyms++;
                        break;

                    case AS_EQU:
                    case AS_NOTEQU:
                    case AS_LESS:
                    case AS_LESSEQ:
                    case AS_GREAT:
                    case AS_GREATEQ:
                        to.insert(to.begin() + to.size(), nums.begin(), nums.end());
                        nums.clear();
                        nnums = 0;
                        nsyms = 0;
                        if (!conds.empty()) {
                            to.emplace_back(conds.top());
                            conds.pop();
                            nconds--;
                        }
                        conds.push(t);
                        nconds++;
                        break;

                    case AS_AND:
                    case AS_OR:
                        to.insert(to.begin() + to.size(), nums.begin(), nums.end());
                        nums.clear();
                        nnums = 0;
                        nsyms = 0;
                        while (!conds.empty()) {
                            to.emplace_back(conds.top());
                            conds.pop();
                            nconds--;
                        }
                        {
                            Token ope;
                            ope.kind = TOKEN_COMMAND;
                            ope.number = (t.number == AS_AND) ? MIN : MAX; 
                            andor.push(ope);
                            nandor++;
                        }
                        break;
                    default:
                        break;
                    }
                }
                expected_tokens[0] = TOKEN_BEGINPARENTHESIS;
                expected_tokens[1] = TOKEN_ARITHF;
                expected_tokens[2] = TOKEN_NUMBER;
                expected_tokens[3] = TOKEN_IDENTIFIER;
                num_expected = 4;
                guide_token = dummy_token;
                break;
            case TOKEN_NUMBER:
                nums.emplace_back(t);
                nnums++;
                if (negate) {
                    Token ng;
                    ng.kind = TOKEN_COMMAND;
                    ng.number = NEG;
                    nums.push_back(ng);
                    negate = false;
                }
                expected_tokens[0] = TOKEN_ARITHS;
                expected_tokens[1] = TOKEN_COMMA;
                num_expected = 2;
                guide_token = t;
                break;
            case TOKEN_COMMA:
                if (num_args > 1) {
                    num_args--;
                    to.insert(to.begin() + to.size(), nums.begin(), nums.end());
                    nums.clear();
                    while (!syms.empty()) {
                        to.emplace_back(syms.top());
                        syms.pop();
                    }
                    nnums = 0;
                    nsyms = 0;
                    nconds = 0;
                    to.emplace_back(t);
                    expected_tokens[0] = TOKEN_BEGINPARENTHESIS;
                    expected_tokens[1] = TOKEN_ARITHF;
                    expected_tokens[2] = TOKEN_NUMBER;
                    expected_tokens[3] = TOKEN_IDENTIFIER;
                    num_expected = 4;
                    guide_token = dummy_token;
                }
                else exit_op = true;
                break;
            default:
                if (num_args != 1) throw t;
                exit_op = true;

            }
            if (exit_op) {
                to.insert(to.begin() + to.size(), nums.begin(), nums.end());
                while (!syms.empty()) {
                    to.emplace_back(syms.top());
                    syms.pop();
                }
                if (!conds.empty()) {
                    to.emplace_back(conds.top());
                    conds.pop();
                    nconds--;
                }
                if (!andor.empty()) {
                    to.emplace_back(andor.top());
                    andor.pop();
                    nandor--;
                }
                idx = i;
                break;
            }
        }
    }
    catch (...) {
        printf("Calc convert Error: \n");
        success = false;
    }
    if (success) {

    }
    return success;
}

bool PresolveTokens(std::deque<Token>& toks) {
    bool success = true;
    int size = toks.size();

    std::stack<Token> stk;
    int num = 0;
    std::deque<Token> res;
    int neg = 0;

    try {
        for (auto& t : toks) {
            switch (t.kind) {
            case TOKEN_NUMBER:
                stk.push(t);
                num++;
                break;
            case TOKEN_REGISTER:
                if (num > 0) {
                    res.push_back(stk.top());
                    stk.pop();
                    num--;
                }
                res.push_back(t);
                break;
            case TOKEN_ARITHS:
                {
                    if (num >= 2) {
                        Token p1 = stk.top();
                        stk.pop();
                        switch (t.number) {
                            case AS_ADD: stk.top().number += p1.number; break;
                            case AS_SUB: stk.top().number -= p1.number; break;
                            case AS_MUL: stk.top().number *= p1.number; break;
                            case AS_DIV: stk.top().number /= p1.number; break;
                            case AS_MOD: stk.top().number %= p1.number; break;

                            case AS_EQU: stk.top().number = stk.top().number == p1.number; break;
                            case AS_NOTEQU: stk.top().number = stk.top().number != p1.number; break;
                            case AS_GREAT: stk.top().number = stk.top().number > p1.number; break;
                            case AS_GREATEQ: stk.top().number = stk.top().number >= p1.number; break;
                            case AS_LESS: stk.top().number = stk.top().number < p1.number; break;
                            case AS_LESSEQ: stk.top().number = stk.top().number <= p1.number; break;
                        }
                        num--;
                    }
                    else {
                        if (num > 0) {
                            res.emplace_back(stk.top());
                            stk.pop();
                            num--;
                        }
                        res.push_back(t);
                    }
                }
                break;
            case TOKEN_COMMAND:
            {

                switch (t.number) {
                case SINL:
                case COSL:
                case MIN:
                case MAX:
                case ATAN: {
                    if (num >= 2) {
                        Token p1 = stk.top();
                        stk.pop();
                        switch (t.number) {
                        case SINL: stk.top().number = sin((t.number % 256) / 256.0 * 6.2831852) * 256 * p1.number; break;
                        case COSL: stk.top().number = cos((t.number % 256) / 256.0 * 6.2831852) * 256 * p1.number; break;
                        case ATAN: stk.top().number = p1.number; break;
                        case MIN: stk.top().number = (stk.top().number < p1.number) ? stk.top().number :p1.number; break;
                        case MAX: stk.top().number = (stk.top().number > p1.number) ? stk.top().number : p1.number; break;
                        }
                        num--;
                    }
                    else {
                        if (num > 1) {
                            Token p1 = stk.top();
                            stk.pop();
                            res.emplace_back(stk.top());
                            res.emplace_back(p1);
                            stk.pop();
                            num-=2;
                        }
                        else if (num > 0) {
                            res.emplace_back(stk.top());
                            stk.pop();
                            num--;
                        }
                        res.push_back(t);
                    }
                }break;
                case RND:
                    if (num > 1) {
                        Token p1 = stk.top();
                        stk.pop();
                        res.emplace_back(stk.top());
                        res.emplace_back(p1);
                        stk.pop();
                        num -= 2;
                    }
                    else if (num > 0) {
                        res.emplace_back(stk.top());
                        stk.pop();
                        num--;
                    }
                    res.push_back(t);
                    break;
                case NEG:
                    if (num > 0) {
                        Token& tk = stk.top();
                        tk.number = -tk.number;
                    }
                    else {
                        res.push_back(t);
                    }
                    break;
                }
            } break;
            case TOKEN_COMMA:
                if (num > 0) {
                    res.emplace_back(stk.top());
                    stk.pop();
                    num = 0;
                }
                break;
            }
        }
    }
    catch (...) {
        success = false;
    }
    if (success) {
        if (num > 0) {
            res.emplace_back(stk.top());
        }
        toks = res;
    }
    return success;
}

bool Token2ProcessedData(std::vector<Token>* to, const std::deque<Token>& toks, int reg) {
    bool success = true;
    if (toks.size() == 1 && toks[0].kind == TOKEN_NUMBER && reg != -1) {
        Token cmd;
        cmd.kind = TOKEN_COMMAND;
        cmd.number = MOVC;
        Token cnt;
        cnt.kind = TOKEN_INCOUNT;
        cnt.number = 2;
        to->emplace_back(cmd);
        to->emplace_back(cnt);
        Token rg;
        rg.kind = TOKEN_NUMBER;
        rg.number = reg;
        rg.advance = 1;
        to->emplace_back(rg);
        Token num;
        num.kind = TOKEN_NUMBER;
        num.number = toks[0].number;
        num.advance = 4;
        to->emplace_back(num);
        return true;
    }

    size_t size = toks.size();
    for (int i = 0; i < size; i++) {
        const auto& tok = toks[i];
        switch (tok.kind) {
        case TOKEN_NUMBER: {
            Token cmd;
            cmd.kind = TOKEN_COMMAND;
            cmd.number = PUSHC;

            Token cnt;
            cnt.kind = TOKEN_INCOUNT;
            cnt.number = 1;

            Token n = tok;
            n.advance = 4;

            //printf("Num: %d\n", n.number);
            to->emplace_back(cmd);
            to->emplace_back(cnt);
            to->emplace_back(n);
        }break;
        case TOKEN_REGISTER:
        {
            Token cmd;
            cmd.kind = TOKEN_COMMAND;
            cmd.number = PUSHR;

            Token cnt;
            cnt.kind = TOKEN_INCOUNT;
            cnt.number = 1;

            Token n = tok;
            n.kind = TOKEN_NUMBER;
            n.advance = 1;

            //printf("Reg: %s\n", n.pStr.c_str());
            to->emplace_back(cmd);
            to->emplace_back(cnt);
            to->emplace_back(n);
        }
            break;
        case TOKEN_ARITHS: {

            Token cmd;
            cmd.kind = TOKEN_COMMAND;
            Token cnt;
            cnt.kind = TOKEN_INCOUNT;
            cnt.number = 0;
            
            switch (tok.number) {
            case AS_ADD: cmd.number = ADD;
                //printf("+\n");
                break;
            case AS_SUB: cmd.number = SUB;
                //printf("-\n");
                break;
            case AS_MUL: cmd.number = MUL;
                //printf("*\n");
                break;
            case AS_DIV: cmd.number = DIV;
                //printf("/\n");
                break;
            case AS_MOD: cmd.number = MOD;
                //printf("%\n");
                break;

            case AS_EQU: cmd.number = EQUAL;
                //printf("==\n");
                break;
            case AS_NOTEQU: cmd.number = NOTEQ;
                //printf("!=\n");
                break;
            case AS_LESS: cmd.number = LESS;
                //printf("<\n");
                break;
            case AS_LESSEQ: cmd.number = LESSEQ;
                //printf("<=\n");
                break;
            case AS_GREAT: cmd.number = ABOVE;
                //printf(">\n");
                break;
            case AS_GREATEQ: cmd.number = ABOVEEQ;
                //printf(">=\n");
                break;
            }

            to->emplace_back(cmd);
            to->emplace_back(cnt);
        }
        break;
        case TOKEN_COMMAND: {
            Token cmd = tok;
            Token cnt;
            cnt.kind = TOKEN_INCOUNT;
            cnt.number = 0;
            switch (cmd.number) {
            case SINL:
                //printf("sinl\n");
                break;
            case COSL:
                //printf("cosl\n");
                break;
            case MIN:
                //printf("min\n");
                break;
            case MAX:
                //printf("max\n");
                break;
            case ATAN:
                //printf("atan\n");
                break;
            case RND:
                //printf("rnd\n");
                break;
            case NEG:
                //printf("neg\n");
                break;
            }
            to->emplace_back(cmd);
            to->emplace_back(cnt);
        }break;
        }
    }
    if (reg != -1) {
        Token cmd;
        cmd.kind = TOKEN_COMMAND;
        cmd.number = POPR;
        Token cnt;
        cnt.kind = TOKEN_INCOUNT;
        cnt.number = 1;
        to->emplace_back(cmd);
        to->emplace_back(cnt);
        Token rg;
        rg.kind = TOKEN_NUMBER;
        rg.number = reg;
        rg.advance = 1;
        to->emplace_back(rg);
    }

    return success;
}
