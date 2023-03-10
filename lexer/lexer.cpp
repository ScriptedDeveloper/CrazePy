#include "lexer.h"
#include "../parser/parser.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

lexer::lexer(std::string filename) : name(filename), tokens({}) {}

void lexer::add_token(ArgVector &curr_tokens, std::string &token) {
	std::pair<bool, bool> is_bool = {
		(token == "true" || token == "false") ? true : false,
		// clang-format off
	(token == "true") ? true : false}; // first pair says if is bool or not, second pair then tells if its true, or false
	// clang-format on
	if (token.empty())
		return; // invalid input
	if (std::all_of(token.begin(), token.end(), ::isdigit)) {
		curr_tokens.push_back(std::stoi(token));
		token.clear();
	} else if (is_bool.first) {
		curr_tokens.push_back(is_bool.second);
		token.clear();
	} else {
		curr_tokens.push_back(token);
		token.clear();
	}
}

ArgVector lexer::get_tokens() {
	std::ifstream ifs(this->name);
	std::string token, line;
	ArgVector curr_tokens;
	while (std::getline(ifs, line)) {
		if (line.find(";") != line.npos && line.find("{") != line.npos && line.find("}") != line.npos &&
			line.find("\0") != line.npos && line.find(" ") != line.npos)
			exit(1); // right now just simply exit, will do error handeling properly l8ter
		bool is_string = false, is_not_equal_to = false;
		for (char c : line) {
			std::string pot_operator = std::string(1, c);
			auto is_op = parser::is_operator(pot_operator, true);
			if (is_not_equal_to) {
				token.push_back(c);
				add_token(curr_tokens, token);
				continue;
			}
			if ((std::isalpha(c) || std::isdigit(c) || c == '(' || c == '\\') && !is_string) {
				if (c == '(') {
					token.append(pot_operator + ")"); // using variable that is not designed for this purpose
					add_token(curr_tokens, token);
					continue;
				}
				if (token != "var") { // might as well add later a array for keywords or something like that
					token.append(std::string(1, c));
					continue;
				}
			} else if (c == '"') { // string began/ended
				is_string = (is_string) ? false : true;
				if (is_string)
					continue;
				token.append("\\s"); // marking var as string
			} else if (is_string || c == '!') {
				if (c == '!' && !is_string) {
					add_token(curr_tokens, token);
					is_not_equal_to = true;
				}
				token.push_back(c);
				continue;
			} else if (c == '#')
				break; // current line is a comment
			else if (is_op || c == ' ') {
				add_token(curr_tokens, token);
				curr_tokens.push_back(pot_operator);
				continue;
			} else if (c == '}') {
				token.push_back(c); // adding code new code block indication
			}
			add_token(curr_tokens, token);
		}
		if (!token.empty())
			add_token(curr_tokens,
					  token); // adding last token to vector, wasn't added. probably because of empty comment
		curr_tokens.push_back("\b");
		// letting parser know that statement has ended, using backslash because script can contain \n
	}
	if (!token.empty()) {
		auto is_str = std::holds_alternative<std::string>(tokens[tokens.size() - 1]);
		if (is_str && std::get<std::string>(tokens[tokens.size() - 1]) == "\n") {
			curr_tokens.pop_back();
			curr_tokens.push_back(token);
			curr_tokens.push_back("\n");
		} else {
			curr_tokens.push_back(token);
		}
	}
	this->tokens = curr_tokens;
	return tokens;
}
