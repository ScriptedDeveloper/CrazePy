#pragma once
#include <memory>
#include <map>
#include <any>
#include <variant>
#include <functional>
#include "../lexer/lexer.h"

using ArgVector = std::vector<std::variant<std::string, int, bool, double, float>>;


class AST {
	public:	
		std::shared_ptr<AST> add_node(std::string token, std::shared_ptr<AST> node);
		std::string root{};
		size_t nodes;
		bool read = false; // for parsing the tree to check if it has been read or not
		std::shared_ptr<AST> left_node{}; // like a binary tree
		void allocate_nodes(std::shared_ptr<AST> ptr = nullptr);
		ArgVector get_params(ArgVector params, std::shared_ptr<AST> root);
		std::shared_ptr<AST> right_node{};
		AST();

};

class parser {
	private:
		
		using FunctionMap = std::map<std::string, std::any>;
		std::vector<std::string> tokens;
		ArgVector sort_tree(ArgVector args);
		bool tree_is_full(std::shared_ptr<AST> single_t);
		void init_FMap(std::shared_ptr<FunctionMap> FMap);
		void print(ArgVector args);
		static bool is_variant_int(std::variant<std::string, int, bool, double, float> i);
		template <typename P>
		void call_function(std::shared_ptr<FunctionMap> FMap, std::string func_name, P params);
	public:
		template <typename T>
		static bool find_variant(ArgVector &args, T key);
		static bool is_operator(std::string token);
		std::vector<std::shared_ptr<AST>> create_tree(); // pair because i wanna know the amount of nodes
		void parse_tree(std::vector<std::shared_ptr<AST>> tree, std::shared_ptr<FunctionMap> FMap);
		void init_parser();
		bool is_function(std::string token);
		static ArgVector calc_args(ArgVector &args); // for expressions like 1+1 or Hello + World
		parser(std::vector<std::string> tokens);

};

// have to implement some kind of data structure to save function return values or parameters to access them l8ter.. but this is good for now
template <typename  P>
void parser::call_function(std::shared_ptr<FunctionMap> FMap, std::string func_name, P params) {
	std::any& any = (*FMap)[func_name];
	auto function = std::any_cast<std::function<void(P &params)>>(any);
	std::invoke(function, params ); // std::invoke will be important for later
}

template <typename T>
bool parser::find_variant(ArgVector &args, T key) {
	return (std::find_if(args.begin(), args.end(), [&](const auto &arg) {
		if(std::holds_alternative<T>(arg)) {
			return std::get<T>(arg) == key;
		}
		return false;
	}) == args.end()) ? true : false;
}
