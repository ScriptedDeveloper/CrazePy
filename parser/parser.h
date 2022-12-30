#pragma once
#include <memory>
#include <map>
#include <any>
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
		void call_function(std::string name, std::vector<std::string> params, std::string op = "");
		void init_FMap(std::shared_ptr<FunctionMap> FMap);
	public:
		static bool is_operator(std::string token);
		std::vector<std::shared_ptr<AST>> create_tree();
		void parse_tree(std::vector<std::shared_ptr<AST>> tree);
		void init_parser();
		bool is_function(std::string token);
		parser(std::vector<std::string> tokens);

};

