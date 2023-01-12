#pragma once
#include <iostream>
#include <vector>
#include <variant>

using ArgVector = std::vector<std::variant<std::string, int, bool, double, float, char>>;

class lexer {
	private:
		void add_token(ArgVector &curr_tokens, std::string &token);
	public:
		std::string name;
		ArgVector tokens;
		lexer(std::string filename);
		ArgVector get_tokens(int EIP = 0);
};
