#include <iostream>
#include <exception>
#include <functional>
#include <memory>
#include <variant>
#include "parser.h"
	
parser::parser(ArgVector &tokens_, std::string &fname) : file_name(), tokens(tokens_)  {
	file_name = fname;
}

AST::AST() : root(), nodes() {
}

bool parser::has_one_value(const ArgVector &args) {
	int i = 0;
	for(auto x : args) {
		if(std::holds_alternative<int>(x))
			i++;
		if(i == 2)
			return false;
	}
	return true;
}

ArgVector parser::calc_args(ArgVector &args) {
	int i = 0;
	if(has_one_value(args))
		return args; // only has one value, no need to do anything
	for(auto x : args) {	
		if(args.size() <= 1) { 
			break;
		}
		if(std::holds_alternative<std::string>(x) && is_operator(std::get<std::string>(x))) {
			std::string op = std::get<std::string>(x);
			int x1, x2;
			try {
				x1 = std::get<int>((args[i - 1])), x2 = std::get<int>(args[i + 1]);
			} catch(const std::bad_variant_access&) {
				return args; // no arithmatic operation involved
			}
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


ArgVector AST::get_params(ArgVector &params, std::shared_ptr<AST> root_ptr, VarMap *vmap) {
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
	int it = 0;
	for(auto i : params) {
		if(std::holds_alternative<std::string>(i)) {
			params[it] = parser::replace_variable(i, *vmap);
		}
		it++;
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

bool parser::end_of_code_block(std::string token) {
	return (token.find("}") == std::string::npos) ? false : true;
}

bool parser::compare_values(ArgVector &args) {
	if(args.size() <= 2) {
		return false; // wrong if statement
	}
	args.erase(args.begin());
	args.erase(args.begin() + 1, args.begin() + 3); // removing first 2 if and ==
	auto val = args[0];
	int it = 0;
	for(auto i : args) {
		if(i == val && it > 0)
			return true;
		it++;
	}
	return false;
}

bool parser::is_if_statement(std::string token) {
	return token == "if" || token == "else" || token == "elif";
}

bool parser::is_function_declaration(std::string token) {
	return token == "def"; 
}

bool parser::is_function(std::string token) {
	return (token.find("(") == token.npos) ? false : true;
}

bool parser::is_var(std::string token) {
	return token == "var";
}

bool parser::contains_args(ArgVector &args, std::variant<std::string, int, bool, double, float, char> keyword) {
	for(auto i : args) {
		if(i == keyword)
			return true;
	}
	return false;
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

void parser::get_function_name(std::string &func) {
	func = func.substr(0, func.find("("));
}

void parser::save_function(std::shared_ptr<FunctionMap> FMap, std::shared_ptr<AST> s_tree, int i) {
	auto func_name = std::get<std::string>(s_tree->right_node->root);
	get_function_name(func_name);
	(*FMap)[func_name] = i + 1; // putting function name in Map, so later it can get called by that name, i++ so it knows which line the function starts
}

void parser::parse_tree(std::vector<std::shared_ptr<AST>> tree, std::shared_ptr<CPPFunctionMap> CPPMap, int i, bool single_function, VarMap vmap_global) { 
	ArgVector temp_args, args;
	auto PyFMap = std::make_shared<FunctionMap>();
	std::pair<bool, bool> is_if_is_else = {false, false}; // first : is_if, second : is_else
	bool is_str = false, is_elif = false, skip = false; // this is for elif statements, not like above for else statements, skip is for skipping for example a function block
	VarMap vmap_block, vmap_all; // vmap_block is for local variables declared in a if/else block or later functions, see get_vmap() for vmap_all usage
	std::array<VarMap*, 2> vmap_arr = {&vmap_block, &vmap_global};
	std::map<std::string, std::shared_ptr<AST>> declared_funcs; 	
	std::string root;
	for(auto s_tree : tree) {
		if(i > 1 && single_function) {
			i--;
			continue; // jumping to line i, so it goes to function for example
		} else if(skip) {
			skip = (end_of_code_block(std::get<std::string>(s_tree->root))) ? false : true; // once end of function block arrives, dont skip anymore
			continue;
		}
		if(single_function && end_of_code_block(std::get<std::string>(s_tree->root)))
			break; // only execute that function and return
		vmap_all = get_vmap(vmap_arr);
		args = s_tree->get_params(args, s_tree,  &vmap_all);
		if(is_if_is_else.first) {
			is_if_is_else.first = (contains_args(args, "}")) ? false : true;
		} else if(is_if_is_else.second) {
			is_elif = contains_args(args, "elif");
			if(contains_args(args, "}")) {
				is_if_is_else.second = false;
				vmap_all = *vmap_arr[1];
			}
			if(!is_elif) {
				args.clear();
				continue; // not continuing if its else if statement because it'd skip it
			}
			is_str = true; // using is_str to reduce code, causing root str to say as is
			root = "elif";  
			erase_key(args, "}");
			// might be shit code.. but im sleepy
		}
		if(!is_if_is_else.first && !is_if_is_else.second) {
			vmap_block.clear();
			vmap_all = *vmap_arr[1];
		}
		is_str =  (is_str) ? std::holds_alternative<std::string>(s_tree->root) : is_str;
		root = (is_str) ? root : std::get<std::string>(s_tree->root);
		is_str = false; // setting back to default, so next line root is set properly
		if(is_function(root)) {
			std::string f_name = root;
			args.erase(args.begin()); // emptying function call
			temp_args = args;
			do {
				calc_args(args);
			} while(args.size() > 1 && temp_args != args);
			call_function(CPPMap, f_name, args, PyFMap, tree, vmap_global);
			temp_args.clear();
		} else if(is_var(root)) {
			remove_space(args, ' '); // removes the whitespaces between vars definition
			args = calc_args(args);
			if(single_function || (is_if_is_else.first || is_if_is_else.second)) // always put vars in vmap_block if its only a single function
				vmap_block[std::get<std::string>(args[1])] = (temp_args.empty()) ? args[3] : temp_args[0];
			else
				vmap_global[std::get<std::string>(args[1])] = (temp_args.empty()) ? args[3] : temp_args[0];
			// either making it to the third member (the value of var) or assigning it to the only member of vector
		} else if(is_function_declaration(root)) {
			save_function(PyFMap, s_tree, i);
			skip = true; // skips next function block, not called yet
		} else if(is_if_statement(root)) {
			if(args.empty())
				exit(1); // exitting, invalid if statement.
			calc_args(args);
			if(!compare_values(args) && !is_if_is_else.second) { 
				is_if_is_else = {false, true}; // setting else_if true, because if statement is false
			} else { 
				is_if_is_else = {true, false}; // opposite here
			}
		}
		i = (single_function) ? i : i + 1; // not incrementing if its single_function because it's already decrementing (line 230)
		args.clear();
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

void parser::init_FMap(std::shared_ptr<CPPFunctionMap> FMap) { // have to hardcode the functions, this is gonna be ugly
	(*FMap)["print"] = (std::function<void(ArgVector &args)>)[](ArgVector &args) {
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
	auto CPPFMap = std::make_shared<CPPFunctionMap>();
	init_FMap(CPPFMap); // adding function pointers to Map
	std::vector<std::shared_ptr<AST>> tree = create_tree();
	parse_tree(tree, CPPFMap);
}
