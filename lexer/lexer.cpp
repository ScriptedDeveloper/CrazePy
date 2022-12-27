#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "lexer.h"
#include "../parser/parser.h"

lexer::lexer(std::string filename) {
	this->name = filename;
}

std::vector<std::string> lexer::get_tokens() {
	std::ifstream ifs(this->name);
	std::string token, line;
	std::vector<std::string> tokens;
	while(std::getline(ifs, line)) {
		for(char c : line) {
			std::string pot_operator = std::string(1, c);
			if(std::isalpha(c) || c == '(' || std::isdigit(c)) {
				token.push_back(c);
				continue;
			} else if(parser::is_operator(pot_operator)) {
				tokens.push_back(pot_operator);
			}
			(token.empty()) ? void() : tokens.push_back(token); // functions dont work yet, if statements too
			token.clear();
		}
		tokens.push_back("\n"); // letting parser know that statement has ended
	}
	if(tokens[tokens.size() - 1] == "\n") {
		tokens.pop_back();
		tokens.push_back(token);
		tokens.push_back("\n");
	}
	else {
		tokens.push_back(token);
	}
	this->tokens = tokens;
	return tokens;
}

