#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "lexer.h"
#include "../parser/parser.h"

lexer::lexer(std::string filename) : name(filename), tokens({}) {
	
}

void lexer::add_token(ArgVector &curr_tokens, std::string &token) {
	if(token.empty())
		return; // invalid input
	if(std::all_of(token.begin(), token.end(), ::isdigit)) {
		curr_tokens.push_back(std::stoi(token));
		token.clear();
		return;
	}
	curr_tokens.push_back(token);
	token.clear();
}

ArgVector lexer::get_tokens() {
	std::ifstream ifs(this->name);
	std::string token, line;
	ArgVector curr_tokens;
	while(std::getline(ifs, line)) {
		if(line.find(";") != line.npos && line.find("{") != line.npos && line.find("}") != line.npos
		 && line.find("\0") != line.npos && line.find(" ") != line.npos)
			exit(1); // right now just simply exit, will do error handeling properly l8ter
		bool is_string = false;		
		for(char c : line) {
			std::string pot_operator = std::string(1, c);
			if((std::isalpha(c) || std::isdigit(c) || c == '(' || c == '\\') && !is_string) {
				if(c == '(')  {
					token.append(pot_operator + ")"); // using variable that is not designed for this purpose
					add_token(curr_tokens, token);
					continue;
				}
				if(token != "var") { // might as well add later a array for keywords or something like that
					token.append(std::string(1, c));
					continue;
				}
			}
			else if(c == '"') { // string began/ended
				is_string = (is_string) ? false : true;
				if(is_string)
					continue;
				token.append("\\s"); // marking var as string
			}
			else if(is_string || c == '!') {
				token.push_back(c);
				continue;
			} else if(c == '#')
				break; // current line is a comment
			else if(parser::is_operator(pot_operator) || c == ' ') {
				add_token(curr_tokens, token);
				curr_tokens.push_back(pot_operator);
				continue;
			} else if(c == '}') {
				token.push_back(c); // adding code new code block indication
			}
			add_token(curr_tokens, token);
		}
		(token.empty()) ? void() : add_token(curr_tokens, token);  // adding last token to vector, wasn't added. probably because of empty comment
		curr_tokens.push_back("\b"); // letting parser know that statement has ended, using backslash because script can contain \n
	}
	if(!token.empty()) {
		auto is_str = std::holds_alternative<std::string>(tokens[tokens.size() - 1]);
		if(is_str && std::get<std::string>(tokens[tokens.size() - 1]) == "\n") {
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

