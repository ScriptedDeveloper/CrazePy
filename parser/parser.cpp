#include <iostream>
#include <memory>
#include <functional>
#include <variant>
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

ArgVector AST::get_params(ArgVector params) {
	if(root.empty()) {
		return {}; // returning nothing since invalid AST
	}
	(left_node == nullptr) ? void() : params.push_back(left_node->root);
	(right_node == nullptr) ? void() : params.push_back(right_node->root);
	return params;
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
		if(tokens[i] == "\b") {
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

void parser::parse_tree(std::vector<std::shared_ptr<AST>> tree, std::shared_ptr<FunctionMap> FMap) {
	for(std::shared_ptr<AST> s_tree : tree) {
		std::vector<std::string> args;
		if(is_function(s_tree->root)) {
			std::string f_name = s_tree->root;
			ArgVector args, temp_args;
			auto next_tree = s_tree;
			do {
				temp_args = next_tree->get_params(args);
				if(temp_args.empty()) {
					break;
				}
				args = temp_args;
				next_tree = (next_tree == s_tree->left_node || next_tree == nullptr) ? s_tree->right_node : s_tree->left_node;
			} while(true);
			call_function(FMap, f_name, args);
		}
	}
}

void parser::init_FMap(std::shared_ptr<FunctionMap> FMap) { // have to hardcode the functions, this is gonna be ugly
	(*FMap)["print()"] = (std::function<void(ArgVector &args)>)[](ArgVector &args) {
		for(auto i : args) {
			std::string str;
			try {
				str = std::get<std::string>(i);
			} catch(const std::bad_variant_access&) {}
			bool newline = (std::get<std::string>(i).find("\\n") == std::string::npos) ? false : true;
			std::visit([=](auto &arg) {
				if(str.empty()) {
					std::cout << arg;
				} else {
					if(newline) {
						for(int i = 0; i< str.length() - 2; i++) {
							std::cout << str[i];
						}
						std::cout << std::endl;
					}
					else {
						std::cout << str;
					}
				}
			}, i); // btw, this is what happens if you write code under pressure during New year celebrations LOL
		}
		
	};
}

void parser::init_parser() {
	auto FMap = std::make_shared<FunctionMap>();
	init_FMap(FMap); // adding function pointers to Map
	std::vector<std::shared_ptr<AST>> tree = create_tree();
	parse_tree(tree, FMap);
}
