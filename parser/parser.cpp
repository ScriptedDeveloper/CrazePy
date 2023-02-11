#include "parser.h"
#include <functional>
#include <iostream>
#include <memory>
#include <variant>

parser::parser(ArgVector &tokens_, std::string &fname) : param_map(), file_name(), tokens(tokens_) {
	file_name = fname;
}

AST::AST() : root(), nodes() {}

bool parser::has_one_value(const ArgVector &args) {
	int i = 0;
	for (auto x : args) {
		if (std::holds_alternative<int>(x))
			i++;
		if (i == 2)
			return false;
	}
	return true;
}

ArgVector parser::calc_args(ArgVector &args) {
	ArgVector temp_args;
	while (true) {
		temp_args = args;
		size_t i = 0;
		bool no_equation = true; // no_equation checks whether it has one or not
		long i_ = 0;
		if (has_one_value(args))
			return args; // only has one value, no need to do anything
		for (auto x : args) {
			if (args.size() <= 1) {
				break;
			}
			if (std::holds_alternative<std::string>(x) && is_operator(std::get<std::string>(x))) {
				no_equation = false;
				std::string op = std::get<std::string>(x);
				int x1, x2;
				try {
					x1 = std::get<int>((args[i + 1])), x2 = std::get<int>(args[i - 1]);
				} catch (const std::bad_variant_access &) {
					return args; // no arithmatic operation involved
				}
				if (op == "+") {
					args[i - 1] = x1 + x2;
				} else if (op == "-") {
					args[i - 1] = x1 - x2;
				} else if (op == "/") {
					args[i - 1] = x1 / x2;
				} else if (op == "*") { // ugly but it works for now
					args[i - 1] = x1 * x2;
				} else {
					return args; // no equation left
				}
				for (int it = 0; it <= 1; it++)
					args.erase(args.begin() + i_);
				if (args == temp_args)
					return args; // nothing has changed, return!
				break;
			}
			i++;
			i_++;
		}
		if (no_equation)
			break; // nothing's there, return!
	}
	return args;
}

std::shared_ptr<AST> AST::add_node(AnyVar token, std::shared_ptr<AST> node) {
	auto node_temp = node;
	while (true) {
		auto root_str = node_temp->root;
		if (node_temp->root.valueless_by_exception() ||
			(std::holds_alternative<std::string>(root_str) && std::get<std::string>(root_str).empty())) {
			node_temp->root = token;
			break;
		}
		(node_temp->left_node == nullptr || node_temp->right_node == nullptr) ? allocate_nodes(node_temp) : void();
		auto left_node_str = node_temp->left_node->root;
		if (left_node_str.valueless_by_exception() ||
			(std::holds_alternative<std::string>(left_node_str) && std::get<std::string>(left_node_str).empty())) {
			node_temp = node_temp->left_node;
		} else {
			node_temp = node_temp->right_node;
		}
	}
	return node;
}

void AST::allocate_nodes(std::shared_ptr<AST> ptr) {
	if (ptr == nullptr) {
		left_node = std::make_shared<AST>();
		right_node = std::make_shared<AST>();
		return;
	}
	ptr->left_node = std::make_shared<AST>();
	ptr->right_node = std::make_shared<AST>();
}

ArgVector AST::get_params(ArgVector &params, std::shared_ptr<AST> root_ptr, VarMap *vmap, bool free) {
	auto temp_ptr = root_ptr, previous_ptr = temp_ptr;
	if (root.valueless_by_exception() || root_ptr == nullptr) {
		return {}; // returning nothing since invalid AST
	}
	while (params.size() < root_ptr->nodes) {
		try {

			if (!temp_ptr->read && !free) {
				params.push_back(temp_ptr->root);
				temp_ptr->read = true;
			} else if (params.size() < 1 && !free) {
				params.push_back(temp_ptr->root); // preventing segfault by this
				temp_ptr->read = true;
			} else if (free) { // unread the nodes, so we can get the args of skipped functions
				temp_ptr->read = false;
				params.push_back('\0');
			}
		} catch (...) {
		}
		if (temp_ptr->left_node != nullptr && !temp_ptr->left_node->root.valueless_by_exception()) {
			previous_ptr = temp_ptr;
			temp_ptr = temp_ptr->left_node;
		} else {
			temp_ptr = (temp_ptr->right_node == nullptr) ? previous_ptr->right_node : temp_ptr->right_node;
		}
	}
	parser::remove_space(params, ' ');
	ArgVector::size_type it = 0;
	int i_it = 0;
	if (free)
		return params; // do nothing if we simply free the nodes
	for (auto i : params) {
		if (std::holds_alternative<std::string>(i)) {
			int pos_arg = parser::contains_args(params, "=");
			int pos_arg_equal = parser::contains_args(params, "=", true);
			if (pos_arg != i_it + 1 ||
				pos_arg_equal != -1) { // if for example : i = 3 + 2, i dont want that i gets replaced
				params[it] = parser::replace_variable(i, *vmap);
			}
		}
		it++;
		i_it++;
	}
	ArgVector test_params;
	get_params(test_params, root_ptr, vmap, true); // unreading previous nodes
	return params;
}

