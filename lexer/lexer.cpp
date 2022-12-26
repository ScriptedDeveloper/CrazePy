#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "lexer.h"

std::vector<std::string> lexer::get_tokens() {
	std::ifstream ifs(this->name);
	std::string token, line;
	std::vector<std::string> tokens;
	while(std::getline(ifs, line)) {
		for(char c : line) {
			if(std::isalpha(c)) {
				token.push_back(c);
				continue;
			} else if(c == '(' || c == ')') {
				continue; // ignoring parentesis
			}
			tokens.push_back(token); // functions dont work yet, if statements too
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

