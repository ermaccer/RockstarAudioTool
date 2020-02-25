// mhsfx.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <filesystem>


#include "sfx.h"
#include "filef.h"


enum eModes {
	MODE_EXTRACT = 1,
	MODE_CREATE
};


int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		std::cout << "Manhunt (PC) Audio Tool by ermaccer\n"
			<< "Usage: mhsfx <params> <input>\n"
			<< "Params:\n"
			<< " -e        Extracts input file\n"
			<< " -c        Creates file with input name\n"
			<< " -t <file> Specifies Sound Data Table (sdt) file\n"
			<< " -l <file> Specifies List file (lst) for named extraction\n"
			<< " -r <file> Specifies file required for rebuild\n";
		return 1;
    }


	int mode = 0;
	int param = 0;
	std::string l_param;
	std::string t_param;
	std::string r_param;

	std::unique_ptr<std::string[]> vSoundNames;
	std::unique_ptr<sdt_entry[]> vSoundTable;

	for (int i = 1; i < argc - 1; i++)
	{
		if (argv[i][0] != '-' || strlen(argv[i]) != 2) {
			return 1;
		}
		switch (argv[i][1])
		{
		case 'e': mode = MODE_EXTRACT;
			break;
		case 'c': mode = MODE_CREATE;
			break;
		case 'l':
			i++;
			l_param = argv[i];
			break;
		case 't':
			i++;
			t_param = argv[i];
			break;
		case 'r':
			i++;
			r_param = argv[i];
			break;
		default:
			std::cout << "ERROR: Param does not exist: " << argv[i] << std::endl;
			break;
		}
	}

	if (mode == MODE_EXTRACT)
	{
		std::ifstream pFile(argv[argc - 1], std::ifstream::binary);

		if (!pFile)
		{
			std::cout << "ERROR: Could not open " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}

		if (t_param.empty())
		{
			std::cout << "ERROR: Table file not specified!" << std::endl;
			return 1;
		}

		std::ifstream pTable(t_param, std::ifstream::binary);
		
		if (!pTable)
		{
			std::cout << "ERROR: Could not open table file" << argv[argc - 1] << "!" << std::endl;
			return 1;
		}

		int sounds = getSizeToEnd(pTable) / sizeof(sdt_entry);
		vSoundTable = std::make_unique<sdt_entry[]>(sounds);
		vSoundNames = std::make_unique<std::string[]>(sounds);
		std::cout << "INFO: Loaded " << sounds << " sound entries." << std::endl;


		for (int i = 0; i < sounds; i++)
		{
			pTable.read((char*)&vSoundTable[i], sizeof(sdt_entry));
		}


		if (!l_param.empty())
		{
			FILE* pList = fopen(l_param.c_str(), "rb");
			char szLine[1024];

			int i = 0;

			while (fgets(szLine, sizeof(szLine), pList))
			{
				if (szLine[0] == ';' || szLine[0] == '-' || szLine[0] == '\n')
					continue;

				if (szLine[0] == 'q')
					break;

				// skip .. and \n
				std::string strName(szLine + 4, strlen(szLine + 4) - 2);
				vSoundNames[i] = strName;
				i++;
			}
		}
		else
		{
			for (int i = 0; i < sounds; i++)
			{
				std::string temp = "sound" + std::to_string(i) + ".wav";
				vSoundNames[i] = temp;
			}
		}

		if (!r_param.empty())
		{
			std::ofstream pBuild(r_param, std::ofstream::binary);
			
			pBuild << "; build file created by mhsfx\n"
				"; format:\n"
				"; a - file\n; b - loopStart\n; c - loopEnd\n";

			for (int i = 0; i < sounds; i++)
				pBuild << vSoundNames[i] << " " << vSoundTable[i].loopStart << " " << vSoundTable[i].loopEnd << std::endl;
		}

		for (int i = 0; i < sounds; i++)
		{
			if (checkSlash(vSoundNames[i]))
				std::experimental::filesystem::create_directories(splitString(vSoundNames[i],false));

			if (l_param.empty())
			{
				std::experimental::filesystem::create_directory("extract");
				vSoundNames[i].insert(0, "extract\\");
			}

			std::ofstream oFile(vSoundNames[i], std::ofstream::binary);

			std::cout << "Processing: " << vSoundNames[i] << std::endl;

			wav_header wav;
			wav.header = 'FFIR';
			wav.filesize = (int)vSoundTable[i].size + 36;
			wav.waveheader = 'EVAW';
			wav.format = ' tmf';
			wav.sectionsize = 16;
			wav.waveformat =1;
			wav.channels = 1;
			wav.samplespersecond = vSoundTable[i].freq;
			wav.bytespersecond = vSoundTable[i].freq * 2;
			wav.blockalign = 2;
			wav.bitspersample = 16;
			wav.dataheader = 'atad';
			wav.datasize = wav.filesize - sizeof(wav_header) - 8;
			// write wave header
			oFile.write((char*)&wav, sizeof(wav_header));

			// get sound data
			pFile.seekg(vSoundTable[i].offset, pFile.beg);
			std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(vSoundTable[i].size);
			pFile.read(dataBuff.get(), vSoundTable[i].size);
			oFile.write(dataBuff.get(), vSoundTable[i].size);
		}
	}
	if (mode == MODE_CREATE)
	{
		if (r_param.empty() || t_param.empty())
		{
			std::cout << "ERROR: Rebuild or table file was not specified!" << std::endl;
			return 1;
		}

		FILE* pRebuild = fopen(r_param.c_str(), "rb");

		if (!pRebuild)
		{
			std::cout << "ERROR: Could not open " << r_param << "!" << std::endl;
			return 1;
		}
		
		int sounds = 0;

		if (pRebuild)
		{
			char szLine[1024];
			int i = 0;
			// could use vector here but lazy
			while (fgets(szLine, sizeof(szLine), pRebuild))
			{
				if (szLine[0] == ';' || szLine[0] == '\n')
					continue;

				char tempStr[256];
				if (sscanf(szLine, "%s", &tempStr) == 1)
				{
					i++;
				}
			}
			sounds = i;
			vSoundTable = std::make_unique<sdt_entry[]>(sounds);
			vSoundNames = std::make_unique<std::string[]>(sounds);

			i = 0;
			fseek(pRebuild, 0, SEEK_SET);
			while (fgets(szLine, sizeof(szLine), pRebuild))
			{
				if (szLine[0] == ';' || szLine[0] == '\n')
					continue;

				char tempStr[256];
				if (sscanf(szLine, "%s", &tempStr) == 1)
				{
					int loopStart, loopEnd;
					sscanf(szLine, "%s %d %d", &tempStr, &loopStart, &loopEnd);
					sdt_entry ent;
					ent.loopStart = loopStart;
					ent.loopEnd = loopEnd;
					vSoundTable[i] = ent;

					std::string name(tempStr, strlen(tempStr));
					vSoundNames[i] = name;
					i++;
				}
			}
		}

		// build raw

		std::cout << "INFO: Building archive for " << sounds << " sounds." << std::endl;

		if (!argv[argc - 1])
		{
			std::cout << "ERROR: Output file not specified!" << std::endl;
			return 1;
		}

		std::ofstream oSFXRaw(argv[argc - 1], std::ofstream::binary);

		int baseOffset = 0;

		for (int i = 0; i < sounds; i++)
		{
			std::ifstream pFile(vSoundNames[i], std::ifstream::binary);

			if (!pFile)
			{
				std::cout << "ERROR: Could not open " << vSoundNames[i] << "!" << std::endl;
				return 1;
			}

			if (pFile)
			{
				wav_header wav;
				pFile.read((char*)&wav, sizeof(wav_header));

				// mono and pcm audio only
				if (!(wav.channels == 1 || wav.waveformat == 1))
				{
					std::cout << "ERROR: " << vSoundNames[i] << " invalid sound format!" << std::endl;
					return 1;
				}

				std::cout << "Processing: " << vSoundNames[i] << std::endl;

				int dataSize = (int)getSizeToEnd(pFile);
				std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);

				pFile.read(dataBuff.get(), dataSize);
				oSFXRaw.write(dataBuff.get(), dataSize);

				vSoundTable[i].freq = wav.samplespersecond;
				vSoundTable[i].size = calcOffsetFromPad(dataSize, 2048);
				vSoundTable[i].offset = baseOffset;

				baseOffset += calcOffsetFromPad(dataSize, 2048); 
			}
		}

		// build sdt

		std::ofstream oSDT(t_param, std::ofstream::binary);

		for (int i = 0; i < sounds; i++)
    		oSDT.write((char*)&vSoundTable[i], sizeof(sdt_entry));

		std::cout << "Finished." << std::endl; 
	}

}