bool parser::is_operator(std::string token, bool equal) { // bool equal is if it should check for equal sign too
	const std::vector<std::string> ops = {"/", "+", "-", "*",
										  "="}; // treating char as string because token is string too
	for (std::string op : ops) {
		if ((token == ops.back() && equal) || (token != ops[ops.size() - 1] && op == token)) {
			return true;
		}
	}
	return false;
}

bool parser::contains_str(const std::string &str, const std::string &key) {
	return (str.find(key) == str.npos) ? false : true;
}

bool parser::end_of_code_block(std::string root) { return (root.find("}") == std::string::npos) ? false : true; }

bool parser::compare_values(ArgVector &args) {
	if (args.size() <= 2) {
		return false; // wrong if statement
	}
	args.erase(args.begin());
	args.erase(args.begin() + 1, args.begin() + 3); // removing first 2 if and ==
	auto val = args[0];
	int it = 0;
	for (auto i : args) {
		if (i == val && it > 0)
			return true;
		it++;
	}
	return false;
}

// clang-format off

bool parser::is_if_statement(std::string token) {
	return contains_str(token ,"if") || contains_str(token, "else") || contains_str(token, "elif");
}

bool parser::contains_body(ArgVector &args) {
	bool contains = (contains_args(args, "{") == -1) ? false : true;
	if(contains)
		return true; // again, function body
	for(auto i : args) {
		if(std::holds_alternative<std::string>(i)) {
			auto str = std::get<std::string>(i);
			if(is_if_statement(str))
				return true; // its a function body
		}
	}
	return false;
}

bool parser::is_function_declaration(std::string token) {
	return token == "def"; 
}

bool parser::is_function(std::string token) {
	return contains_str(token, "(") && !contains_str(token, "if");
}

void parser::get_function_name(std::string &func) { 
	func = func.substr(0, func.find("(")); 
}

bool parser::is_var(std::string token) {
	return token == "var";
}

bool parser::contains_while(std::string token) {
	return (token.find("while") == token.npos) ? false : true;
}

// clang-format on
// if i wouldn't do this, clang-format would make these functions a one-liner, which make it even more unreadable

int parser::contains_args(ArgVector &args, AnyVar keyword,
						  bool duplicated) { // duplicated checks for duplicated members
	int it = 0;
	bool duplicated_found = false;
	for (auto i : args) {
		if ((i == keyword && !duplicated) || (i == keyword && duplicated_found && duplicated))
			return it;
		duplicated_found = (duplicated && i == keyword) ? true : false;
		it++;
	}
	return -1;
}

bool parser::tree_is_full(std::shared_ptr<AST> single_t) {
	if (single_t != nullptr && single_t->left_node != nullptr && single_t->right_node != nullptr) {
		return (!single_t->left_node->root.valueless_by_exception() &&
				!single_t->right_node->root.valueless_by_exception() && !single_t->root.valueless_by_exception())
				   ? true
				   : false;
	}
	return false;
}

