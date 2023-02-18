#pragma once

namespace exception {
	constexpr int division_err = 1;
	constexpr int subtraction_err = 2;
	constexpr int addition_err = 3;
	constexpr int multiplication_err = 5;
	constexpr int function_not_found = 6;
	constexpr int undefined_error = 7; // for undefined keywords
	void raise(int code, int line);
}
