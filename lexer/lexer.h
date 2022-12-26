#pragma once
#include <iostream>
#include <vector>

class lexer {
	private:
		std::string name;
	protected:
		std::vector<std::string> tokens;
	public:
		lexer(std::string filename) {
			name = filename;
		}
		std::vector<std::string> get_tokens();
};
