
#include <iostream>
#include <vector>
#include <string>

#include "my_scheme.h"

int main(void)
{
	VM vm;
	std::string_view code = "(+ (* 4 5 ) (* 3 4 5))  "sv;

	try {
		std::cout << vm.run(Generate(code.data(), code.size())).x << "\n";
	}
	catch (const char* str) {
		std::cout << "ERROR " << str << "\n";
	}
	//catch (...) {
	//	std::cout << "internal err\n";
	//}
	return 0;
}

