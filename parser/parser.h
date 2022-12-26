#pragma once
#include "../lexer/lexer.h"

class parser : public lexer {
	private:
		bool is_operator(std::string token);
	public:
		void parse_tree();
		bool is_function(std::string token);
};

class AST  {
	public:	
		std::string root;
		std::string left_node; // like a binary tree
		std::string right_node;

		AST();
};
