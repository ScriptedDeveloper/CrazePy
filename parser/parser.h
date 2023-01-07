#pragma once
#include <memory>
#include <unordered_map>
#include <map>
#include <any>
#include <variant>
#include <functional>
#include "../lexer/lexer.h"

using ArgVector = std::vector<std::variant<std::string, int, bool, double, float, char>>;
using VarMap = std::unordered_map<std::string, std::variant<std::string, int, bool, double, float, char>>;


class AST {
	public:	
		std::shared_ptr<AST> add_node(std::variant<std::string, int, bool, double, float, char> token, std::shared_ptr<AST> node);
		std::variant<std::string, int, bool, double, float, char> root;
		size_t nodes;
		bool read = false; // for parsing the tree to check if it has been read or not
		std::shared_ptr<AST> left_node{}; // like a binary tree
		void allocate_nodes(std::shared_ptr<AST> ptr = nullptr);
		ArgVector get_params(ArgVector &params, std::shared_ptr<AST> root);
		std::shared_ptr<AST> right_node{};
		AST();
	private:
		template <typename T>
		auto add_data(ArgVector &params, T data);

};

class parser {
	private:
		using FunctionMap = std::map<std::string, std::any>;
		VarMap vmap;
		ArgVector tokens;
		ArgVector replace_vars(ArgVector &args);
		bool tree_is_full(std::shared_ptr<AST> single_t);
		bool is_var(std::string token);
		void init_FMap(std::shared_ptr<FunctionMap> FMap);
		void print(ArgVector &args);
		static bool is_variant_int(std::variant<std::string, int, bool, double, float> i);
		template <typename P>
		void call_function(std::shared_ptr<FunctionMap> FMap, std::string func_name, P params);
	public:
		template <typename T>
		static void remove_space(ArgVector &args, const T &space);
		static bool is_operator(std::string token);
		static bool is_function(std::string token);
		std::vector<std::shared_ptr<AST>> create_tree(); // pair because i wanna know the amount of nodes
		void parse_tree(std::vector<std::shared_ptr<AST>> tree, std::shared_ptr<FunctionMap> FMap);
		void init_parser();
		static ArgVector calc_args(ArgVector &args); // for expressions like 1+1 or Hello + World
		parser(ArgVector &tokens_);

};

// have to implement some kind of data structure to save function return values or parameters to access them l8ter.. but this is good for now
template <typename  P>
void parser::call_function(std::shared_ptr<FunctionMap> FMap, std::string func_name, P params) {
	std::any& any = (*FMap)[func_name];
	auto function = std::any_cast<std::function<void(P &params)>>(any);
	std::invoke(function, params ); // std::invoke will be important for later
}

template <typename T>
void parser::remove_space(ArgVector &args, const T &space) {
    args.erase(std::remove_if(args.begin(), args.end(), [&](const auto& elm) {
        return (std::holds_alternative<std::string>(elm) && std::all_of(std::get<std::string>(elm).begin(), std::get<std::string>(elm).end(), [&](char ch) { return (space != '\0') ? ch == space : true; }));
    }), args.end());
}

/*
template <typename T>
auto AST::add_data(ArgVector &params, T data) {
	if(std::holds_alternative<std::string>(data)) {
		auto str = std::get<std::string>(data);
		auto i = str.find("\\s");
		if(i != str.npos) {
			str.erase(i, 2);
			params.push_back(str); // adding str to params
		} else {
			if(parser::is_function(str) || parser::is_operator(str)) {
				params.push_back(str);
			}
			 // continuing l8ter, for keywords
		} 
	} else {
		params.push_back(data);
	}
}
*/
