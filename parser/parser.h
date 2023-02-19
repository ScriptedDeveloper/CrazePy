#pragma once
#include "../lexer/lexer.h"
#include <any>
#include <functional>
#include <map>
#include <stack>
#include <memory>
#include <unordered_map>
#include <variant>
#include <sstream>
#include "../type_names.h"
#include "../error_handeling/error_handeling.h"
class AST {
  public:
	std::shared_ptr<AST> add_node(AnyVar token, std::shared_ptr<AST> node);
	AnyVar root;
	size_t nodes;
	bool read = false;				  // for parsing the tree to check if it has been read or not
	std::shared_ptr<AST> left_node{}; // like a binary tree
	void allocate_nodes(std::shared_ptr<AST> ptr = nullptr);
	ArgVector get_params(ArgVector &params, std::shared_ptr<AST> root, VarMap *vmap, bool free = false);
	std::shared_ptr<AST> right_node{};
	AST();

  private:
	template <typename T> auto add_data(ArgVector &params, T data);
};

extern std::string file_name;

class parser {
  private:
	using CPPFunctionMap = std::map<std::string, std::any>; // FMap is for pre-defined C++ functions, FunctionMap is for
															// CrazePy defined functions
	using FunctionMap = std::map<std::string, int>;			// int is for line number
	std::map<std::string, VarMap> param_map;
	ArgVector tokens;
	ArgVector replace_vars(ArgVector &args);
	void save_function(std::shared_ptr<FunctionMap> FMap, std::shared_ptr<AST> tree, int i,
					   ArgVector &args); // i is for iteration till it finds the correct line
	bool tree_is_full(std::shared_ptr<AST> single_t);
	bool end_of_code_block(std::string root);
	bool is_not_equal_statement(ArgVector &args);
	template <typename T>
	static auto is_type(const std::string &str);
	bool contains_body(ArgVector &args);
	bool check_statement(ArgVector &args, std::stack<char> &brackets, std::pair<bool ,bool> &is_if_is_else, int line);
	bool contains_while(std::string token);
	std::string contains_function_vec(ArgVector &args);
	bool call_if_contains_func(std::shared_ptr<CPPFunctionMap> CPPMap, std::shared_ptr<FunctionMap> PyFMap,
							   ArgVector &args, std::vector<std::shared_ptr<AST>> tree, VarMap &vmap_global, std::stack<char> brackets, int line);
	void set_variable_values(ArgVector &args, std::string f_name, bool is_name = false);
	void get_function_name(std::string &func);
	void save_iterator_skip(std::vector<std::stack<char>::size_type> &loop_it, std::stack<char> &brackets);
	void erase_iterator_skip(std::vector<std::stack<char>::size_type> &loop_it, std::stack<char> &brackets);
	template <typename T> void erase_key(T &args, std::string key);
	template <typename T> VarMap get_vmap(T arr);
	static bool has_one_value(ArgVector args);
	bool compare_values(ArgVector &args);
	static bool contains_str(const std::string &str, const std::string &key);
	bool is_function_declaration(std::string token);
	bool is_var(std::string token);
	bool is_if_statement(std::string token);
	void init_FMap(std::shared_ptr<CPPFunctionMap> FMap);
	void print(ArgVector &args);
	template <typename P>
	AnyVar call_function(std::shared_ptr<CPPFunctionMap> FMap, std::string func_name, P params,
						 std::shared_ptr<FunctionMap> PyMap, std::vector<std::shared_ptr<AST>> tree,
						 VarMap &vmap_global, int line);

  public:
	template <typename T> static auto replace_variable(T &var, const VarMap &vmap);
	template <typename T> static void remove_space(ArgVector &args, const T &space);
	template <typename T>
	static int contains_args(T &args, AnyVar keyword, bool duplicated = false);
	static bool is_operator(std::string token, bool equal = false);
	static bool is_function(std::string token);
	static bool is_variant_int(AnyVar i);
	std::vector<std::shared_ptr<AST>> create_tree(); // pair because i wanna know the amount of nodes
	AnyVar parse_tree(std::vector<std::shared_ptr<AST>> tree, std::shared_ptr<CPPFunctionMap> FMap,
					  std::shared_ptr<FunctionMap> PyMap, int i = 1, bool single_function = false,
					  VarMap vmap_global = VarMap(),
					  VarMap vmap_params = VarMap()); // line counting starts from 1, not 0
	void init_parser();
	static ArgVector calc_args(ArgVector &args, int line); // for expressions like 1+1 or Hello + World
	parser(ArgVector &tokens_, std::string &fname);
};

void input(ArgVector &args, AnyVar &return_val);
// have to implement some kind of data structure to save function return values or parameters to access them l8ter.. but
// this is good for now
template <typename P>
AnyVar parser::call_function(std::shared_ptr<CPPFunctionMap> CPPFMap, std::string func_name, P params,
							 std::shared_ptr<FunctionMap> PyMap, std::vector<std::shared_ptr<AST>> tree,
							 VarMap &vmap_global, int line) {
	auto CPPmap = *CPPFMap;
	auto FMap = *PyMap;
	AnyVar return_val = 0; // havent found a better way of doing this
	get_function_name(func_name);
	if (CPPmap.find(func_name) != CPPmap.end()) {
		auto &any = CPPmap[func_name];
		try {
			auto function = std::any_cast<std::function<void(P & params, AnyVar &return_val)>>(any);
			std::invoke(function, params, return_val); // std::invoke will be important for later
		} catch(std::bad_any_cast&) {
			auto function = std::any_cast<std::function<void(P & params)>>(any); // retrying without return val
			std::invoke(function, params); 
		}
	} else if (FMap.find(func_name) != FMap.end()) {
		if (PyMap->find(func_name) == PyMap->end())
			return 1; // function name not found, proper error handeling later
		return parse_tree(tree, CPPFMap, PyMap, (*PyMap)[func_name], true, vmap_global, param_map[func_name]);
	} else 
		exception::raise(exception::function_not_found, line);
	return return_val;
}

template <typename T> void parser::remove_space(ArgVector &args, const T &space) {
	args.erase(std::remove_if(args.begin(), args.end(),
							  [&](const auto &elm) {
								  return (std::holds_alternative<std::string>(elm) &&
										  std::all_of(std::get<std::string>(elm).begin(),
													  std::get<std::string>(elm).end(),
													  [&](char ch) { return (space != '\0') ? ch == space : true; }));
							  }),
			   args.end());
}

template <typename T> auto parser::replace_variable(T &var, const VarMap &vmap) {
	for (auto x : vmap) {
		if (x.first == std::get<std::string>(var)) {
			var = x.second; // replacing var with value
			break;
		}
	}
	return var;
}

template <typename T> VarMap parser::get_vmap(T arr) {
	VarMap vmap_all; // putting local as well as global variables into a single map
	for (auto i : arr)
		vmap_all.insert(i->begin(), i->end());
	return vmap_all;
}

template <typename T> void parser::erase_key(T &args, std::string key) {
	int it = 0;
	for (auto i : args) {
		if (std::holds_alternative<std::string>(i) && std::get<std::string>(i) == key) {
			args.erase(args.begin() + it);
			return;
		}
		it++;
	}
}

template <typename T>
auto parser::is_type(const std::string &str) {
	auto result = T();
	auto istream = std::istringstream(str);
	istream >> result;
	std::pair<bool, double> pair1 = {true, result}, pair2 = {false, 0};
	return (istream.eof() && !istream.fail()) ? pair1 : pair2;
}
template <typename T>
int parser::contains_args(T &args, AnyVar keyword, // T being here either std::array<AnyVar> or ArgVector
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

