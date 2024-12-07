#ifndef CALC2TOKEN_INCLUDED
#define CALC2TOKEN_INCLUDED

#include "edt_compile.hpp"
#include <deque>
#include <stack>

//Turn all those funky symbols into something meaningfull
bool CalcConvert (
	std::vector<Token>& from,
	std::vector<Token>* to,
	size_t& idx,
	KEYWORD_KIND proc,
	constant_map& const_map,
	bool save_ret
);

bool CalcConvertConst(
	std::vector<Token>& from,
	std::vector<Token>* to,
	size_t& idx,
	KEYWORD_KIND proc,
	constant_map& const_map,
	int& result
);

bool Calc2Token(
	std::vector<Token>& from,
	std::deque<Token>& to,
	size_t& idx,
	KEYWORD_KIND proc,
	constant_map& const_map,
	bool constant_exp,
	int& receiver,
	int num_args = 1
);

bool PresolveTokens(
	std::deque<Token>& toks
);

bool Token2ProcessedData(
	std::vector<Token>* to,
	const std::deque<Token>& toks,
	int reg
);

#endif