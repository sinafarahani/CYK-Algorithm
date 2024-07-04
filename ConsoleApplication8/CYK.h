#pragma once

#include <iostream>
#include <mutex>
#include <string>
#include <vector>
#include <Windows.h>

class CYK {
public:
	CYK(std::pair<std::vector<std::vector<std::string>>, char> grammer, std::string input);
	void start(int grammerNo, int InputNo, std::mutex& mtx, HANDLE& hcon);
private:
	void concat(std::string& a, const std::string& b);
	std::string gen_comb(const std::string& a, const std::string& b);

	char Start;
	std::vector<std::vector<std::string>> productionRules;
	std::string input;
};