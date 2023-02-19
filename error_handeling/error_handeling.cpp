#include "../parser/parser.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

namespace exception {
void raise(int code, int line) {
	std::ifstream ifs(file_name);
	std::string curr_line;
	int i = 1;
	while (std::getline(ifs, curr_line)) {
		if (i == line)
			break;
		i++;
	}
	std::cout << std::endl << "Error at line : " << line << "!" << std::endl << curr_line << std::endl;
	switch (code) {
	case division_err:
		std::cout << "Division error! " << std::endl;
		break;
	case subtraction_err:
		std::cout << "Subtraction error! " << std::endl;
		break;
	case addition_err:
		std::cout << "Addition error!" << std::endl;
		break;
	case function_not_found:
		std::cout << "Function not found!" << std::endl;
		break;
	case undefined_error:
		std::cout << "Undefined behaviour!" << std::endl;
		break;
	default:
		break;
	}
	std::exit(1);
}
}; // namespace exception
