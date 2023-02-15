#pragma once
#include <iostream>
#include <variant>
#include <vector>
#include "../type_names.h"
using ArgVector = std::vector<AnyVar>;

class lexer {
  private:
	void add_token(ArgVector &curr_tokens, std::string &token);

  public:
	std::string name;
	ArgVector tokens;
	lexer(std::string filename);
	ArgVector get_tokens();
};
