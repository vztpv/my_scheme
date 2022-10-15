#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;
// (func_name argument ...)

class Data;

class FuncInfo {
public:
	int func_no;
	std::vector<Data> param;
};


class Data {
public:
	std::string x;

	explicit Data(std::string x="") : x(x) {
		// 
	}

};

// func_name -> func_no?

inline bool chk_ws(char x) { // no include '\0'
	return x == ' ' || x == '\t' || x == '\r' || x == '\n';
}

inline bool chk_delim(char x) {
	return chk_ws(x) || x == '(' || x == ')';
}

class Wrap {
public:
	std::string_view op = ""sv;
	std::string_view func_name = ""sv;
	size_t param_count = 0;
	size_t param_idx = 0;
};

class Wrap2 {
public:
	size_t count = 0;
};

std::pair<std::vector<Wrap>, std::vector<Data>> Generate(const char* str, size_t len) {
	std::vector<Wrap> result;

	std::vector<std::pair<std::string_view, Wrap2>> func_info_vec;
	std::vector<Data> param_vec;

	size_t idx = 0;

	// find ( 
	while (chk_ws(str[idx])) { idx++; }
	if (str[idx] == '(') {
	func_name_found:

		++idx;
		while (chk_ws(str[idx])) { idx++; }

		size_t idx2 = idx;

		while (!chk_delim(str[idx2])) { idx2++; }

		func_info_vec.push_back({ std::string_view(str + idx, idx2 - idx), Wrap2{ 0 } });

		idx = idx2;
	}
	else {
		goto error; // error.
	}
param_found:
	while (chk_ws(str[idx])) { idx++; }
	
	if (str[idx] == ')') {
		// (+ 3 4)
		//    0 1   param_id?
		
		// (+ (* 3 5 ) 4)

		Wrap temp;
		temp.op = "func";
		temp.func_name = func_info_vec.back().first;

		temp.param_count = func_info_vec.back().second.count;

		func_info_vec.pop_back();

		result.push_back(std::move(temp));
		
		//
		idx++;
		while (chk_ws(str[idx])) { idx++; }

		if (idx >= len) {
			goto end;
		}
		goto param_found;
	}
	else if (str[idx] == '(') {
		func_info_vec.back().second.count++;

		goto func_name_found;
	}

	{
		func_info_vec.back().second.count++;

		size_t idx2 = idx;
		//
		while (!chk_delim(str[idx2])) { idx2++; }

		std::string x(str + idx, idx2 - idx);
		param_vec.push_back(Data(std::move(x)));

		Wrap temp;
		temp.op = "push"sv;
		temp.param_idx = param_vec.size() - 1;

		result.push_back(std::move(temp));
		
		idx = idx2;
		goto param_found;
	}
error:
	{
		throw "ERROR in my scheme.";
	}

end:
	return { result, param_vec };
}

class VM {
public:
	Data run(std::pair<std::vector<Wrap>, std::vector<Data>> code) {
		
		auto& x = code.first;

		std::vector<Data> _stack;

		for (auto& _ : x) {
			Data result;

			if (_.op == "func"sv) {
				if (_.func_name == "+"sv) {
					long long value = 0;
					for (size_t i = 0; i < _.param_count; ++i) {
						value += std::stoi(_stack.back().x);
						_stack.pop_back();
					}
					result.x = std::to_string(value);
					_stack.push_back(std::move(result));
				}
				else if (_.func_name == "*"sv) {
					long long value = 1;
					for (size_t i = 0; i < _.param_count; ++i) {
						value *= std::stoi(_stack.back().x);
						_stack.pop_back();
					}
					result.x = std::to_string(value);
					_stack.push_back(std::move(result));
				}
			}
			else if (_.op == "push"sv) {
				_stack.push_back(code.second[_.param_idx]);
			}
		}

		return _stack.back();
	}
};