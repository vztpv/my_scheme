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
	std::string_view func_name = ""sv;
	bool use_stack = false;
	std::vector<size_t> param_ids;
};

class Wrap2 {
public:
	size_t count = 0;
	bool use_stack = false;
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

		func_info_vec.push_back({ std::string_view(str + idx, idx2 - idx), Wrap2{ 0, false} });

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
		temp.func_name = func_info_vec.back().first;
		temp.use_stack = func_info_vec.back().second.use_stack;

		size_t sz = (func_info_vec.back().second.count);
		for (size_t i = sz; i > 0; --i) {
			temp.param_ids.push_back(param_vec.size() - i);
		}

		func_info_vec.pop_back();

		result.push_back(std::move(temp));
		
		if (!func_info_vec.empty()) {
			Wrap temp;
			temp.func_name = "$push"; // push 'recent result' to stack of VM.

			result.push_back(std::move(temp));
			func_info_vec.back().second.use_stack = true;
		}
		

		//
		idx++;
		while (chk_ws(str[idx])) { idx++; }

		if (idx >= len) {
			goto end;
		}
		goto param_found;
	}
	else if (str[idx] == '(') {
		goto func_name_found;
	}

	{
		func_info_vec.back().second.count++;

		size_t idx2 = idx;
		//
		while (!chk_delim(str[idx2])) { idx2++; }

		std::string x(str + idx, idx2 - idx);
		param_vec.push_back(Data(std::move(x)));

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
		
		auto x = code.first;
		
		Data before_result;

		std::vector<Data> _stack;

		for (auto& _ : x) {
			Data result;

			if (_.func_name == "+"sv) {
				long long value = 0;
				while (!_stack.empty() && _.use_stack) {
					value += std::stoi(_stack.back().x);
					_stack.pop_back();
				}
				for (size_t i = 0; i < _.param_ids.size(); ++i) {
					value += std::stoi(code.second[_.param_ids[i]].x);
				}
				result.x = std::to_string(value);
			}
			else if (_.func_name == "*"sv) {
				long long value = 1;
				while (!_stack.empty() && _.use_stack) {
					value *= std::stoi(_stack.back().x);
					_stack.pop_back();
				}
				for (size_t i = 0; i < _.param_ids.size(); ++i) {
					value *= std::stoi(code.second[_.param_ids[i]].x);
				}
				result.x = std::to_string(value);
			}
			else if (_.func_name == "$push"sv) {
				_stack.push_back(before_result);
			}

			before_result = std::move(result);
		}

		return before_result;
	}
};