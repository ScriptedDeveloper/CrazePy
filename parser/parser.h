#pragma once
#include "../lexer/lexer.h"

class AST {
	public:	
		void add_node(std::string token);
		std::string root;
		AST *left_node; // like a binary tree
		AST *right_node;
		AST() {};
};


class parser {
	private:
		std::vector<std::string> tokens;
		bool tree_is_full(AST *single_t, std::vector<AST> *ast_vec_ptr); // not using smart pointers since not allocating to heap
	public:
		static bool is_operator(std::string token);
		std::vector<AST> parse_tree();
		bool is_function(std::string token);
		parser(std::vector<std::string> tokens);

};

