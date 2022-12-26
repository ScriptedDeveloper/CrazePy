#include <iostream>
#include "lexer/lexer.h"

int main(int argc, char **argv) {
	if(argc < 2) {
		std::cout << "Need source file!" << std::endl << "Usage : " << argv[0] << " <files>";
		exit(0);
	}
	lexer l(argv[1]);
	std::vector<std::string> test = l.get_tokens();
	return 0;
}
