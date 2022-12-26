#include <iostream>
#include "parser.h"

bool parser::is_operator(std::string token) {
	const std::vector<std::string> ops = {"=", "/", "+", "-"}; // treating char as string because token is string too
	for(std::string op : ops) {
		if(op == token) {
			return true;
		}
	}
	return false;
}

bool parser::is_function(std::string token) {
	if(token.find("(", token.size()) != token.size()) {
		return true;
	}
	return false;
}

void parser::parse_tree() {
	std::vector<AST> ast_vec;
	AST single_t;
	for(int i = 0; i < tokens.size(); i++) {
		if(tokens[i] == "\n") {
			ast_vec.push_back(single_t);
			single_t = AST(); // emptying tree for next iteration
			continue;
		}
		if(!single_t.root.empty()) {
			if(is_function(tokens[i])) {
				tokens[i].pop_back();
				single_t.root = tokens[i]; // setting function as root, if there is one
				continue;
			}
			else if(is_operator(tokens[i])) { // setting operators as root
				single_t.root = tokens[i];
				continue;
			}
		}
		(single_t.left_node.empty()) ? single_t.left_node = tokens[i] : single_t.right_node = tokens[i]; // inserting to right/left node of AST
	}
}
