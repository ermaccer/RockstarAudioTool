#pragma once
#include <string>
#include <fstream>

std::streampos getSizeToEnd(std::ifstream& is);
bool checkSlash(std::string& str, bool first = false);
std::string getWideStr(std::ifstream &file, bool f = false);
std::string convertWide(std::string& str);
std::string splitString(std::string& str, bool file);
int calcOffsetFromPad(int val, int padsize);