void parser::set_variable_values(ArgVector &args, std::string f_name, bool is_name) {
	int it = 0;
	get_function_name(f_name);
	if (param_map.find(f_name) == param_map.end())
		param_map[f_name] = VarMap(); // if member doesnt exist, create one
	if (is_name) {
		bool var_begin = false;
		for (auto i : args) {
			std::string i_str = std::get<std::string>(i); // i is guranteed to be of type std::string if is name
			bool is_call = (i_str.find("()") == i_str.npos) ? false : true;
			var_begin = (i_str == "()" || is_call || var_begin) ? true : false;
			if (var_begin && (i_str != "()" || !is_call))
				param_map[f_name][std::get<std::string>(i)] =
					{}; // creating empty member, so later in is_name = false it can be assigned
		}
		return;
	}
	for (auto i : args) {
		int it_loop = 0;
		for (auto &i_ : param_map[f_name]) {
			if (it == it_loop) {
				i_.second = i;
				break;
			}
			it_loop++;
		}
		it++;
	}
}

std::vector<std::shared_ptr<AST>> parser::create_tree() {
	std::vector<std::shared_ptr<AST>> ast_vec{};
	auto single_t = std::make_shared<AST>();
	single_t->allocate_nodes();
	size_t i = 0;
	for (auto token : tokens) {
		single_t = (single_t == nullptr) ? std::make_shared<AST>() : single_t;
		if (std::holds_alternative<std::string>(token) && std::get<std::string>(token) == "\b") {
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

std::string parser::contains_function_vec(ArgVector &args) {
	for (auto i : args) {
		if (std::holds_alternative<std::string>(i)) {
			std::string func_str = std::get<std::string>(i);
			if (is_function(func_str))
				return func_str;
		}
	}
	return "";
}

void parser::save_function(std::shared_ptr<FunctionMap> FMap, std::shared_ptr<AST> s_tree, int i, ArgVector &args) {
	auto func_name = std::get<std::string>(s_tree->right_node->root);
	get_function_name(func_name);
	set_variable_values(args, func_name, true);
	(*FMap)[func_name] = i + 1; // putting function name in Map, so later it can get called by that name, i++ so it
								// knows which line the function starts
}

bool parser::check_statement(ArgVector &args, std::stack<char> &brackets, std::pair<bool, bool> &is_if_is_else) {
	if (args.empty())
		exit(1); // exitting, invalid if statement.
	calc_args(args);
	brackets.push('{');
	if (!compare_values(args) && !is_if_is_else.second) {
		is_if_is_else = {false, true}; // setting else_if true, because if statement is false
		return false;
	} else {
		is_if_is_else = {true, false}; // opposite here
		return true;
	}
}

bool parser::call_if_contains_func(std::shared_ptr<CPPFunctionMap> CPPMap, std::shared_ptr<FunctionMap> PyFMap,
								   ArgVector &args, std::vector<std::shared_ptr<AST>> tree, VarMap &vmap_global,
								   std::stack<char> brackets) {
	std::string f_name = contains_function_vec(args);
	if (f_name.empty())
		return false; // no function name there
	ArgVector args_temp;
	bool save = false; // save all params after function call
	for (auto i : args) {
		if (std::holds_alternative<std::string>(i)) {
			auto pot_func_name = std::get<std::string>(i);
			if (pot_func_name == f_name) {
				save = true;
			}
			continue;
		}
		if (save)
			args_temp.push_back(i);
	}
	set_variable_values(args_temp, f_name);
	brackets.push('{');
	vmap_global[std::get<std::string>(args[1])] = call_function(
		CPPMap, f_name, args_temp, PyFMap, tree, vmap_global); // setting function variable to return value
	return true;
}

void parser::save_iterator_skip(std::vector<std::stack<char>::size_type> &loop_it, std::stack<char> &brackets) {
	brackets.push('{');
	loop_it.push_back(brackets.size());
}

void parser::erase_iterator_skip(std::vector<std::stack<char>::size_type> &loop_it, std::stack<char> &brackets) {
	loop_it.erase(loop_it.begin());
	if (!brackets.empty())
		brackets.pop();
}

AnyVar parser::parse_tree(std::vector<std::shared_ptr<AST>> tree, std::shared_ptr<CPPFunctionMap> CPPMap,
						  std::shared_ptr<FunctionMap> PyFMap, int i, bool single_function, VarMap vmap_global,
						  VarMap vmap_params) {
	ArgVector temp_args, args;
	std::pair<bool, bool> is_if_is_else = {false, false}; // first : is_if, second : is_else
	bool is_str = false, is_elif = false, skip = false;	  // this is for elif statements, not like above for else
														// statements, skip is for skipping for example a function block
	VarMap vmap_all, vmap_block; // vmap_block is for local variables declared in a if/else block or later functions,
								 // see get_vmap() for vmap_all usage
	std::stack<char> brackets;	 // using stack to keep track of nested loops/statements
	std::array<VarMap *, 3> vmap_arr = {&vmap_block, &vmap_global, &vmap_params};
	std::vector<std::stack<char>::size_type> is_while,
		loop_it; // stores the index number of brackets in stack, loop_it is for everything else
	std::vector<std::vector<AnyVar>::size_type> while_iteration; // saves beginning of while loop
	vmap_global = vmap_params;									 // params are globals
	std::map<std::string, std::shared_ptr<AST>> declared_funcs;
	bool skipped_block = false; // checks whether the code block was skipped
	std::string root;
	if (single_function)
		brackets.push('{'); // the function body itself is a body
	for (std::vector<AnyVar>::size_type it = 0; it <= tree.size() - 1; it++) {
		auto s_tree = tree[it];
		vmap_all = get_vmap(vmap_arr);
		args.clear();
		args = s_tree->get_params(args, s_tree, &vmap_all);
		bool contains_brackets = (contains_args(args, "}") != -1) ? true : false;
		if (contains_brackets && brackets.size() > 0 && !skip && !is_while.empty()) {
			if (is_while.size() != 0 && contains_args(args, "}") != -1) {
				if (is_while[0] == brackets.size()) {
					it = while_iteration[0] - 1;
					continue;
				}
			}
			brackets.pop();
			contains_brackets = false;
		}
		if (i > 1 && single_function) {
			i--;
			continue; // jumping to line i, so it goes to function for example
		} else if (skip) {
			if (contains_body(args) || contains_args(args, "while()") != -1 || contains_args(args, "if()") != -1)
				brackets.push('{'); // also keeping track in function
			else if (contains_args(args, "}") != -1)
				brackets.pop(); // either add new body, or remove
			skip = (end_of_code_block(std::get<std::string>(s_tree->root)) &&
					(brackets.size() == 0 || (!is_while.empty() && is_while[0] == brackets.size()) ||
					 (!loop_it.empty() && loop_it[0] == brackets.size())))
					   ? false
					   : true; // once end of function block arrives, dont skip anymore
			if (single_function && brackets.size() == 1)
				skip = false;
			if (!skip) {
				auto brackets_temp = brackets;
				if (!is_while.empty()) {
					erase_iterator_skip(is_while, brackets_temp);
				} else if (!loop_it.empty())
					erase_iterator_skip(loop_it, brackets_temp);
				skipped_block = true;
			}
			continue;
		}

		if (single_function && end_of_code_block(std::get<std::string>(s_tree->root)) && brackets.size() == 1 &&
			args.size() == 1)
			break; // only execute that function and return
		if (contains_brackets)
			brackets.pop();
		if (is_if_is_else.first) {
			if (contains_args(args, "else") != -1) {
				save_iterator_skip(loop_it, brackets);
				skip = true;
			}
			is_if_is_else.first = (brackets.size() == 0 || brackets.size() == loop_it[0]) ? false : true;
		} else if (is_if_is_else.second) {
			is_elif = (contains_args(args, "elif") != -1 || contains_args(args, "elif()") != -1) ? true : false;
			if (!skip && !skipped_block) {
				save_iterator_skip(loop_it, brackets);
				skip = true;
				continue;
			}
			is_if_is_else.second = false;
			skipped_block = false;
			vmap_all = *vmap_arr[1];
			if (is_elif) {
				is_str = true; // using is_str to reduce code, causing root str to say as is
				root = "elif";
				erase_key(args, "}");
			}
		}
		if (!is_if_is_else.first && !is_if_is_else.second) {
			vmap_block.clear();
			vmap_all = *vmap_arr[1];
		}
		is_str = (is_str) ? std::holds_alternative<std::string>(s_tree->root) : is_str;
		root = (is_str) ? root : std::get<std::string>(s_tree->root);
		is_str = false; // setting back to default, so next line root is set properly
		if (contains_while(root)) {
			std::stack<char> brackets_temp;
			std::pair<bool, bool> temp_is_if;
			bool is_true = check_statement(args, brackets_temp, temp_is_if);
			if (!is_true && is_while.size() != 0) { // while loop is false, break!
				is_while.erase(is_while.begin()); // erasing this specific is_while because the while function has ended
				while_iteration.erase(while_iteration.begin());
				brackets.pop();
				save_iterator_skip(is_while, brackets);
				skip = true;
			} else if (is_true) { // while loop is true and is on beginning
				auto it_find = std::find(is_while.begin(), is_while.end(), brackets.size());
				if (it_find == is_while.end()) {
					is_while.push_back(brackets.size());
					while_iteration.push_back(it);
					brackets.push('{');
				}
			} else if (!is_true) {
				save_iterator_skip(is_while, brackets); // skipping this code block, expression is not true
				skip = true;
			}
			continue;
		} else if (is_function(root)) {
			std::string f_name = root;
			args.erase(args.begin()); // emptying function call
			temp_args = args;
			calc_args(args);
			set_variable_values(args, f_name);
			call_function(CPPMap, f_name, args, PyFMap, tree, vmap_global);
			temp_args.clear();
		} else if (is_var(root)) {
			remove_space(args, ' '); // removes the whitespaces between vars definition
			args = calc_args(args);
			bool is_func = call_if_contains_func(CPPMap, PyFMap, args, tree, vmap_global, brackets);
			if ((single_function || (is_if_is_else.first || is_if_is_else.second)) &&
				!is_func) // always put vars in vmap_block if its only a single function
				vmap_block[std::get<std::string>(args[1])] = args[args.size() - 1];
			else if (!is_func) // if is function inside variable, the vmap member already got replaced
				vmap_global[std::get<std::string>(args[1])] = args[args.size() - 1];
			// either making it to the third member (the value of var) or assigning it to the only member of vector
		} else if (is_function_declaration(root)) {
			save_function(PyFMap, s_tree, i, args);
			save_iterator_skip(loop_it, brackets);
			skip = true; // skips next function block, not called yet
		} else if (contains_args(args, "return") != -1) {
			return args[1]; // returning object
		} else if (is_if_statement(root)) {
			check_statement(args, brackets, is_if_is_else);
			loop_it.push_back(brackets.size());
		} else if (contains_args(args, "=") != -1 && contains_args(args, "var") == -1) { // redefining variable
			calc_args(args);
			auto key = std::get<std::string>(args[0]);
			if (vmap_block.count(key))
				vmap_block[key] = args.back();
			else if (vmap_global.count(key))
				vmap_global[key] = args.back();
		}
		i = (single_function)
				? i
				: i + 1; // not incrementing if its single_function because it's already decrementing (line 230)
	}
	return 0;
}

bool parser::is_variant_int(AnyVar i) {
	try {
		std::get<int>(i);
	} catch (std::bad_variant_access const &) {
		return false;
	}
	return true;
}

void parser::init_FMap(std::shared_ptr<CPPFunctionMap> FMap) { // have to hardcode the functions, this is gonna be ugly
	(*FMap)["print"] = (std::function<void(ArgVector & args)>)[](ArgVector & args) {
		for (auto i : args) {
			std::string str{};
			bool newline{};
			try {
				str = std::get<std::string>(i);
				newline = (std::get<std::string>(i).find("\\n") == std::string::npos) ? false : true;
			} catch (const std::bad_variant_access &) {
			}
			std::visit(
				[=](auto &arg) {
					if (str.empty()) {
						std::cout << arg;
						if (is_variant_int(arg)) {
							std::cout << std::endl; // printing newline for numbers, otherwise zsh wont output them
						}
					} else {
						if (newline) {
							for (size_t i_ = 0; i_ < str.length() - 2; i_++) {
								std::cout << str[i_];
							}
							std::cout << std::endl;
						} else {
							std::cout << arg;
						}
					}
				},
				i); // btw, this is what happens if you write code under pressure during New year celebrations LOL
		}
	};
}

void parser::init_parser() {
	auto CPPFMap = std::make_shared<CPPFunctionMap>();
	auto PyFMap = std::make_shared<FunctionMap>();
	init_FMap(CPPFMap); // adding function pointers to Map
	std::vector<std::shared_ptr<AST>> tree = create_tree();
	parse_tree(tree, CPPFMap, PyFMap);
}
