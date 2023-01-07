#include <iostream>
#include <exception>
#include <functional>
#include <memory>
#include <variant>
#include "parser.h"
	
parser::parser(ArgVector &tokens_) : vmap(), tokens(tokens_) {

}

AST::AST() : root(), nodes() {
}

ArgVector parser::calc_args(ArgVector &args) {
	int i = 0;
	for(auto x : args) {	
		if(args.size() <= 1) { 
			break;
		}
		if(std::holds_alternative<std::string>(x) && is_operator(std::get<std::string>(x))) {
			std::string op = std::get<std::string>(x);
			auto x1 = std::get<int>((args[i - 1])), x2 = std::get<int>(args[i + 1]);
			if(op == "+") {
				args[i] = x1 + x2;
			} else if(op == "-") {
				args[i] = x1 - x2;
			} else if(op == "/") {
				args[i] = x1 / x2;
			} else { // ugly but it works for now
				args[i] = x1 * x2;
			}
			args.erase(args.begin() + i + 1);
			args.erase(args.begin() + i - 1);
			break;
		} else {
			if(std::holds_alternative<std::string>(x)) {
				auto str = std::get<std::string>(x);
				if(!std::all_of(str.begin(), str.end(), ::isalnum)) {
					args[i] = ""; // emptying that member
				} 
			}
		}
		i++;
	}
	return args;
}

std::shared_ptr<AST> AST::add_node(std::variant<std::string, int, bool, double, float, char> token, std::shared_ptr<AST> node) {
	auto node_temp = node;
	while(true) {
		auto root_str = node_temp->root;
		if(node_temp->root.valueless_by_exception() || (std::holds_alternative<std::string>(root_str) && std::get<std::string>(root_str).empty())) {
			node_temp->root = token;
			break;
		}
		(node_temp->left_node == nullptr || node_temp->right_node == nullptr) ? allocate_nodes(node_temp) : void();
		auto left_node_str = node_temp->left_node->root;
		if(left_node_str.valueless_by_exception() || (std::holds_alternative<std::string>(left_node_str) && std::get<std::string>(left_node_str).empty())) {
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


ArgVector AST::get_params(ArgVector &params, std::shared_ptr<AST> root_ptr) {
	auto temp_ptr = root_ptr, previous_ptr = temp_ptr;	
	if(root.valueless_by_exception() || root_ptr == nullptr) {
		return {}; // returning nothing since invalid AST
	}
	while(params.size() < root_ptr->nodes) {
		try {
			if(!temp_ptr->read) { 
				params.push_back(temp_ptr->root);
				temp_ptr->read = true;
			} else if(params.size() < 1) {
				params.push_back(temp_ptr->root); // preventing segfault by this
				temp_ptr->read = true;
			}
		} catch(...) {}
		if(temp_ptr->left_node != nullptr && !temp_ptr->left_node->root.valueless_by_exception()) {
			previous_ptr = temp_ptr;
			temp_ptr = temp_ptr->left_node;
		} else {
			temp_ptr = (temp_ptr->right_node == nullptr) ? previous_ptr->right_node : temp_ptr->right_node;
		}
	}	
	parser::remove_space(params, ' '); 
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

bool parser::is_var(std::string token) {
	return token == "var";
}

bool parser::tree_is_full(std::shared_ptr<AST> single_t) {
	if(single_t != nullptr && single_t->left_node != nullptr && single_t->right_node != nullptr) {
		return (!single_t->left_node->root.valueless_by_exception() && !single_t->right_node->root.valueless_by_exception() && !single_t->root.valueless_by_exception()) ? true : false;
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
		if(std::holds_alternative<std::string>(token) && std::get<std::string>(token) == "\b") {
			single_t->nodes = i;
			ast_vec.push_back(std::move(single_t));
			single_t.reset();
			i = 0;
			continue;
		}
		single_t->add_node(token, single_t);
		i++;

	}
	return ast_vec;
}

void parser::parse_tree(std::vector<std::shared_ptr<AST>> tree, std::shared_ptr<FunctionMap> FMap) {
	ArgVector temp_args, args;
	for(std::shared_ptr<AST> s_tree : tree) {
		auto is_str = std::holds_alternative<std::string>(s_tree->root);
		std::string root;
		root = (is_str) ? std::get<std::string>(s_tree->root) : root;
		if(is_function(root)) {
			std::string f_name = root;
			args = s_tree->get_params(args, s_tree);
			args.erase(args.begin()); // emptying function call
			temp_args = args;
			do {
				args = calc_args(args);
			} while(args.size() > 1 && temp_args != args);
			call_function(FMap, f_name, args);
			args.clear();
		} else if(is_var(root)) {
			temp_args = args;
			args = s_tree->get_params(args, s_tree);
			remove_space(args, ' '); // removes the whitespaces between vars definition
			temp_args = calc_args(temp_args);
			vmap[std::get<std::string>(args[1])] = (temp_args.empty()) ? args[3] : temp_args[0];
			// either making it to the third member (the value of var) or assigning it to the only member of vector
			args.clear();
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
