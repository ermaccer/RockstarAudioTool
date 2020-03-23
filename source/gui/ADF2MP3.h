#pragma once
#include <string>

class ADF2MP3 {
private:
	std::string inPath;
	std::string outPath;
public:
	ADF2MP3(std::string in, std::string out);
	bool Process();
};