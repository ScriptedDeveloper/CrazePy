#include <iostream>
#include <exception>
#include <functional>
#include <memory>
#include <variant>
#include "parser.h"
	
parser::parser(std::vector<std::string> tokens_) : tokens(tokens_) {

}

AST::AST() : nodes() {
	root.clear();
}

ArgVector parser::calc_args(ArgVector &args) {
	int i = 0;
	for(auto x : args) {	
		if(args.size() <= 1) { 
			break;
		}
		if(is_operator(std::get<std::string>(x))) {
			std::string op = std::get<std::string>(x);
			auto x1 = std::atoi(std::get<std::string>(args[i - 1]).c_str()), x2 = std::atoi(std::get<std::string>(args[i + 1]).c_str());
			if(op == "+") {
				args[i] = std::to_string(x1 + x2);
			} else if(op == "-") {
				args[i] = std::to_string(x1 - x2);
			} else if(op == "/") {
				args[i] = std::to_string(x1 / x2);
			} else { // ugly but it works for now
				args[i] = std::to_string(x1 * x2);
			}
			
			args.erase(args.begin() + i + 1);
			args.erase(args.begin() + i - 1);
			break;
		}
		i++;
	}
	return args;
}

std::shared_ptr<AST> AST::add_node(std::string token, std::shared_ptr<AST> node) {
	/*
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
	*/
	auto node_temp = node;
	while(true) {
		if(node_temp->root.empty()) {
			node_temp->root = token;
			break;
		}
	//	node_temp = (node_temp->left_node == nullptr || node_temp->left_node->root.empty()) ? node_temp->left_node : node_temp->right_node;
		(node_temp->left_node == nullptr || node_temp->right_node == nullptr) ? allocate_nodes(node_temp) : void();
		if(node_temp->left_node->root.empty()) {
			node_temp = node_temp->left_node;
		} else {
			node_temp = node_temp->right_node;
		}
	}
	return node;
}

void AST::allocate_nodes(std::shared_ptr<AST> ptr) {
	if(ptr == nullptr) {
		left_node = std::make_shared<AST>();
		right_node = std::make_shared<AST>();
		return;
	}
	ptr->left_node = std::make_shared<AST>();
	ptr->right_node = std::make_shared<AST>();
}

ArgVector AST::get_params(ArgVector params, std::shared_ptr<AST> root_ptr) {
	auto temp_ptr = root_ptr, previous_ptr = temp_ptr;	
	if(root.empty() || root_ptr == nullptr) {
		return {}; // returning nothing since invalid AST
	}
	while(params.size() <= root_ptr->nodes) {
		try {
			if(!temp_ptr->read) { 
				params.push_back(temp_ptr->root);
				temp_ptr->read = true;
			} else if(params.size() < 1) {
				params.push_back(temp_ptr->root); // preventing segfault by this
				temp_ptr->read = true;
			}
		} catch(...) {}
		if(temp_ptr->left_node != nullptr && !temp_ptr->left_node->root.empty()) {
			previous_ptr = temp_ptr;
			temp_ptr = temp_ptr->left_node;
		} else {
			temp_ptr = (temp_ptr->right_node == nullptr) ? previous_ptr->right_node : temp_ptr->right_node;
		}
	}
	return params;
}

 bool parser::is_operator(std::string token) {
	const std::vector<std::string> ops = {"=", "/", "+", "-", "*"}; // treating char as string because token is string too
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

bool parser::tree_is_full(std::shared_ptr<AST> single_t) {
	if(single_t != nullptr && single_t->left_node != nullptr && single_t->right_node != nullptr) {
		return (!single_t->left_node->root.empty() && !single_t->right_node->root.empty() && !single_t->root.empty()) ? true : false;
	}
	return false;
}

std::vector<std::shared_ptr<AST>> parser::create_tree() {
	std::vector<std::shared_ptr<AST>> ast_vec{};
	auto single_t = std::make_shared<AST>();
	single_t->allocate_nodes();
	int i = 0;
	for(auto token : tokens) {
		single_t = (single_t == nullptr) ? std::make_shared<AST>() : single_t;
		if(token == "\b") {
			single_t->nodes = i;
			ast_vec.push_back(std::move(single_t));
			single_t.reset();
			i = 0;
			continue;
		}
		if(single_t != nullptr && single_t->root.empty()) {
			single_t->root = token; // setting function as root, if there is one
			i++;
			continue;
		}	
		if(tree_is_full(single_t)) {
			single_t->add_node(token, single_t);
			i++;
			continue;
		}
		single_t->add_node(token, single_t);
		i++;

	}
	return ast_vec;
}

void parser::parse_tree(std::vector<std::shared_ptr<AST>> tree, std::shared_ptr<FunctionMap> FMap) {
	for(std::shared_ptr<AST> s_tree : tree) {
		if(is_function(s_tree->root)) {
			std::string f_name = s_tree->root;
			ArgVector temp_args, args;
			auto next_tree = s_tree;
			temp_args = next_tree->get_params(args, s_tree);
			args = temp_args;
			next_tree = (next_tree == s_tree->left_node || next_tree == nullptr) ? s_tree->right_node : s_tree->left_node;
			args.erase(args.begin()); // emptying function call
			args.erase(std::remove_if(args.begin(), args.end(), [](const auto& elm) {
				if(std::holds_alternative<std::string>(elm)) {
					return (std::get<std::string>(elm) == "") ? true : false;
				}
				return false;
					}));
			temp_args = args;
			do {
				args = calc_args(args);
			} while(args.size() > 1 && temp_args != args);
			call_function(FMap, f_name, args);
		}
	}
}

bool parser::is_variant_int(std::variant<std::string, int, bool, double, float> i) {
	try {
		std::get<int>(i);
	} catch(std::bad_variant_access const&) {
		return false;
	}
	return true;
}

void parser::init_FMap(std::shared_ptr<FunctionMap> FMap) { // have to hardcode the functions, this is gonna be ugly
	(*FMap)["print()"] = (std::function<void(ArgVector &args)>)[](ArgVector &args) {
		for(auto i : args) {
			std::string str{};
			bool newline{};
			try {
				str = std::get<std::string>(i);
				newline = (std::get<std::string>(i).find("\\n") == std::string::npos) ? false : true;
			} catch(const std::bad_variant_access&) {}
			std::visit([=](auto &arg) {
				if(str.empty()) {
					std::cout << arg;	
					if(is_variant_int(arg)) {
						std::cout << std::endl; // printing newline for numbers, otherwise zsh wont output them
					}
				} else {
					if(newline) {
						for(size_t i_ = 0; i_ < str.length() - 2; i_++) {
							std::cout << str[i_];
						}
						std::cout << std::endl;
					} else {
						std::cout << arg;
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
