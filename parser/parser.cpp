#include <iostream>
#include "parser.h"

parser::parser(std::vector<std::string> tokens) {
	this->tokens = tokens;
}

void AST::add_node(std::string token) {
	AST *next_t;
	if(!root.empty()) {
		next_t->root = token;
		(left_node == nullptr) ? left_node = next_t : right_node = next_t;
		return;
	}
	root = token;
}

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
	return (token.find("(") == token.npos) ? false : true;
}

bool parser::tree_is_full(AST *single_t, std::vector<AST> *ast_vec_ptr) {
	return (!single_t->left_node->root.empty() && !single_t->right_node->root.empty() && !single_t->root.empty() && !ast_vec_ptr->empty()) ? true : false;
}

std::vector<AST> parser::parse_tree() {
	std::vector<AST> ast_vec;
	AST single_t, member;
	for(int i = 0; i < tokens.size(); i++) {
		if(tokens[i] == "\n") {
			ast_vec.push_back(single_t);
			single_t = AST(); // emptying tree for next iteration
			member = AST();
			continue;
		}
		if(single_t.root.empty()) {
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
		if(tree_is_full(&single_t, &ast_vec)) {
			(single_t.left_node == nullptr) ? single_t.left_node->add_node(tokens[i]) : single_t.right_node->add_node(tokens[i]);
		}
	}
	return ast_vec;
}
