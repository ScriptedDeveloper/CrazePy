#pragma once
#include "../lexer/lexer.h"

class AST : lexer {
	private:
		std::string root;
		std::string left_node; // like a binary tree
		std::string right_node;
	
	public:
		void parse_tree();
}
