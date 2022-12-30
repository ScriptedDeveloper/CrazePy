#include <iostream>
#include <memory>
#include <functional>
#include "parser.h"

parser::parser(std::vector<std::string> tokens) {
	this->tokens = tokens;
}

AST::AST() {
	root.clear();
}
void AST::add_node(std::string token) {
	std::shared_ptr<AST> next_t = std::make_shared<AST>();
	if(!root.empty()) {
		next_t->root = token; 
		(left_node == nullptr) ? allocate_nodes() : void();
		if(left_node->root.empty()) {
			left_node.reset();
			left_node = std::move(next_t);
			return;
		}
		right_node.reset();
		right_node = std::move(next_t);
		return;
	}
	root = token;
}

void AST::allocate_nodes() {
	left_node = std::shared_ptr<AST>(new AST());
	right_node = std::shared_ptr<AST>(new AST());
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

bool parser::tree_is_full(std::shared_ptr<AST> single_t, std::vector<std::shared_ptr<AST>> *ast_vec_ptr) {
	if(single_t != nullptr && single_t->left_node != nullptr && single_t->right_node != nullptr) {
		return (!single_t->left_node->root.empty() && !single_t->right_node->root.empty() && !single_t->root.empty()) ? true : false;
	}
	return false;
}

std::vector<std::shared_ptr<AST>> parser::create_tree() {
	std::vector<std::shared_ptr<AST>> ast_vec{};
	auto single_t = std::make_shared<AST>();
	single_t->allocate_nodes();
	for(int i = 0; i < tokens.size(); i++) {
		single_t = (single_t == nullptr) ? std::make_shared<AST>() : single_t;
		if(tokens[i] == "\n") {
			ast_vec.push_back(std::move(single_t));
			continue;
		}
		if(single_t != nullptr && single_t->root.empty()) {
		//	if(is_function(tokens[i])) {
				single_t->root = tokens[i]; // setting function as root, if there is one
				continue;
		//	}
			/*
			else if(is_operator(tokens[i])) { // setting operators as root
				single_t->root = tokens[i];
				continue;
			}*/
		}	
		if(tree_is_full(single_t, &ast_vec)) {
			(single_t->left_node->left_node == nullptr) ? single_t->left_node->add_node(tokens[i]) : single_t->right_node->add_node(tokens[i]);
			continue;
		}
		single_t->add_node(tokens[i]);

	}
	return ast_vec;
}

void parser::parse_tree(std::vector<std::shared_ptr<AST>> tree) {
	for(std::shared_ptr<AST> s_tree : tree) {
		std::vector<std::string> args;
		if(is_function(s_tree->root)) {
			while(s_tree->left_node != nullptr && s_tree->right_node != nullptr) {
				// continuing later
			}
		}
	}
}

void parser::init_FMap(std::shared_ptr<FunctionMap> FMap) { // have to hardcode the functions, this is gonna be ugly
	(*FMap)["print"] = (std::function<void(const std::string&)>)[](const std::string& s) {
		std::cout << s << std::endl;
	};
}

void parser::init_parser() {
	auto FMap = std::make_shared<FunctionMap>();
	init_FMap(FMap); // adding function pointers to Map
	std::vector<std::shared_ptr<AST>> tree = create_tree();
}
