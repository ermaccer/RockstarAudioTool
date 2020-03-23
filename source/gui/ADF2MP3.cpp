#include "ADF2MP3.h"
#include <fstream>
#include <memory>
#include <Windows.h>
#include "filef.h"

ADF2MP3::ADF2MP3(std::string in, std::string out)
{
	inPath = in;
	outPath = out;
}

bool ADF2MP3::Process()
{
	std::ifstream pFile(inPath, std::ifstream::binary);

	if (!pFile)
	{
		MessageBoxA(0, "Could not open input file!", 0, MB_ICONWARNING);
		return false;
	}
	if (pFile)
	{
		int dataSize = (int)getSizeToEnd(pFile);
		std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);
		pFile.read(dataBuff.get(), dataSize);

		for (int i = 0; i < dataSize; i++)
			dataBuff[i] ^= 0x22;

		std::ofstream oFile(outPath, std::ofstream::binary);
		oFile.write(dataBuff.get(), dataSize);
		return true;
	}

	return false;
}
