#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "lexer.h"
#include "../parser/parser.h"

lexer::lexer(std::string filename) : name(filename), tokens({}) {
	
}

void lexer::add_token(std::vector<std::string> &curr_tokens, std::string &token) {
	(token.empty()) ? void() : curr_tokens.push_back(token); // functions dont work yet, if statements too
	token.clear();
}

std::vector<std::string> lexer::get_tokens() {
	std::ifstream ifs(this->name);
	std::string token, line;
	std::vector<std::string> curr_tokens;
	while(std::getline(ifs, line)) {
		(line[line.length() - 1] != ';') ? exit(1) : void(); // right now just simply exit, will do error handeling properly l8ter
		for(char c : line) {
			std::string pot_operator = std::string(1, c);
			if(std::isalpha(c) || std::isdigit(c) || c == '(' || c == '\\') {
				if(c == '(')  {
					token.append(pot_operator + ")"); // using variable that is not designed for this purpose
					add_token(curr_tokens, token);
					continue;
				} 
				token.push_back(c);
				(token == "var") ? add_token(curr_tokens, token) : void();
				continue;
			} else if(parser::is_operator(pot_operator) || c == ' ') {
				add_token(curr_tokens, token);
				curr_tokens.push_back(pot_operator);
				continue;
			}
			add_token(curr_tokens, token);
		}
		curr_tokens.push_back("\b"); // letting parser know that statement has ended, using backslash because script can contain \n
	}
	if(!token.empty()) {
		if(tokens[tokens.size() - 1] == "\n") {
			curr_tokens.pop_back();
			curr_tokens.push_back(token);
			curr_tokens.push_back("\n");
		}
		else {
			curr_tokens.push_back(token);
		}
	}
	this->tokens = curr_tokens;
	return tokens;
}

