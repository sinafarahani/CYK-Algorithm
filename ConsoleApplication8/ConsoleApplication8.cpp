#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <Windows.h>

#include "CYK.h"

#define GRAMMERS_FILE "grammers"
#define INPUTS_FILE "inputs"

std::mutex mtx;

std::vector<std::pair<std::vector<std::vector<std::string>>, char>> grammers;
std::vector<std::thread> CYKs;

HANDLE hcon = GetStdHandle(STD_OUTPUT_HANDLE);

CONSOLE_SCREEN_BUFFER_INFO buf;

void handleGrammer() {
	std::ifstream gfile(GRAMMERS_FILE);
	bool productionLine = false;
	bool startFound = false;
	std::vector<std::vector<std::string>> productionRules;

	//handle grammers
	for (std::string line; std::getline(gfile, line);) {
		if (line == "PRODUCTIONS:") {
			productionLine = true;
			continue;
		}
		if (productionLine) {
			if (line == "[END]") {
				productionLine = false;
				grammers.push_back(std::make_pair(productionRules, 'S'));
				productionRules.clear();
			}
			else if (line == "START:") {
				startFound = true;
			}
			else if (!startFound) {
				line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
				line.erase(remove(line.begin(), line.end(), ';'), line.end());

				std::vector<std::string> prod_line;

				if (auto pt = line.find("->") != std::string::npos) {
					prod_line.push_back(line.substr(0, pt));
					line = line.substr(pt + 2, line.length());
					while (!line.empty())
					{
						auto i = line.find("|");
						if (i > line.length())
						{
							prod_line.push_back(line);
							line.clear();
						}
						else
						{
							prod_line.push_back(line.substr(0, i));
							line = line.substr(i + 1, line.length());
						}
					}
					productionRules.push_back(prod_line);
				}
			}
			else {
				productionLine = false;
				line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
				line.erase(remove(line.begin(), line.end(), ';'), line.end());
				grammers.push_back(std::make_pair(productionRules, line[0]));
				productionRules.clear();
				startFound = false;
			}
		}
	}
}

int main()
{
	GetConsoleScreenBufferInfo(hcon, &buf);
	const auto dc = buf.wAttributes;

	handleGrammer();

	if (grammers.empty()) {
		std::cout << "No grammer detected!";
		exit(0);
	}

	//handle inputs
	std::ifstream ifile(INPUTS_FILE);
	bool readInputsFromFile = false;
	if (ifile && ifile.peek() != EOF) {
		std::cout << "Input file is exist. Do you want to read inputs from file?(Y/N)";
		char ans;
		std::cin >> ans;
		if (ans == 'Y' || ans == 'y') {
			readInputsFromFile = true;
		}
	}

	int j = 0;

	if (readInputsFromFile) {
		for (std::string input; std::getline(ifile, input);) {
			j++;
			int i = 0;
			for (const auto& grammer : grammers) {
				i++;
				CYK c(grammer, input);
				auto w = [=]() mutable {c.start(i, j, std::ref(mtx), hcon); };
				CYKs.push_back(std::thread{ w });
			}
		}
	}
	else {
		while (true) {
			std::string input;

			mtx.lock();

			SetConsoleTextAttribute(hcon, dc);
			std::cout << "Enter Input: (Enter ; to quit)" << std::endl;
			std::cin >> input;

			mtx.unlock();

			if (input == ";") {
				break;
			}

			j++;

			int i = 0;

			for (const auto& grammer : grammers) {
				i++;
				CYK c(grammer, input);
				auto w = [=]() mutable {c.start(i, j, std::ref(mtx),hcon); };
				CYKs.push_back(std::thread{ w });
			}
		}
	}

	for (auto& w : CYKs) {
		w.join();
	}

	SetConsoleTextAttribute(hcon, dc);
	system("pause");
}