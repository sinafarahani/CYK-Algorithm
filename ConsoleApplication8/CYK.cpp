#include "CYK.h"

CYK::CYK(std::pair<std::vector<std::vector<std::string>>, char> grammer, std::string input)
	:
	productionRules(grammer.first),
	Start(grammer.second),
	input(input)
{
}

void CYK::start(int grammerNo, int InputNo, std::mutex& mtx, HANDLE& hcon)
{
	//allocate memory
	int size = input.length();
	std::string** table = new std::string* [size];
	for (int i = 0; i < size; i++) {
		table[i] = new std::string[size];
	}

	//Assigns values to principal diagonal of matrix
	int i = 0;
	for (const auto& c : input)
	{
		std::string r;
		std::string st;
		st += c;
		for (const auto& prod_rule : productionRules)
		{
			for (int k = 1; k < prod_rule.size(); k++)
			{
				if (prod_rule[k] == st)
				{
					concat(r, prod_rule[0]);
				}
			}
		}
		table[i][i] = r;
		i++;
	}

	//Assigns values to upper half of the matrix
	for (int k = 1; k < input.length(); k++)
	{
		for (int j = k; j < input.length(); j++)
		{
			std::string r;
			for (int l = j - k; l < j; l++)
			{
				std::string pr = gen_comb(table[j - k][l], table[l + 1][j]);
				concat(r, pr);
			}
			table[j - k][j] = r;
		}
	}

	//Checks if last element of first row contains a Start variable
	mtx.lock();
	if (table[0][input.length() - 1].find(Start) <= table[0][input.length() - 1].length()) {
		SetConsoleTextAttribute(hcon, FOREGROUND_GREEN);
		std::cout << "Input " << InputNo << " is Part of language " << grammerNo << std::endl;
	}
	else {
		SetConsoleTextAttribute(hcon, FOREGROUND_RED);
		std::cout << "Input " << InputNo << " is Not part of language " << grammerNo << std::endl;
	}
	mtx.unlock();

	//release allocated memory
	for (int i = 0; i < size; i++)
		delete[] table[i];
	delete[] table;
}

void CYK::concat(std::string& a, const std::string& b)
{
	for (const auto& c : b) {
		if (a.find(c) == std::string::npos) {
			a += c;
		}
	}
}

std::string CYK::gen_comb(const std::string& a, const std::string& b)
{
	std::string r;
	for (const auto& i : a)
		for (const auto& j : b)
		{
			std::string pri;
			pri = pri + i + j;

			//searches if the generated productions p can be created by variables
			for (const auto& prod_rule : productionRules)
			{
				for (int k = 1; k < prod_rule.size(); k++)
				{
					if (prod_rule[k] == pri)
					{
						concat(r, prod_rule[0]);
					}
				}
			}
		}
	return r;
}
