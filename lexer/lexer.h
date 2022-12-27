#pragma once
#include <iostream>
#include <vector>

class lexer {
	private:
		std::string name;
	public:
		std::vector<std::string> tokens;
		lexer(std::string filename);
		std::vector<std::string> get_tokens();
};
