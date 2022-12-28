#pragma once
#include <memory>
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
		std::vector<std::string> tokens;
		//bool tree_is_full(std::shared_ptr<AST> single_t, std::vector<AST> *ast_vec_ptr); // not using smart pointers since not allocating to heap
		bool tree_is_full(std::shared_ptr<AST> single_t, std::vector<std::shared_ptr<AST>> *ast_vec_ptr);
	public:
		static bool is_operator(std::string token);
		std::vector<std::shared_ptr<AST>> parse_tree();
		bool is_function(std::string token);
		parser(std::vector<std::string> tokens);

};

