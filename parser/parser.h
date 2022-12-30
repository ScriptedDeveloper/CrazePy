#pragma once
#include <memory>
#include <map>
#include <any>
#include <variant>
#include <functional>
#include "../lexer/lexer.h"

class AST {
	public:	
		void add_node(std::string token);
		std::string root{};
		std::shared_ptr<AST> left_node{}; // like a binary tree
		void allocate_nodes();
		std::shared_ptr<AST> right_node{};
		AST();

};


class parser {
	private:
		
		using FunctionMap = std::map<std::string, std::any>;
		std::vector<std::string> tokens;
		bool tree_is_full(std::shared_ptr<AST> single_t, std::vector<std::shared_ptr<AST>> *ast_vec_ptr);
		void init_FMap(std::shared_ptr<FunctionMap> FMap);
		template <typename ... P>
		void call_function(std::shared_ptr<FunctionMap> FMap, std::string func_name, P ... params);
	public:
		static bool is_operator(std::string token);
		std::vector<std::shared_ptr<AST>> create_tree();
		void parse_tree(std::vector<std::shared_ptr<AST>> tree);
		void init_parser();
		bool is_function(std::string token);
		parser(std::vector<std::string> tokens);

};

// have to implement some kind of data structure to save function return values or parameters to access them l8ter.. but this is good for now
template <typename ... P>
void parser::call_function(std::shared_ptr<FunctionMap> FMap, std::string func_name, P ... params) {
	std::any& any = (*FMap)[func_name];
	auto function = std::any_cast<std::function<void(const std::string&)>>(any);
	std::invoke(function, params ...); // std::invoke will be important for later
}

