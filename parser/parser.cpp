#include "parser.h"
#include <cmath>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <variant>

std::string file_name;

parser::parser(ArgVector &tokens_, std::string &fname) : param_map(), tokens(tokens_) { file_name = fname; }

AST::AST() : root(), nodes() {}

bool parser::has_one_value(ArgVector args) {
	if (args.empty())
		return false;
	auto is_int = [&](AnyVar x) { return std::holds_alternative<int>(x); };
	args.erase(std::remove_if(args.begin(), args.end(), is_int));
	return (args.size() == 1) ? true : false;
}

auto parser::replace_variable(AnyVar &var, const VarMap &vmap) {
	for (auto x : vmap) {
		if (x.first == std::get<std::string>(var)) {
			auto var_replace = x.second; // replacing var with value
			if (std::holds_alternative<std::string>(var_replace))
				var_replace = std::get<std::string>(x.second) + "\\s"; // marking it as string
			var = std::move(var_replace);
			break;
		}
	}
	return var;
}

ArgVector parser::calc_args(ArgVector &args, int line) {
	ArgVector temp_args;
	while (true) {
		temp_args = args;
		size_t i = 0;
		bool no_equation = true; // no_equation checks whether it has one or not
		long i_ = 0;
		if (has_one_value(args))
			return args; // only has one value, no need to do anything
		auto is_op = [](AnyVar x) {
			return (std::holds_alternative<std::string>(x) && is_operator(std::get<std::string>(x))) ? true : false;
		};
		for (auto it = std::find_if(args.begin(), args.end(), is_op); it != args.end();
			 it = std::find_if(it++, args.end(), is_op)) {
			if (args.size() <= 1)
				break;
			if (std::holds_alternative<std::string>(*it) && is_operator(std::get<std::string>(*it))) {
				size_t dist = 0;
				for (auto it1 = args.begin(); it1 != it; it1++)
					dist++; // not using std::distance because it returns long, which has different signess to size_t

				bool failed = false;
				no_equation = false;
				std::string op = std::get<std::string>(*it);
				int x1 = 0, x2 = 0;
				try {
					x1 = std::get<int>((args[dist + 1])), x2 = std::get<int>(args[dist - 1]);
				} catch (const std::bad_variant_access &) {
					try {
						x1 = static_cast<int>(std::round(std::get<double>((args[dist + 1])))),
						x2 = static_cast<int>(std::round(std::get<double>(args[dist - 1]))); // trying double
						// I know you shouldnt round the double up, this is a quick solution for now
					} catch (const std::bad_variant_access &) {
						failed = true;
					}
					// no arithmatic operation involved
				}
				if (op == "+") {
					if (failed)
						exception::raise(exception::addition_err, line);
					args[dist--] = x1 + x2;
				} else if (op == "-") {
					if (failed)
						exception::raise(exception::subtraction_err, line);
					args[dist--] = x1 - x2;
				} else if (op == "/") {
					if (failed)
						exception::raise(exception::division_err, line);
					args[dist--] = x1 / x2;
				} else if (op == "*") { // ugly but it works for now
					if (failed)
						exception::raise(exception::multiplication_err, line);
					args[dist--] = x1 * x2;
				} else {
					return args; // no equation left
				}
				for (size_t it1 = 0; it1 <= 1; it1++)
					args.erase(args.begin() + static_cast<long>(dist) + static_cast<long>(it1));
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
			if ((pos_arg != i_it + 1 ||
				 pos_arg_equal != -1)) { // if for example : i = 3 + 2, i dont want that i gets replaced
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

bool parser::is_operator(std::string token,
						 bool equal) {							// bool equal is if it should check for equal sign too
	std::array<std::string, 5> ops = {"/", "+", "-", "*", "="}; // treating char as string because token is string too
	auto it = std::find(ops.begin(), ops.end(), token);
	if (it != ops.end()) {
		if ((*it == ops.back() && equal) || (token != ops.back() && *it == token)) {
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
	auto is_appropriate =
		[](AnyVar i) { // this lambda checks if the value is appropriate (double, int, string...) something to check
			if (std::holds_alternative<int>(i) || std::holds_alternative<double>(i) || std::holds_alternative<float>(i))
				return false;
			else if (std::holds_alternative<std::string>(i)) {
				auto str = std::get<std::string>(i);
				if (str.find("\\s") != str.npos)
					return false;
			}
			return true;
		};
	while (args.size() > 2)
		args.erase(std::remove_if(args.begin(), args.end(), is_appropriate));
	ArgVector temp_args = args;
	auto has_string = [](AnyVar &i) {
		if (std::holds_alternative<std::string>(i)) {
			std::string str = std::get<std::string>(i);
			auto it = str.find("\\s");
			if (it != str.npos) {
				str.erase(it, str.length());
				return true;
			}
		}
		return false;
	};
	if (std::find_if(temp_args.begin(), temp_args.end(), has_string) != temp_args.end())
		return (temp_args[0] == temp_args[1]) ? true : false;
	return (args[0] == args[1]) ? true : false;
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

bool parser::is_not_equal_statement(ArgVector &args) { return (contains_args(args, "!=") != -1) ? true : false; }

bool parser::check_statement(ArgVector &args, std::stack<char> &brackets, std::pair<bool, bool> &is_if_is_else,
							 int line) {
	if (args.empty())
		exit(1); // exitting, invalid if statement.
	calc_args(args, line);
	brackets.push('{');
	bool is_not_equal = is_not_equal_statement(args);
	if (!compare_values(args) && !is_if_is_else.second) {
		if (is_not_equal)
			is_if_is_else = {true, false}; // swapped because we are checking for the opposite
		else
			is_if_is_else = {false, true}; // setting else_if true, because if statement is false
		return is_not_equal;
	} else {
		if (is_not_equal) {
			is_if_is_else = {false, true};
			return false;
		} else
			is_if_is_else = {true, false};
		return true;
	}
}

bool parser::call_if_contains_func(std::shared_ptr<CPPFunctionMap> CPPMap, std::shared_ptr<FunctionMap> PyFMap,
								   ArgVector &args, std::vector<std::shared_ptr<AST>> tree, VarMap &vmap_global,
								   std::stack<char> brackets, int line) {
	std::string f_name = contains_function_vec(args);
	if (f_name.empty())
		return false; // no function name there
	ArgVector args_temp;
	auto is_func = [f_name](AnyVar i) {
		return (std::holds_alternative<std::string>(i) && std::get<std::string>(i) == f_name) ? true : false;
	};
	auto result = std::find_if(args.begin(), args.end(), is_func);
	if (result != args.end()) {
		for (auto it = result; it != args.end(); it++)
			args_temp.push_back(*it);
	}
	set_variable_values(args_temp, f_name);
	brackets.push('{');
	// clang-format off
	vmap_global[std::get<std::string>(args[1])] = call_function(CPPMap, f_name, args_temp, PyFMap, tree, vmap_global,
		   line); // setting function variable to return value
	// clang-format on
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
	bool removed_bracket = false, removed_bracket_loop = false;
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
				brackets.pop();
				if (is_while[0] == brackets.size()) {
					it = while_iteration[0] - 1;
					removed_bracket_loop = true;
					is_while.erase(is_while.begin());
					continue;
				}
			}
			contains_brackets = false;
		}
		if (i > 1 && single_function) {
			i--;
			continue; // jumping to line i, so it goes to function for example
		} else if (skip) {
			if (contains_body(args) || contains_args(args, "while()") != -1 || contains_args(args, "if()") != -1)
				brackets.push('{'); // also keeping track in function
			if (contains_args(args, "}") != -1) {
				brackets.pop(); // either add new body, or remove
				removed_bracket = true;
			}
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
				if (brackets.size() == 1)
					continue; // if not continuing, next if statement would catch it and exit function
			} else
				continue;
		}

		if (single_function && end_of_code_block(std::get<std::string>(s_tree->root)) && brackets.size() == 1 &&
			args.size() == 1)
			break; // only execute that function and return
		if (contains_brackets && !removed_bracket)
			brackets.pop();
		removed_bracket = false;
		if (is_if_is_else.first) {
			std::stack<char> brackets_temp;
			if (contains_args(args, "else") != -1) {
				save_iterator_skip(loop_it, brackets_temp);
				skip = true;
			}
			is_if_is_else.first = (brackets.size() == 0 || brackets.size() == loop_it[0]) ? false : true;
		} else if (is_if_is_else.second) {
			std::stack<char> brackets_temp;
			is_elif = (contains_args(args, "elif") != -1 || contains_args(args, "elif()") != -1) ? true : false;
			if (!skip && !skipped_block) {
				save_iterator_skip(loop_it, brackets_temp);
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
			bool is_true = check_statement(args, brackets_temp, temp_is_if, i);
			if (!is_true && is_while.size() != 0) { // while loop is false, break!
				is_while.erase(is_while.begin()); // erasing this specific is_while because the while function has ended
				while_iteration.erase(while_iteration.begin());
				if (!removed_bracket_loop)
					brackets.pop();
				removed_bracket_loop = false;
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
			calc_args(args, i);
			set_variable_values(args, f_name);
			call_function(CPPMap, f_name, args, PyFMap, tree, vmap_global, i);
			temp_args.clear();
		} else if (is_var(root)) {
			remove_space(args, ' '); // removes the whitespaces between vars definition
			args = calc_args(args, i);
			bool contains_func = call_if_contains_func(CPPMap, PyFMap, args, tree, vmap_global, brackets, i);
			if (!contains_func)
				vmap_global[std::get<std::string>(args[1])] = args[args.size() - 1];
			// either making it to the third member (the value of var) or assigning it to the only member of vector
		} else if (is_function_declaration(root)) {
			std::vector<std::stack<char>::size_type> loop_it_temp;
			save_function(PyFMap, s_tree, i, args);
			save_iterator_skip(loop_it_temp, brackets);
			skip = true; // skips next function block, not called yet
		} else if (contains_args(args, "return") != -1) {
			return args[1]; // returning object
		} else if (is_if_statement(root)) {
			check_statement(args, brackets, is_if_is_else, i);
			loop_it.push_back(brackets.size());
		} else if (contains_args(args, "=") != -1 && contains_args(args, "var") == -1) { // redefining variable
			calc_args(args, i);
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
	(*FMap)["print"] =
		(std::function<void(ArgVector & args, AnyVar & return_val)>)[](ArgVector & args, AnyVar & return_val) {
		for (auto i : args) {
			std::string str{};
			bool newline{};
			try {
				str = std::get<std::string>(i);
				newline = (std::get<std::string>(i).find("\\n") == std::string::npos) ? false : true;
			} catch (const std::bad_variant_access &) {
			}
			std::visit(
				[&](auto &arg) {
					if (str.empty()) {
						std::cout << arg << std::endl;
						if (is_variant_int(arg)) {
							std::cout << std::endl; // printing newline for numbers, otherwise zsh wont output them
						}
					} else {
						if (newline) {
							std::string str_convert;
							AnyVar converted = arg;
							if (std::holds_alternative<std::string>(converted))
								str_convert = std::get<std::string>(converted);
							else {
								return_val = 1;
								return;
							}
							for (auto it = str_convert.find("\\n"); it != str_convert.npos;
								 it = str_convert.find("\\n"))
								str_convert.replace(it, 2, "\n");
							std::cout << str_convert << std::endl;
						} else {
							std::cout << arg << std::endl; // double newline
						}
					}
				},
				i); // btw, this is what happens if you write code under pressure during New year celebrations LOL
		}
	};
	(*FMap)["input"] =
		(std::function<void(ArgVector & args, AnyVar & return_val)>)[](ArgVector & args, AnyVar & return_val) {
		std::string input;
		std::cin >> input;
		auto val1 = is_type<int>(input);
		auto val2 = is_type<bool>(input);
		auto val3 = is_type<double>(input);
		auto val4 = is_type<float>(input); // sorry, im trying to do this quickly
		if (val1.first)
			return_val = val1.second;
		else if (val2.first)
			return_val = val2.second;
		else if (val3.first)
			return_val = val3.second;
		else if (val4.first)
			return_val = val4.second;
		else
			return_val = input;
		args.clear(); // very unnecessary, getting rid of compiler unused warning
	};
}

void parser::init_parser() {
	auto CPPFMap = std::make_shared<CPPFunctionMap>();
	auto PyFMap = std::make_shared<FunctionMap>();
	init_FMap(CPPFMap); // adding function pointers to Map
	std::vector<std::shared_ptr<AST>> tree = create_tree();
	parse_tree(tree, CPPFMap, PyFMap);
}
