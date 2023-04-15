#include "lexer/lexer.h"
#include "parser/parser.h"
#include <filesystem>
#include <iostream>

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cout << "Need source file!" << std::endl << "Usage : " << argv[0] << " <files>";
		std::exit(1);
	}
	if (!std::filesystem::exists(argv[1])) {
		std::cout << "Error! File '" << argv[1] << "' doesn't exist!" << std::endl;
		std::exit(1);
	}

	lexer l(argv[1]);
	auto test = l.get_tokens();
	parser p(l.tokens, l.name);
	p.init_parser();
	return 0;
}
