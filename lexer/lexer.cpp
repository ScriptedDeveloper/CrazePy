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

ArgVector lexer::get_tokens(int EIP) {
	std::ifstream ifs(this->name);
	std::string token, line;
	ArgVector curr_tokens;
	bool is_eip = (EIP > 0) ? true : false, is_code_block = false;
	while(std::getline(ifs, line)) {
		if(is_eip && line.find("}") != line.npos)
			break; //  end of if block
		if(is_code_block) { // skip code block
			is_code_block = (line.find("}") != line.npos) ? false : true;
			continue;
		}
		char last_c = (line.length() <= 1) ? line[0] : line[line.length() - 1];
		(last_c != ';' && last_c != '{' && last_c != '}'
		 && last_c != '\0' && last_c != ' ') ? exit(1) : void(); // right now just simply exit, will do error handeling properly l8ter
		bool is_string = false;
		if(EIP > 0) {
			EIP--;
			continue;
		}
		if((EIP == 0 && line.find("{") != line.npos)) { // dont get tokens of code blocks (if statements etc)
			is_code_block = true;
		}
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
			else if(is_string) {
				token.push_back(c);
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

