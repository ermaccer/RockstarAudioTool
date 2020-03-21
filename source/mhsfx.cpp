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

enum eGames {
	GTA2 =1 ,
	GTA3,
	GTA_VICE_CITY,
	MANHUNT,
	STORIES,
};

enum ePlatform {
	PLATFORM_PC = 1,
	PLATFORM_XBOX,
	PLATFORM_PSP,
	PLATFORM_PS2
};


void changeEndINT(int *value)
{
	*value = (*value & 0x000000FFU) << 24 | (*value & 0x0000FF00U) << 8 |
		(*value & 0x00FF0000U) >> 8 | (*value & 0xFF000000U) >> 24;
}


int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		std::cout << "Rockstar Audio Tool (SFX) by ermaccer\n"
			<< "Usage: rsfx <params> <input>\n"
			<< "Params:\n"
			<< " -e            Extracts input file\n"
			<< " -c            Creates file with input name\n"
			<< " -p <platform> Specifies platform\n"
			<< " Platforms: ps2, pc, xbox\n"
			<< " -g <game>     Specifies game\n"
			<< " Games: gta2, gta3, gtavc, mh\n"
			<< " -t <file>     Specifies Sound Data Table (sdt) file\n"
			<< " -l <file>     Specifies List file (lst) or (sdf) for gta2 for named extraction\n"
			<< " -r <file>     Specifies file required for rebuild\n";
		return 1;
    }


	int mode = 0;
	int param = 0;
	int game = 0;
	int platform = 0;
	std::string l_param;
	std::string t_param;
	std::string r_param;
	std::string g_param;
	std::string p_param;

	std::unique_ptr<std::string[]> vSoundNames;
	std::unique_ptr<sdt_entry[]> vSoundTable;
	std::unique_ptr<sdt_entry_ps2_gta[]> vSoundTablePS2;
	std::unique_ptr<sdt_entry_gta2[]> vSoundTableGTA2;

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
		case 'g':
			i++;
			g_param = argv[i];
			break;
		case 'p':
			i++;
			p_param = argv[i];
			break;
		default:
			std::cout << "ERROR: Param does not exist: " << argv[i] << std::endl;
			break;
		}
	}

	if (p_param == "pc")
		platform = PLATFORM_PC;
	else if (p_param == "xbox")
		platform = PLATFORM_XBOX;
	else if (p_param == "ps2")
		platform = PLATFORM_PS2;
	else if (p_param == "psp")
		platform = PLATFORM_PSP;
	else
	{
		std::cout << "ERROR: Unknown platform\n";
		return 1;
	}

	if (g_param == "gta2")
		game = GTA2;
	else if (g_param == "gta3")
		game = GTA3;
	else if (g_param == "gtavc")
		game = GTA_VICE_CITY;
	else if (g_param == "mh")
		game = MANHUNT;
	else if (g_param == "lcs" || g_param == "vcs")
		game = STORIES;
	else
	{
		std::cout << "ERROR: Unknown game\n";
		return 1;
	}


	if (mode == MODE_EXTRACT)
	{
		std::ifstream pFile(argv[argc - 1], std::ifstream::binary);

		std::cout << "Game: " << g_param << std::endl;
		std::cout << "Platform: " << p_param << std::endl;


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
		if (game == GTA2)
			sounds = getSizeToEnd(pTable) / sizeof(sdt_entry_gta2);

		if (platform == PLATFORM_PS2 && (game == GTA3 || game == GTA_VICE_CITY || game == MANHUNT))
			sounds = getSizeToEnd(pTable) / sizeof(sdt_entry_ps2_gta);

		if (!(game == GTA2))
		{
			if (platform == PLATFORM_PS2 && (game == GTA3 || game == GTA_VICE_CITY || game == MANHUNT))
				vSoundTablePS2 = std::make_unique<sdt_entry_ps2_gta[]>(sounds);
			else
		    	vSoundTable = std::make_unique<sdt_entry[]>(sounds);
		}

		else
			vSoundTableGTA2 = std::make_unique<sdt_entry_gta2[]>(sounds);

		vSoundNames = std::make_unique<std::string[]>(sounds);
		std::cout << "INFO: Loaded " << sounds << " sound entries." << std::endl;


		for (int i = 0; i < sounds; i++)
		{
			if (!(game == GTA2))
			{
				if (platform == PLATFORM_PS2 && (game == GTA3 || game == GTA_VICE_CITY || game == MANHUNT))
					pTable.read((char*)&vSoundTablePS2[i], sizeof(sdt_entry_ps2_gta));
				else
				  pTable.read((char*)&vSoundTable[i], sizeof(sdt_entry));
			}
				
			else
                pTable.read((char*)&vSoundTableGTA2[i], sizeof(sdt_entry_gta2));
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

				
				if (game == GTA3 || game == GTA_VICE_CITY)
				{
					std::string strName(szLine, strlen(szLine) - 2);
					vSoundNames[i] = strName;
					i++;
				}
				else if (game == MANHUNT)
				{
					std::string strName(szLine + 4, strlen(szLine + 4) - 2);
					vSoundNames[i] = strName;
					i++;
				}
				else if (game == GTA2)
				{
					std::string strName(szLine, strlen(szLine) - 2);
					strName += ".wav";
					strName.insert(0, "extract\\");
					vSoundNames[i] = strName;
					i++;
				}
				// skip .. and \n

			}
		}
		else
		{
			for (int i = 0; i < sounds; i++)
			{
				std::string format = ".wav";

				if (platform == PLATFORM_PS2)
					format = ".vag";

				std::string temp = "sound" + std::to_string(i);
				temp += format;
				vSoundNames[i] = temp;
			}
		}

		if (!r_param.empty())
		{
			std::ofstream pBuild(r_param, std::ofstream::binary);
			pBuild << "; build file created by rsfx\n"
				"; format:\n";
			if (!(game == GTA2))
			{
				if (platform == PLATFORM_PS2 && (game == GTA3 || game == GTA_VICE_CITY || game == MANHUNT))
					pBuild << "; nothing special required for ps2\n";
				else
         	    	pBuild << "; a - file\n; b - loopStart\n; c - loopEnd\n";
			}
			
			else
				pBuild << "; a - file\n; b - loopStart\n; c - loopEnd\n; d - unknown\n";

			for (int i = 0; i < sounds; i++)
			if (!(game == GTA2))
			{
				if (platform == PLATFORM_PS2 )
					pBuild << "extract\\" << vSoundNames[i] << std::endl;
				else
				{
					if (!l_param.empty())
						pBuild << vSoundNames[i] << " " << vSoundTable[i].loopStart << " " << vSoundTable[i].loopEnd << std::endl;
					else
					    pBuild << "extract\\" << vSoundNames[i] << " " << vSoundTable[i].loopStart << " " << vSoundTable[i].loopEnd << std::endl;
				}
					
			}
			else
			{ 
				if (!l_param.empty())
			    	pBuild  << vSoundNames[i] << " " << vSoundTableGTA2[i].loopStart << " " << vSoundTableGTA2[i].loopEnd << " " << vSoundTableGTA2[i].unk << std::endl;
				else
					pBuild << "extract\\" << vSoundNames[i] << " " << vSoundTableGTA2[i].loopStart << " " << vSoundTableGTA2[i].loopEnd << " " << vSoundTableGTA2[i].unk << std::endl;
			}

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

			if (platform == PLATFORM_PC || platform == PLATFORM_XBOX)
			{
				wav_header wav;
				wav.header = 'FFIR';
				if (!(game == GTA2))
					wav.filesize = (int)vSoundTable[i].size + 36;
				else
					wav.filesize = (int)vSoundTableGTA2[i].size + 36;
				wav.waveheader = 'EVAW';
				wav.format = ' tmf';
				wav.sectionsize = 16;
				wav.waveformat = 1;
				wav.channels = 1;
				if (!(game == GTA2))
				{
					wav.samplespersecond = vSoundTable[i].freq;
					wav.bytespersecond = vSoundTable[i].freq * 2;
				}
				else
				{
					wav.samplespersecond = vSoundTableGTA2[i].freq;
					wav.bytespersecond = vSoundTableGTA2[i].freq * 2;
				}
				wav.blockalign = 2;
				wav.bitspersample = 16;
				wav.dataheader = 'atad';
				wav.datasize = wav.filesize - sizeof(wav_header) - 8;
				// write wave header
				oFile.write((char*)&wav, sizeof(wav_header));
			}

			if (platform == PLATFORM_PS2 && (game == GTA3 || game == GTA_VICE_CITY || game == MANHUNT))
			{
				vag_header vag;
				vag.header = 'VAGp';
				vag.version = 4;
				vag.freq = vSoundTablePS2[i].freq;
				vag.dataSize = vSoundTablePS2[i].size;
				std::string name = g_param;
				name += "_";
				name += p_param;
				name += "_";
				name += std::to_string(i);
				sprintf(vag.name, name.c_str());

				changeEndINT(&vag.version);
				changeEndINT(&vag.dataSize);
				changeEndINT(&vag.freq);
				changeEndINT(&vag.header);
				oFile.write((char*)&vag, sizeof(vag_header));
			}

			

			// get sound data
			if (!(game == GTA2))
			{
				if (platform == PLATFORM_PC || platform == PLATFORM_XBOX)
				{
					pFile.seekg(vSoundTable[i].offset, pFile.beg);
					std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(vSoundTable[i].size);
					pFile.read(dataBuff.get(), vSoundTable[i].size);
					oFile.write(dataBuff.get(), vSoundTable[i].size);
				}
				else if (platform == PLATFORM_PS2)
				{
					pFile.seekg(vSoundTablePS2[i].offset, pFile.beg);
					std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(vSoundTablePS2[i].size);
					pFile.read(dataBuff.get(), vSoundTablePS2[i].size);
					oFile.write(dataBuff.get(), vSoundTablePS2[i].size);
				}
			}
			else 
			{

				pFile.seekg(vSoundTableGTA2[i].offset, pFile.beg);
				std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(vSoundTableGTA2[i].size);
				pFile.read(dataBuff.get(), vSoundTableGTA2[i].size);
				oFile.write(dataBuff.get(), vSoundTableGTA2[i].size);
			}
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
			vSoundTableGTA2 = std::make_unique<sdt_entry_gta2[]>(sounds);
			vSoundTablePS2 = std::make_unique<sdt_entry_ps2_gta[]>(sounds);
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
					if (game == GTA2)
					{
						int loopStart, loopEnd, unk;
						sscanf(szLine, "%s %d %d %d", &tempStr, &loopStart, &loopEnd, &unk);
						sdt_entry_gta2 ent;
						ent.loopStart = loopStart;
						ent.loopEnd = loopEnd;
						ent.unk = unk;
						vSoundTableGTA2[i] = ent;

						std::string name(tempStr, strlen(tempStr));
						vSoundNames[i] = name;
						i++;
					}
					else if (game == GTA3 || game == GTA_VICE_CITY || game == MANHUNT)
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
					else if (platform == PLATFORM_PS2)
					{
						if (game == GTA3 || game == GTA_VICE_CITY || game == MANHUNT)
						{
							sscanf(szLine, "%s", &tempStr);
							std::string name(tempStr, strlen(tempStr));
							vSoundNames[i] = name;
							i++;
						}
					}
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
				if (platform == PLATFORM_PC || platform == PLATFORM_XBOX)
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

					if (game == MANHUNT)
					{
						std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(calcOffsetFromPad(dataSize, 2048));

						pFile.read(dataBuff.get(), calcOffsetFromPad(dataSize, 2048));
						oSFXRaw.write(dataBuff.get(), calcOffsetFromPad(dataSize, 2048));

						vSoundTable[i].freq = wav.samplespersecond;
						vSoundTable[i].size = calcOffsetFromPad(dataSize, 2048);
						vSoundTable[i].offset = baseOffset;

						baseOffset += calcOffsetFromPad(dataSize, 2048);
					}
					else if (game == GTA2 || game == GTA3 || game == GTA_VICE_CITY)
					{
						std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);

						pFile.read(dataBuff.get(), dataSize);
						oSFXRaw.write(dataBuff.get(), dataSize);


						vSoundTable[i].freq = wav.samplespersecond;
						vSoundTable[i].size = dataSize;
						vSoundTable[i].offset = baseOffset;

						if (game == GTA2)
						{
							vSoundTableGTA2[i].freq = wav.samplespersecond;
							vSoundTableGTA2[i].size = dataSize;
							vSoundTableGTA2[i].offset = baseOffset;
						}

						baseOffset += dataSize;
					}


				}
				else if (platform == PLATFORM_PS2)
				{
					if (game == GTA3 || game == GTA_VICE_CITY || game == MANHUNT)
					{
						vag_header vag;
						pFile.read((char*)&vag, sizeof(vag_header));

						if (!(vag.header == 'pGAV'))
						{
							std::cout << "ERROR: Invalid sound format: " << vSoundNames[i] << std::endl;
							return 1;
						}

						std::cout << "Processing: " << vSoundNames[i] << std::endl;

						int dataSize = (int)getSizeToEnd(pFile);

						std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);

						pFile.read(dataBuff.get(), dataSize);
						oSFXRaw.write(dataBuff.get(), dataSize);

						vSoundTablePS2[i].freq = vag.freq;
						vSoundTablePS2[i].offset = baseOffset;
						vSoundTablePS2[i].size = dataSize;

						changeEndINT(&vSoundTablePS2[i].freq);

						baseOffset += dataSize;

						
					}	
				}		
			}
		}

		// build sdt

		std::ofstream oSDT(t_param, std::ofstream::binary);

		for (int i = 0; i < sounds; i++)
		{
			if (game == GTA2)
				oSDT.write((char*)&vSoundTableGTA2[i], sizeof(sdt_entry_gta2));
			else if (game == GTA3 || game == GTA_VICE_CITY || game == MANHUNT)
			{
				if (platform == PLATFORM_PS2)
					oSDT.write((char*)&vSoundTablePS2[i], sizeof(sdt_entry_ps2_gta));
				else
					oSDT.write((char*)&vSoundTable[i], sizeof(sdt_entry));
			}
		}



		std::cout << "Finished." << std::endl; 
	}

}
