#include "SFXManager.h"
#include "sfx.h"
#include <CommCtrl.h>
#include <shlobj.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <sstream>
#include <math.h>
#include <filesystem>

#include "filef.h"

void changeEndINT(int *value)
{
	*value = (*value & 0x000000FFU) << 24 | (*value & 0x0000FF00U) << 8 |
		(*value & 0x00FF0000U) >> 8 | (*value & 0xFF000000U) >> 24;
}


SFXManager::SFXManager()
{
}
SFXManager::SFXManager(std::string r_path, std::string s_path, std::string l_path, std::string c_path, std::string o_path, int g, int p,int w)
{
	rawPath = r_path;
	sdtPath = s_path;
	lstPath = l_path;
	cfgPath = c_path;
	outPath = o_path;
	mode = w;
	game = g;
	platform = p;
}

void SFXManager::AttachProgressBar(HWND * bar)
{
	progressBar = bar;
	SendMessage(*progressBar, PBM_SETSTEP, 1, 0);
}

void SFXManager::AttachFilenameText(HWND * txt)
{
	filename = txt;
}

void SFXManager::AttachNumbersText(HWND * txt)
{
	numbers = txt;
}

void SFXManager::IncrementProgressBar(int value)
{
	for (int i = 0; i < value; i++)
	SendMessage(*progressBar, PBM_STEPIT, 0, 0);
}

const char * SFXManager::GetOperationMode()
{
	char szPlatformName[64];
	char szGameName[128];
	char result[256];

	switch (game)
	{
	case GAME_GTA2: sprintf(szGameName, "Grand Theft Auto 2"); break;
	case GAME_GTA3_VC: sprintf(szGameName, "Grand Theft Auto III/Vice City"); break;
	case GAME_STORIES: sprintf(szGameName, "Grand Theft Auto Stories"); break;
	case GAME_MANHUNT: sprintf(szGameName, "Manhunt"); break;
	}

	switch (platform)
	{
	case PLATFORM_PC: sprintf(szPlatformName, "PC"); break;
	case PLATFORM_PS2: sprintf(szPlatformName, "Playstation 2"); break;
	case PLATFORM_PSP: sprintf(szPlatformName, "Playstation Portable"); break;
	case PLATFORM_XBOX: sprintf(szPlatformName, "XBOX"); break;
	}

	sprintf(result, "%s | %s", szGameName, szPlatformName);

	return result;
}

bool SFXManager::Process()
{

	SendMessage(*progressBar, PBM_SETPOS, 0, 0);
	SendMessage(*progressBar, PBM_SETSTATE, PBST_NORMAL, 0);

	if (game == GAME_GTA2)
		return ProcessGTA2();

	if (platform == PLATFORM_PC)
	{
		if (game == GAME_GTA3_VC || (game == GAME_MANHUNT && mode == MODE_EXTRACT))
			return ProcessGTA3VC();
		if (game == GAME_MANHUNT && mode == MODE_CREATE)
			return ProcessManhuntBuild();
	}
	if (platform == PLATFORM_XBOX)
	{
		if (game == GAME_GTA3_VC)
			return ProcessGTA3VCXBOX();
		if (game == GAME_MANHUNT && mode == MODE_EXTRACT)
			return ProcessGTA3VC();
		if (game == GAME_MANHUNT && mode == MODE_CREATE)
			return ProcessManhuntBuild();
	}

	if (platform == PLATFORM_PS2)
	{
		if (game == GAME_GTA3_VC || game == GAME_MANHUNT)
		    return ProcessPS23VC();
		if (game == GAME_STORIES)
			return ProcessStoriesPS2();
	}

	if (platform == PLATFORM_PSP)
	{
		if (game == GAME_STORIES)
			return ProcessStoriesPSP();
	}


}

bool SFXManager::ProcessGTA2()
{
	if (mode == MODE_EXTRACT)
	{
		std::unique_ptr<sdt_entry_gta2[]> vSoundTable;
		std::unique_ptr<std::string[]> vSoundNames;
		std::ifstream pFile(rawPath, std::ifstream::binary);

		if (!std::experimental::filesystem::exists(outPath))
			std::experimental::filesystem::create_directory(outPath);


		if (!pFile)
		{
			MessageBoxA(0, "Failed to open RAW file!", 0, MB_ICONWARNING);
			return false;
		}

		if (pFile)
		{
			std::ifstream pTable(sdtPath, std::ifstream::binary);
			if (!pTable)
			{
				MessageBoxA(0, "Failed to open SDT file!", 0, MB_ICONWARNING);
				return false;
			}

			int sounds = getSizeToEnd(pTable) / sizeof(sdt_entry_gta2);
			vSoundTable = std::make_unique<sdt_entry_gta2[]>(sounds);
			vSoundNames = std::make_unique<std::string[]>(sounds);

			for (int i = 0; i < sounds; i++)
				pTable.read((char*)&vSoundTable[i], sizeof(sdt_entry_gta2));

			std::experimental::filesystem::current_path(
				std::experimental::filesystem::system_complete(std::experimental::filesystem::path(outPath)));

			if (!lstPath.empty())
			{
				FILE* pList = fopen(lstPath.c_str(), "rb");
				char szLine[1024];

				int i = 0;

				while (fgets(szLine, sizeof(szLine), pList))
				{
					if (szLine[0] == ';' || szLine[0] == '-' || szLine[0] == '\n')
						continue;

					if (szLine[0] == 'q')
						break;
						std::string strName(szLine, strlen(szLine) - 2);
						strName += ".wav";
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


			// update progress bar
			SendMessage(*progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, sounds));

			// generate cfg

			if (!cfgPath.empty())
			{
				std::ofstream pBuild(cfgPath, std::ofstream::binary);
				if (!pBuild)
				{
					MessageBoxA(0, "Failed to open CFG for writing!", 0, 0);
					return false;
				}
				pBuild << "; build file created by rsfx\n"
					"; format:\n";
				pBuild << "; a - file\n; b - loopStart\n; c - loopEnd\n; d - unknown\n";

				for (int i = 0; i < sounds; i++)
					pBuild << vSoundNames[i] << " " << vSoundTable[i].loopStart << " " << vSoundTable[i].loopEnd << " " << vSoundTable[i].unk << std::endl;

				pBuild.close();
			}

			// extract

			for (int i = 0; i < sounds; i++)
			{

				sprintf(progressBuffer, "%d/%d", i, sounds - 1);
				SetWindowText(*filename, vSoundNames[i].c_str());
				SetWindowText(*numbers, progressBuffer);
				SendMessage(*progressBar, PBM_STEPIT, 0, 0);

				if (checkSlash(vSoundNames[i]))
					std::experimental::filesystem::create_directories(splitString(vSoundNames[i], false));

				std::ofstream oFile(vSoundNames[i], std::ofstream::binary);

				wav_header wav;
				wav.header = 'FFIR';
				wav.channels = 1;
				wav.filesize = (int)vSoundTable[i].size + 36;
				wav.waveheader = 'EVAW';
				wav.format = ' tmf';
				wav.sectionsize = 16;
				wav.waveformat = 1;
				wav.bitspersample = 16;
				wav.samplespersecond = vSoundTable[i].freq;
				wav.bytespersecond = vSoundTable[i].freq * 2;
				wav.blockalign = 2;

				wav.dataheader = 'atad';
				wav.datasize = wav.filesize - sizeof(wav_header) - 8;
				// write wave header
				oFile.write((char*)&wav, sizeof(wav_header));

				pFile.seekg(vSoundTable[i].offset, pFile.beg);
				std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(vSoundTable[i].size);
				pFile.read(dataBuff.get(), vSoundTable[i].size);
				oFile.write(dataBuff.get(), vSoundTable[i].size);

			}

			pFile.close();
			pTable.close();

			return true;
		}
	}
	if (mode == MODE_CREATE)
	{
		std::unique_ptr<sdt_entry_gta2[]> vSoundTable;
		std::unique_ptr<std::string[]> vSoundNames;

		FILE* pRebuild = fopen(cfgPath.c_str(), "rb");

		if (!pRebuild)
		{
			MessageBoxA(0, "Failed to open CFG file!", 0, MB_ICONWARNING);
			return false;
		}

		if (pRebuild)
		{
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
				vSoundTable = std::make_unique<sdt_entry_gta2[]>(sounds);
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
					int loopStart, loopEnd, unk;
					sscanf(szLine, "%s %d %d %d", &tempStr, &loopStart, &loopEnd, &unk);
					sdt_entry_gta2 ent;
					ent.loopStart = loopStart;
					ent.loopEnd = loopEnd;
					ent.unk = unk;
					vSoundTable[i] = ent;

					std::string name(tempStr, strlen(tempStr));
					vSoundNames[i] = name;
					i++;
					}
				}

				std::ofstream oSFXRaw(rawPath, std::ofstream::binary);

				int baseOffset = 0;

				// update progress bar
				SendMessage(*progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, sounds));

				std::experimental::filesystem::current_path(
					std::experimental::filesystem::system_complete(std::experimental::filesystem::path(outPath)));

				for (int i = 0; i < sounds; i++)
				{
					std::ifstream pFile(vSoundNames[i], std::ifstream::binary);

					sprintf(progressBuffer, "%d/%d", i, sounds - 1);
					SetWindowText(*filename, vSoundNames[i].c_str());
					SetWindowText(*numbers, progressBuffer);
					SendMessage(*progressBar, PBM_STEPIT, 0, 0);

					if (!pFile)
					{
						char buffer[256];
						sprintf(buffer, "Could not open %s!", vSoundNames[i].c_str());
						MessageBoxA(0, buffer, 0, 0);
						SendMessage(*progressBar, PBM_SETSTATE, PBST_ERROR, 0);
						return false;
					}

					if (pFile)
					{
						wav_header wav;
						pFile.read((char*)&wav, sizeof(wav_header));

						// mono and pcm audio only
						if (!(wav.channels == 1 || wav.waveformat == 1))
						{
							char buffer[256];
							sprintf(buffer, "Invalid sound format: %s", vSoundNames[i].c_str());
							MessageBoxA(0, buffer, 0, 0);
							SendMessage(*progressBar, PBM_SETSTATE, PBST_ERROR, 0);
							return false;
						}

						int dataSize = (int)getSizeToEnd(pFile);

						std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);

						pFile.read(dataBuff.get(), dataSize);
						oSFXRaw.write(dataBuff.get(), dataSize);


						vSoundTable[i].freq = wav.samplespersecond;
						vSoundTable[i].size = dataSize;
						vSoundTable[i].offset = baseOffset;

						baseOffset += dataSize;
					}
					pFile.close();
				}
				// build sdt

				std::ofstream oSDT(sdtPath, std::ofstream::binary);
				SendMessage(*progressBar, PBM_SETPOS,0, 0);
				for (int i = 0; i < sounds; i++)
				{
					sprintf(progressBuffer, "%d/%d", i, sounds - 1);
					SetWindowText(*filename, "Building SDT");
					SetWindowText(*numbers, progressBuffer);
					SendMessage(*progressBar, PBM_STEPIT, 0, 0);
					oSDT.write((char*)&vSoundTable[i], sizeof(sdt_entry_gta2));
				}

				return true;
			}
		}
		fclose(pRebuild);
	}

	return false;
}

bool SFXManager::ProcessGTA3VC()
{
	if (mode == MODE_EXTRACT)
	{

		std::unique_ptr<sdt_entry[]> vSoundTable;
		std::unique_ptr<std::string[]> vSoundNames;
		std::ifstream pFile(rawPath, std::ifstream::binary);

		if (!std::experimental::filesystem::exists(outPath))
			std::experimental::filesystem::create_directory(outPath);


		if (!pFile)
		{
			MessageBoxA(0, "Failed to open RAW file!", 0, MB_ICONWARNING);
			return false;
		}

		if (pFile)
		{
			std::ifstream pTable(sdtPath, std::ifstream::binary);
			if (!pTable)
			{
				MessageBoxA(0, "Failed to open SDT file!", 0, MB_ICONWARNING);
				return false;
			}

			int sounds = getSizeToEnd(pTable) / sizeof(sdt_entry);
			vSoundTable = std::make_unique<sdt_entry[]>(sounds);
			vSoundNames = std::make_unique<std::string[]>(sounds);

			for (int i = 0; i < sounds; i++)
				pTable.read((char*)&vSoundTable[i], sizeof(sdt_entry));

			std::experimental::filesystem::current_path(
				std::experimental::filesystem::system_complete(std::experimental::filesystem::path(outPath)));

			if (!lstPath.empty())
			{
				FILE* pList = fopen(lstPath.c_str(), "rb");
				char szLine[1024];

				int i = 0;

				while (fgets(szLine, sizeof(szLine), pList))
				{
					if (szLine[0] == ';' || szLine[0] == '-' || szLine[0] == '\n' || szLine[0] == ' ')
						continue;

					if (szLine[0] == 'q')
						break;

					if (szLine[0] == '.')
					{
						std::string strName(szLine + 4, strlen(szLine + 4) - 2);
						vSoundNames[i] = strName;		
					}
					else
					{
						std::string strName(szLine, strlen(szLine) - 2);
						vSoundNames[i] = strName;
					}
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


			// update progress bar
			SendMessage(*progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, sounds));

			// generate cfg

			if (!cfgPath.empty())
			{
				std::ofstream pBuild(cfgPath, std::ofstream::binary);
				if (!pBuild)
				{
					MessageBoxA(0, "Failed to open CFG for writing!", 0, 0);
					return false;
				}
				pBuild << "; build file created by rsfx\n"
					"; format:\n";
				pBuild << "; a - file\n; b - loopStart\n; c - loopEnd\n";

				for (int i = 0; i < sounds; i++)
					pBuild << vSoundNames[i] << " " << vSoundTable[i].loopStart << " " << vSoundTable[i].loopEnd << std::endl;

				pBuild.close();
			}


			// extract
			for (int i = 0; i < sounds; i++)
			{

				sprintf(progressBuffer, "%d/%d", i, sounds - 1);
				SetWindowText(*filename, vSoundNames[i].c_str());
				SetWindowText(*numbers, progressBuffer);
				SendMessage(*progressBar, PBM_STEPIT, 0, 0);


				if (checkSlash(vSoundNames[i]))
					std::experimental::filesystem::create_directories(splitString(vSoundNames[i], false));

				std::ofstream oFile(vSoundNames[i], std::ofstream::binary);

				wav_header wav;
				wav.header = 'FFIR';
				wav.channels = 1;
				wav.filesize = (int)vSoundTable[i].size + 36;
				wav.waveheader = 'EVAW';
				wav.format = ' tmf';
				wav.sectionsize = 16;
				wav.waveformat = 1;
				if (platform == PLATFORM_XBOX && game == GAME_GTA3_VC)
					wav.waveformat = 0x69;
				wav.bitspersample = 16;
				wav.samplespersecond = vSoundTable[i].freq;
				wav.bytespersecond = vSoundTable[i].freq * 2;
				wav.blockalign = 2;

				wav.dataheader = 'atad';
				wav.datasize = wav.filesize - sizeof(wav_header) - 8;
				// write wave header
				oFile.write((char*)&wav, sizeof(wav_header));

				pFile.seekg(vSoundTable[i].offset, pFile.beg);
				std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(vSoundTable[i].size);
				pFile.read(dataBuff.get(), vSoundTable[i].size);
				oFile.write(dataBuff.get(), vSoundTable[i].size);
				oFile.close();

			}

			pFile.close();
			pTable.close();

			return true;
		}
	}
	if (mode == MODE_CREATE)
	{
		std::unique_ptr<sdt_entry[]> vSoundTable;
		std::unique_ptr<std::string[]> vSoundNames;

		FILE* pRebuild = fopen(cfgPath.c_str(), "rb");

		if (!pRebuild)
		{
			MessageBoxA(0, "Failed to open CFG file!", 0, MB_ICONWARNING);
			return false;
		}

		if (pRebuild)
		{
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

				std::ofstream oSFXRaw(rawPath, std::ofstream::binary);

				int baseOffset = 0;

				// update progress bar
				SendMessage(*progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, sounds));

				std::experimental::filesystem::current_path(
					std::experimental::filesystem::system_complete(std::experimental::filesystem::path(outPath)));


				for (int i = 0; i < sounds; i++)
				{
					std::ifstream pFile(vSoundNames[i], std::ifstream::binary);

					sprintf(progressBuffer, "%d/%d", i, sounds - 1);
					SetWindowText(*filename, vSoundNames[i].c_str());
					SetWindowText(*numbers, progressBuffer);
					SendMessage(*progressBar, PBM_STEPIT, 0, 0);

					if (!pFile)
					{
						char buffer[256];
						sprintf(buffer, "Could not open %s!", vSoundNames[i].c_str());
						MessageBoxA(0, buffer, 0, 0);
						SendMessage(*progressBar, PBM_SETSTATE, PBST_ERROR, 0);
						return false;
					}

					if (pFile)
					{
						wav_header wav;
						pFile.read((char*)&wav, sizeof(wav_header));

						// mono and pcm audio only
						if (!(wav.channels == 1 || wav.waveformat == 1))
						{
							char buffer[256];
							sprintf(buffer, "Invalid sound format: %s", vSoundNames[i].c_str());
							MessageBoxA(0, buffer, 0, 0);
							SendMessage(*progressBar, PBM_SETSTATE, PBST_ERROR, 0);
							return false;
						}

						int dataSize = (int)getSizeToEnd(pFile);

						std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);

						pFile.read(dataBuff.get(), dataSize);
						oSFXRaw.write(dataBuff.get(), dataSize);


						vSoundTable[i].freq = wav.samplespersecond;
						vSoundTable[i].size = dataSize;
						vSoundTable[i].offset = baseOffset;

						baseOffset += dataSize;
					}
					pFile.close();
				}
				// build sdt

				std::ofstream oSDT(sdtPath, std::ofstream::binary);
				SendMessage(*progressBar, PBM_SETPOS, 0, 0);
				for (int i = 0; i < sounds; i++)
				{
					sprintf(progressBuffer, "%d/%d", i, sounds - 1);
					SetWindowText(*filename, "Building SDT");
					SetWindowText(*numbers, progressBuffer);
					SendMessage(*progressBar, PBM_STEPIT, 0, 0);
					oSDT.write((char*)&vSoundTable[i], sizeof(sdt_entry));
				}

				return true;
			}
		}
		fclose(pRebuild);
	}

	return false;
}

bool SFXManager::ProcessGTA3VCXBOX()
{
	if (mode == MODE_EXTRACT)
	{

		std::unique_ptr<sdt_entry[]> vSoundTable;
		std::unique_ptr<std::string[]> vSoundNames;
		std::ifstream pFile(rawPath, std::ifstream::binary);

		if (!std::experimental::filesystem::exists(outPath))
			std::experimental::filesystem::create_directory(outPath);


		if (!pFile)
		{
			MessageBoxA(0, "Failed to open RAW file!", 0, MB_ICONWARNING);
			return false;
		}

		if (pFile)
		{
			std::ifstream pTable(sdtPath, std::ifstream::binary);
			if (!pTable)
			{
				MessageBoxA(0, "Failed to open SDT file!", 0, MB_ICONWARNING);
				return false;
			}

			int sounds = getSizeToEnd(pTable) / sizeof(sdt_entry);
			vSoundTable = std::make_unique<sdt_entry[]>(sounds);
			vSoundNames = std::make_unique<std::string[]>(sounds);

			for (int i = 0; i < sounds; i++)
				pTable.read((char*)&vSoundTable[i], sizeof(sdt_entry));

			std::experimental::filesystem::current_path(
				std::experimental::filesystem::system_complete(std::experimental::filesystem::path(outPath)));

			if (!lstPath.empty())
			{
				FILE* pList = fopen(lstPath.c_str(), "rb");
				char szLine[1024];

				int i = 0;

				while (fgets(szLine, sizeof(szLine), pList))
				{
					if (szLine[0] == ';' || szLine[0] == '-' || szLine[0] == '\n' || szLine[0] == ' ')
						continue;

					if (szLine[0] == 'q')
						break;

					if (szLine[0] == '.')
					{
						std::string strName(szLine + 4, strlen(szLine + 4) - 2);
						vSoundNames[i] = strName;
					}
					else
					{
						std::string strName(szLine, strlen(szLine) - 2);
						vSoundNames[i] = strName;
					}
					i++;

				}
			}
			else
			{
				for (int i = 0; i < sounds; i++)
				{
					std::string temp = "sound" + std::to_string(i) + ".xwav";
					vSoundNames[i] = temp;
				}

			}


			// update progress bar
			SendMessage(*progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, sounds));

			// generate cfg

			if (!cfgPath.empty())
			{
				std::ofstream pBuild(cfgPath, std::ofstream::binary);
				if (!pBuild)
				{
					MessageBoxA(0, "Failed to open CFG for writing!", 0, 0);
					return false;
				}
				pBuild << "; build file created by rsfx\n"
					"; format:\n";
				pBuild << "; a - file\n; b - loopStart\n; c - loopEnd\n";

				for (int i = 0; i < sounds; i++)
					pBuild << vSoundNames[i] << " " << vSoundTable[i].loopStart << " " << vSoundTable[i].loopEnd << std::endl;

				pBuild.close();
			}


			// extract
			for (int i = 0; i < sounds; i++)
			{

				sprintf(progressBuffer, "%d/%d", i, sounds - 1);
				SetWindowText(*filename, vSoundNames[i].c_str());
				SetWindowText(*numbers, progressBuffer);
				SendMessage(*progressBar, PBM_STEPIT, 0, 0);


				if (checkSlash(vSoundNames[i]))
					std::experimental::filesystem::create_directories(splitString(vSoundNames[i], false));

				std::ofstream oFile(vSoundNames[i], std::ofstream::binary);

				wav_header_xbox wav = { 'FFIR',vSoundTable[i].size + 36 + 20,'EVAW',' tmf',20,0x69,1,vSoundTable[i].freq,vSoundTable[i].freq,2048,4,2,64,'tcaf',4,vSoundTable[i].size + 36 + 20 * 10,'atad',vSoundTable[i].size + 36 + 20 + 60 };
				oFile.write((char*)&wav, sizeof(wav_header_xbox));

				pFile.seekg(vSoundTable[i].offset, pFile.beg);
				std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(vSoundTable[i].size);
				pFile.read(dataBuff.get(), vSoundTable[i].size);
				oFile.write(dataBuff.get(), vSoundTable[i].size);
				oFile.close();

			}

			pFile.close();
			pTable.close();

			return true;
		}
	}
	if (mode == MODE_CREATE)
	{
		std::unique_ptr<sdt_entry[]> vSoundTable;
		std::unique_ptr<std::string[]> vSoundNames;

		FILE* pRebuild = fopen(cfgPath.c_str(), "rb");

		if (!pRebuild)
		{
			MessageBoxA(0, "Failed to open CFG file!", 0, MB_ICONWARNING);
			return false;
		}

		if (pRebuild)
		{
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

				std::ofstream oSFXRaw(rawPath, std::ofstream::binary);

				int baseOffset = 0;

				// update progress bar
				SendMessage(*progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, sounds));

				std::experimental::filesystem::current_path(
					std::experimental::filesystem::system_complete(std::experimental::filesystem::path(outPath)));


				for (int i = 0; i < sounds; i++)
				{
					std::ifstream pFile(vSoundNames[i], std::ifstream::binary);

					sprintf(progressBuffer, "%d/%d", i, sounds - 1);
					SetWindowText(*filename, vSoundNames[i].c_str());
					SetWindowText(*numbers, progressBuffer);
					SendMessage(*progressBar, PBM_STEPIT, 0, 0);

					if (!pFile)
					{
						char buffer[256];
						sprintf(buffer, "Could not open %s!", vSoundNames[i].c_str());
						MessageBoxA(0, buffer, 0, 0);
						SendMessage(*progressBar, PBM_SETSTATE, PBST_ERROR, 0);
						return false;
					}

					if (pFile)
					{
						wav_header_xbox wav;
						pFile.read((char*)&wav, sizeof(wav_header_xbox));

						// mono and adpcm audio only
						if (!(wav.channels == 1 || wav.waveformat == 0x69 || wav.waveformat == 0x11))
						{
							char buffer[256];
							sprintf(buffer, "Invalid sound format: %s", vSoundNames[i].c_str());
							MessageBoxA(0, buffer, 0, 0);
							SendMessage(*progressBar, PBM_SETSTATE, PBST_ERROR, 0);
							return false;
						}

						int dataSize = (int)getSizeToEnd(pFile);

						std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);

						pFile.read(dataBuff.get(), dataSize);
						oSFXRaw.write(dataBuff.get(), dataSize);


						vSoundTable[i].freq = wav.samplespersecond;
						vSoundTable[i].size = dataSize;
						vSoundTable[i].offset = baseOffset;

						baseOffset += dataSize;
					}
					pFile.close();
				}
				// build sdt

				std::ofstream oSDT(sdtPath, std::ofstream::binary);
				SendMessage(*progressBar, PBM_SETPOS, 0, 0);
				for (int i = 0; i < sounds; i++)
				{
					sprintf(progressBuffer, "%d/%d", i, sounds - 1);
					SetWindowText(*filename, "Building SDT");
					SetWindowText(*numbers, progressBuffer);
					SendMessage(*progressBar, PBM_STEPIT, 0, 0);
					oSDT.write((char*)&vSoundTable[i], sizeof(sdt_entry));
				}

				return true;
			}
		}
		fclose(pRebuild);
	}

	return false;
}


bool SFXManager::ProcessManhuntBuild()
{
	std::unique_ptr<sdt_entry[]> vSoundTable;
	std::unique_ptr<std::string[]> vSoundNames;

	FILE* pRebuild = fopen(cfgPath.c_str(), "rb");

	if (!pRebuild)
	{
		MessageBoxA(0, "Failed to open CFG file!", 0, MB_ICONWARNING);
		return false;
	}

	if (pRebuild)
	{
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

			std::ofstream oSFXRaw(rawPath, std::ofstream::binary);

			int baseOffset = 0;

			// update progress bar
			SendMessage(*progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, sounds));

			std::experimental::filesystem::current_path(
				std::experimental::filesystem::system_complete(std::experimental::filesystem::path(outPath)));


			for (int i = 0; i < sounds; i++)
			{
				std::ifstream pFile(vSoundNames[i], std::ifstream::binary);

				sprintf(progressBuffer, "%d/%d", i, sounds - 1);
				SetWindowText(*filename, vSoundNames[i].c_str());
				SetWindowText(*numbers, progressBuffer);
				SendMessage(*progressBar, PBM_STEPIT, 0, 0);

				if (!pFile)
				{
					char buffer[256];
					sprintf(buffer, "Could not open %s!", vSoundNames[i].c_str());
					MessageBoxA(0, buffer, 0, 0);
					SendMessage(*progressBar, PBM_SETSTATE, PBST_ERROR, 0);
					return false;
				}

				if (pFile)
				{
					wav_header wav;
					pFile.read((char*)&wav, sizeof(wav_header));

					// mono and pcm audio only
					if (!(wav.channels == 1 || wav.waveformat == 1))
					{
						char buffer[256];
						sprintf(buffer, "Invalid sound format: %s", vSoundNames[i].c_str());
						MessageBoxA(0, buffer, 0, 0);
						SendMessage(*progressBar, PBM_SETSTATE, PBST_ERROR, 0);
						return false;
					}

					int dataSize = (int)getSizeToEnd(pFile);

					std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(calcOffsetFromPad(dataSize, 2048));

					pFile.read(dataBuff.get(), calcOffsetFromPad(dataSize, 2048));
					oSFXRaw.write(dataBuff.get(), calcOffsetFromPad(dataSize, 2048));

					vSoundTable[i].freq = wav.samplespersecond;
					vSoundTable[i].size = calcOffsetFromPad(dataSize, 2048);
					vSoundTable[i].offset = baseOffset;

					baseOffset += calcOffsetFromPad(dataSize, 2048);
				}
				pFile.close();
			}
			// build sdt

			std::ofstream oSDT(sdtPath, std::ofstream::binary);
			SendMessage(*progressBar, PBM_SETPOS, 0, 0);
			for (int i = 0; i < sounds; i++)
			{
				sprintf(progressBuffer, "%d/%d", i, sounds - 1);
				SetWindowText(*filename, "Building SDT");
				SetWindowText(*numbers, progressBuffer);
				SendMessage(*progressBar, PBM_STEPIT, 0, 0);
				oSDT.write((char*)&vSoundTable[i], sizeof(sdt_entry));
			}
			fclose(pRebuild);
			return true;
		}
	}

	return false;
}

bool SFXManager::ProcessPS23VC()
{
	if (mode == MODE_EXTRACT)
	{
		std::unique_ptr<sdt_entry_ps2_gta[]> vSoundTable;
		std::unique_ptr<std::string[]> vSoundNames;
		std::ifstream pFile(rawPath, std::ifstream::binary);

		if (!std::experimental::filesystem::exists(outPath))
			std::experimental::filesystem::create_directory(outPath);


		if (!pFile)
		{
			MessageBoxA(0, "Failed to open RAW file!", 0, MB_ICONWARNING);
			return false;
		}

		if (pFile)
		{
			std::ifstream pTable(sdtPath, std::ifstream::binary);
			if (!pTable)
			{
				MessageBoxA(0, "Failed to open SDT file!", 0, MB_ICONWARNING);
				return false;
			}

			int sounds = getSizeToEnd(pTable) / sizeof(sdt_entry_ps2_gta);
			vSoundTable = std::make_unique<sdt_entry_ps2_gta[]>(sounds);
			vSoundNames = std::make_unique<std::string[]>(sounds);

			for (int i = 0; i < sounds; i++)
				pTable.read((char*)&vSoundTable[i], sizeof(sdt_entry_ps2_gta));

			std::experimental::filesystem::current_path(
				std::experimental::filesystem::system_complete(std::experimental::filesystem::path(outPath)));

			if (!lstPath.empty())
			{
				FILE* pList = fopen(lstPath.c_str(), "rb");
				char szLine[1024];

				int i = 0;

				while (fgets(szLine, sizeof(szLine), pList))
				{
					if (szLine[0] == ';' || szLine[0] == '-' || szLine[0] == '\n' || szLine[0] == ' ')
						continue;

					if (szLine[0] == 'q')
						break;

					if (szLine[0] == '.')
					{
						std::string strName(szLine + 4, strlen(szLine + 4) - 2);
						vSoundNames[i] = strName;
					}
					else
					{
						std::string strName(szLine, strlen(szLine) - 2);
						vSoundNames[i] = strName;
					}
					i++;

				}
			}
			else
			{
				for (int i = 0; i < sounds; i++)
				{
					std::string temp = "sound" + std::to_string(i) + ".vag";
					vSoundNames[i] = temp;
				}

			}


			// update progress bar
			SendMessage(*progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, sounds));

			// generate cfg

			if (!cfgPath.empty())
			{
				std::ofstream pBuild(cfgPath, std::ofstream::binary);
				if (!pBuild)
				{
					MessageBoxA(0, "Failed to open CFG for writing!", 0, 0);
					return false;
				}
				pBuild << "; build file created by rsfx\n";

				for (int i = 0; i < sounds; i++)
					pBuild << vSoundNames[i] << " " << std::endl;

				pBuild.close();
			}


			// extract
			for (int i = 0; i < sounds; i++)
			{

				sprintf(progressBuffer, "%d/%d", i, sounds - 1);
				SetWindowText(*filename, vSoundNames[i].c_str());
				SetWindowText(*numbers, progressBuffer);
				SendMessage(*progressBar, PBM_STEPIT, 0, 0);


				if (checkSlash(vSoundNames[i]))
					std::experimental::filesystem::create_directories(splitString(vSoundNames[i], false));

				std::ofstream oFile(vSoundNames[i], std::ofstream::binary);

				vag_header vag;
				vag.header = 'VAGp';
				vag.version = 4;
				vag.freq = vSoundTable[i].freq;
				vag.dataSize = vSoundTable[i].size;
				std::string temp = "sound" + std::to_string(i);
				sprintf(vag.name, temp.c_str());

				changeEndINT(&vag.version);
				changeEndINT(&vag.dataSize);
				changeEndINT(&vag.freq);
				changeEndINT(&vag.header);
				oFile.write((char*)&vag, sizeof(vag_header));

				pFile.seekg(vSoundTable[i].offset, pFile.beg);
				std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(vSoundTable[i].size);
				pFile.read(dataBuff.get(), vSoundTable[i].size);
				oFile.write(dataBuff.get(), vSoundTable[i].size);
				oFile.close();

			}

			pFile.close();
			pTable.close();

			return true;
		}
	}
	if (mode == MODE_CREATE)
	{
		std::unique_ptr<sdt_entry_ps2_gta[]> vSoundTable;
		std::unique_ptr<std::string[]> vSoundNames;

		FILE* pRebuild = fopen(cfgPath.c_str(), "rb");

		if (!pRebuild)
		{
			MessageBoxA(0, "Failed to open CFG file!", 0, MB_ICONWARNING);
			return false;
		}

		if (pRebuild)
		{
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
				vSoundTable = std::make_unique<sdt_entry_ps2_gta[]>(sounds);
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
						sscanf(szLine, "%s", &tempStr);
						std::string name(tempStr, strlen(tempStr));
						vSoundNames[i] = name;
						i++;
					}
				}

				std::ofstream oSFXRaw(rawPath, std::ofstream::binary);

				int baseOffset = 0;

				// update progress bar
				SendMessage(*progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, sounds));

				std::experimental::filesystem::current_path(
					std::experimental::filesystem::system_complete(std::experimental::filesystem::path(outPath)));


				for (int i = 0; i < sounds; i++)
				{
					std::ifstream pFile(vSoundNames[i], std::ifstream::binary);

					sprintf(progressBuffer, "%d/%d", i, sounds - 1);
					SetWindowText(*filename, vSoundNames[i].c_str());
					SetWindowText(*numbers, progressBuffer);
					SendMessage(*progressBar, PBM_STEPIT, 0, 0);

					if (!pFile)
					{
						char buffer[256];
						sprintf(buffer, "Could not open %s!", vSoundNames[i].c_str());
						MessageBoxA(0, buffer, 0, 0);
						SendMessage(*progressBar, PBM_SETSTATE, PBST_ERROR, 0);
						return false;
					}

					if (pFile)
					{
						vag_header vag;
						pFile.read((char*)&vag, sizeof(vag_header));

						if (!(vag.header == 'pGAV'))
						{
							char buffer[256];
							sprintf(buffer, "Invalid sound format: %s", vSoundNames[i].c_str());
							MessageBoxA(0, buffer, 0, 0);
							SendMessage(*progressBar, PBM_SETSTATE, PBST_ERROR, 0);
							return false;
						}

						int dataSize = (int)getSizeToEnd(pFile);

						std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);

						pFile.read(dataBuff.get(), dataSize);
						oSFXRaw.write(dataBuff.get(), dataSize);

						vSoundTable[i].freq = vag.freq;
						vSoundTable[i].offset = baseOffset;
						vSoundTable[i].size = dataSize;

						changeEndINT(&vSoundTable[i].freq);

						baseOffset += dataSize;


					}
					pFile.close();
				}
				// build sdt

				std::ofstream oSDT(sdtPath, std::ofstream::binary);
				SendMessage(*progressBar, PBM_SETPOS, 0, 0);
				for (int i = 0; i < sounds; i++)
				{
					sprintf(progressBuffer, "%d/%d", i, sounds - 1);
					SetWindowText(*filename, "Building SDT");
					SetWindowText(*numbers, progressBuffer);
					SendMessage(*progressBar, PBM_STEPIT, 0, 0);
					oSDT.write((char*)&vSoundTable[i], sizeof(sdt_entry_ps2_gta));
				}

				return true;
			}
		}
		fclose(pRebuild);
	}
	return false;
}

bool SFXManager::ProcessStoriesPS2()
{
	if (mode == MODE_EXTRACT)
	{
		std::unique_ptr<sdt_entry_ps2_gta[]> vSoundTable;
		std::unique_ptr<std::string[]> vSoundNames;
		std::ifstream pFile(rawPath, std::ifstream::binary);

		if (!std::experimental::filesystem::exists(outPath))
			std::experimental::filesystem::create_directory(outPath);


		if (!pFile)
		{
			MessageBoxA(0, "Failed to open RAW file!", 0, MB_ICONWARNING);
			return false;
		}

		if (pFile)
		{
			std::ifstream pTable(sdtPath, std::ifstream::binary);
			if (!pTable)
			{
				MessageBoxA(0, "Failed to open SDT file!", 0, MB_ICONWARNING);
				return false;
			}

			int sounds = getSizeToEnd(pTable) / sizeof(sdt_entry_ps2_gta);
			vSoundTable = std::make_unique<sdt_entry_ps2_gta[]>(sounds);
			vSoundNames = std::make_unique<std::string[]>(sounds);

			for (int i = 0; i < sounds; i++)
				pTable.read((char*)&vSoundTable[i], sizeof(sdt_entry_ps2_gta));

			std::experimental::filesystem::current_path(
				std::experimental::filesystem::system_complete(std::experimental::filesystem::path(outPath)));

			if (!lstPath.empty())
			{
				FILE* pList = fopen(lstPath.c_str(), "rb");
				char szLine[1024];

				int i = 0;

				while (fgets(szLine, sizeof(szLine), pList))
				{
					if (szLine[0] == ';' || szLine[0] == '-' || szLine[0] == '\n' || szLine[0] == ' ')
						continue;

					if (szLine[0] == 'q')
						break;

					if (szLine[0] == '.')
					{
						std::string strName(szLine + 4, strlen(szLine + 4) - 2);
						vSoundNames[i] = strName;
					}
					else
					{
						std::string strName(szLine, strlen(szLine) - 2);
						vSoundNames[i] = strName;
					}
					i++;

				}
			}
			else
			{
				for (int i = 0; i < sounds; i++)
				{
					pFile.seekg(vSoundTable[i].offset, pFile.beg);
					vag_header vag;
					pFile.read((char*)&vag, sizeof(vag_header));
					std::string vagName = vag.name;
					std::string temp = std::to_string(i) + "_";
					if (!(checkSlash(vagName)))
						temp += vag.name;
					temp += ".vag";
					vSoundNames[i] = temp;
				}

			}

			pFile.seekg(0, pFile.beg);
			// update progress bar
			SendMessage(*progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, sounds));

			// generate cfg
			std::ofstream pBuild;

			if (!cfgPath.empty())
			{
				std::ofstream pBuild(cfgPath, std::ofstream::binary);
				if (!pBuild)
				{
					MessageBoxA(0, "Failed to open CFG for writing!", 0, 0);
					return false;
				}
				pBuild << "; build file created by rsfx\n";

				for (int i = 0; i < sounds; i++)
					pBuild << vSoundNames[i] << " " << std::endl;

				pBuild.close();
			}


			// extract
			for (int i = 0; i < sounds; i++)
			{

				sprintf(progressBuffer, "%d/%d", i, sounds - 1);
				SetWindowText(*filename, vSoundNames[i].c_str());
				SetWindowText(*numbers, progressBuffer);
				SendMessage(*progressBar, PBM_STEPIT, 0, 0);


				if (checkSlash(vSoundNames[i]))
					std::experimental::filesystem::create_directories(splitString(vSoundNames[i], false));

				std::ofstream oFile(vSoundNames[i], std::ofstream::binary);

				pFile.seekg(vSoundTable[i].offset, pFile.beg);
				std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(vSoundTable[i].size);
				pFile.read(dataBuff.get(), vSoundTable[i].size);
				oFile.write(dataBuff.get(), vSoundTable[i].size);
				oFile.close();

			}

			pFile.close();
			pTable.close();

			return true;
		}
	}
	if (mode == MODE_CREATE)
	{
		std::unique_ptr<sdt_entry_ps2_gta[]> vSoundTable;
		std::unique_ptr<std::string[]> vSoundNames;

		FILE* pRebuild = fopen(cfgPath.c_str(), "rb");

		if (!pRebuild)
		{
			MessageBoxA(0, "Failed to open CFG file!", 0, MB_ICONWARNING);
			return false;
		}

		if (pRebuild)
		{
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
				vSoundTable = std::make_unique<sdt_entry_ps2_gta[]>(sounds);
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
						sscanf(szLine, "%s", &tempStr);
						std::string name(tempStr, strlen(tempStr));
						vSoundNames[i] = name;
						i++;
					}
				}

				std::ofstream oSFXRaw(rawPath, std::ofstream::binary);

				int baseOffset = 0;

				// update progress bar
				SendMessage(*progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, sounds));

				std::experimental::filesystem::current_path(
					std::experimental::filesystem::system_complete(std::experimental::filesystem::path(outPath)));


				for (int i = 0; i < sounds; i++)
				{
					std::ifstream pFile(vSoundNames[i], std::ifstream::binary);

					sprintf(progressBuffer, "%d/%d", i, sounds - 1);
					SetWindowText(*filename, vSoundNames[i].c_str());
					SetWindowText(*numbers, progressBuffer);
					SendMessage(*progressBar, PBM_STEPIT, 0, 0);

					if (!pFile)
					{
						char buffer[256];
						sprintf(buffer, "Could not open %s!", vSoundNames[i].c_str());
						MessageBoxA(0, buffer, 0, 0);
						SendMessage(*progressBar, PBM_SETSTATE, PBST_ERROR, 0);
						return false;
					}

					if (pFile)
					{
						vag_header vag;
						pFile.read((char*)&vag, sizeof(vag_header));

						if (!(vag.header == 'pGAV'))
						{
							char buffer[256];
							sprintf(buffer, "Invalid sound format: %s", vSoundNames[i].c_str());
							MessageBoxA(0, buffer, 0, 0);
							SendMessage(*progressBar, PBM_SETSTATE, PBST_ERROR, 0);
							return false;
						}

						pFile.seekg(0, pFile.beg);
						int dataSize = (int)getSizeToEnd(pFile);

						std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);

						pFile.read(dataBuff.get(), dataSize);
						oSFXRaw.write(dataBuff.get(), dataSize);

						vSoundTable[i].freq = vag.freq;
						vSoundTable[i].offset = baseOffset;
						vSoundTable[i].size = dataSize;

						changeEndINT(&vSoundTable[i].freq);

						baseOffset += dataSize;


					}
					pFile.close();
				}
				// build sdt

				std::ofstream oSDT(sdtPath, std::ofstream::binary);
				SendMessage(*progressBar, PBM_SETPOS, 0, 0);
				for (int i = 0; i < sounds; i++)
				{
					sprintf(progressBuffer, "%d/%d", i, sounds - 1);
					SetWindowText(*filename, "Building SDT");
					SetWindowText(*numbers, progressBuffer);
					SendMessage(*progressBar, PBM_STEPIT, 0, 0);
					oSDT.write((char*)&vSoundTable[i], sizeof(sdt_entry_ps2_gta));
				}

				return true;
			}
		}
		fclose(pRebuild);
	}
	return false;
}

bool SFXManager::ProcessStoriesPSP()
{
	if (mode == MODE_EXTRACT)
	{
		std::ifstream pFile(rawPath, std::ifstream::binary);
		std::vector<std::string> vVagNames;
		std::vector<int> vVagPositions;
		std::vector<vag_header> vVags;
 
		if (!std::experimental::filesystem::exists(outPath))
			std::experimental::filesystem::create_directory(outPath);


		if (!pFile)
		{
			MessageBoxA(0, "Failed to open RAW file!", 0, MB_ICONWARNING);
			return false;
		}


		if (pFile)
		{
			std::experimental::filesystem::current_path(
				std::experimental::filesystem::system_complete(std::experimental::filesystem::path(outPath)));
			
			SetWindowText(*filename, "Analyzing RAW");
			SetWindowText(*numbers, "Please wait...");
			int i = 0;
			while (!pFile.eof())
			{
				char temp;
				pFile.read((char*)&temp, sizeof(char));
				if (temp == 'V')
				{
					pFile.read((char*)&temp, sizeof(char));
					if (temp == 'A')
					{
						pFile.read((char*)&temp, sizeof(char));
						if (temp == 'G')
						{
							pFile.seekg(-3, pFile.cur);
							vag_header vag;
							pFile.read((char*)&vag, sizeof(vag_header));
							if (vag.header == 'pGAV')
							{
								int cur_pos = (int)pFile.tellg() - sizeof(vag_header);
								std::string temp = vag.name;
								std::string name;
								name += std::to_string(i) + "_";
								if (!checkSlash(temp))
								{
									name += vag.name;
								}
								name += ".vag";
								changeEndINT(&vag.dataSize);
								vVagNames.push_back(name);
								vVagPositions.push_back(cur_pos);
								vVags.push_back(vag);
								i++;
							}
						}
					}
				}
			}

			int sounds = i;
			pFile.clear();
			pFile.seekg(0, pFile.beg);
			// update progress bar
			SendMessage(*progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, sounds));

			// generate cfg

			if (!cfgPath.empty())
			{
				std::ofstream pBuild(cfgPath, std::ofstream::binary);
				if (!pBuild)
				{
					MessageBoxA(0, "Failed to open CFG for writing!", 0, 0);
					return false;
				}
				pBuild << "; build file created by rsfx\n";

				for (int i = 0; i < sounds; i++)
					pBuild << vVagNames[i] << " " << std::endl;

				pBuild.close();
			}


			// extract
			for (int i = 0; i < sounds; i++)
			{

				sprintf(progressBuffer, "%d/%d", i, sounds - 1);
				SetWindowText(*filename, vVagNames[i].c_str());
				SetWindowText(*numbers, progressBuffer);
				SendMessage(*progressBar, PBM_STEPIT, 0, 0);

				int pos = vVagPositions[i];

				if (checkSlash(vVagNames[i]))
					std::experimental::filesystem::create_directories(splitString(vVagNames[i], false));

				std::ofstream oFile(vVagNames[i], std::ofstream::binary);

				pFile.seekg(pos, pFile.beg);

				int dataSize = sizeof(vag_header) - 0x10 + vVags[i].dataSize;

				std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);
				pFile.read(dataBuff.get(), dataSize);
				oFile.write(dataBuff.get(), dataSize);
				oFile.close();

			}

			pFile.close();

			return true;
		}
	}
	if (mode == MODE_CREATE)
	{
		std::unique_ptr<std::string[]> vSoundNames;

		FILE* pRebuild = fopen(cfgPath.c_str(), "rb");

		if (!pRebuild)
		{
			MessageBoxA(0, "Failed to open CFG file!", 0, MB_ICONWARNING);
			return false;
		}

		if (pRebuild)
		{
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
						sscanf(szLine, "%s", &tempStr);
						std::string name(tempStr, strlen(tempStr));
						vSoundNames[i] = name;
						i++;
					}
				}

				std::ofstream oSFXRaw(rawPath, std::ofstream::binary);


				// update progress bar
				SendMessage(*progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, sounds));

				std::experimental::filesystem::current_path(
					std::experimental::filesystem::system_complete(std::experimental::filesystem::path(outPath)));


				for (int i = 0; i < sounds; i++)
				{
					std::ifstream pFile(vSoundNames[i], std::ifstream::binary);

					sprintf(progressBuffer, "%d/%d", i, sounds - 1);
					SetWindowText(*filename, vSoundNames[i].c_str());
					SetWindowText(*numbers, progressBuffer);
					SendMessage(*progressBar, PBM_STEPIT, 0, 0);

					if (!pFile)
					{
						char buffer[256];
						sprintf(buffer, "Could not open %s!", vSoundNames[i].c_str());
						MessageBoxA(0, buffer, 0, 0);
						SendMessage(*progressBar, PBM_SETSTATE, PBST_ERROR, 0);
						return false;
					}

					if (pFile)
					{
						vag_header vag;
						pFile.read((char*)&vag, sizeof(vag_header));

						if (!(vag.header == 'pGAV'))
						{
							char buffer[256];
							sprintf(buffer, "Invalid sound format: %s", vSoundNames[i].c_str());
							MessageBoxA(0, buffer, 0, 0);
							SendMessage(*progressBar, PBM_SETSTATE, PBST_ERROR, 0);
							return false;
						}

						pFile.seekg(0, pFile.beg);
						int dataSize = (int)getSizeToEnd(pFile);

						std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);

						pFile.read(dataBuff.get(), dataSize);
						oSFXRaw.write(dataBuff.get(), dataSize);

					}
					pFile.close();
				}

				return true;
			}
		}
		fclose(pRebuild);
	}
	return false;
}

const char * SetPathFromButton(const char * filter, const char* ext, HWND hWnd)
{
	char szBuffer[MAX_PATH] = {};
	OPENFILENAME ofn = {};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = (LPCSTR)filter;
	ofn.lpstrFile = (LPSTR)szBuffer;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = (LPCSTR)ext;
	if (GetOpenFileName(&ofn))
	{
		return szBuffer;
	}
	else
		return nullptr;
}


const char * SetSavePathFromButton(const char * filter, const char* ext, HWND hWnd)
{
	char szBuffer[MAX_PATH] = {};
	OPENFILENAME ofn = {};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = (LPCSTR)filter;
	ofn.lpstrFile = (LPSTR)szBuffer;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = (LPCSTR)ext;
	if (GetSaveFileName(&ofn))
	{
		return szBuffer;
	}
	else
		return nullptr;
}

const char * SetFolderFromButton(HWND hWnd)
{
	char szBuffer[MAX_PATH];

	BROWSEINFO bi = {};
	bi.lpszTitle = ("Select Folder");
	bi.hwndOwner = hWnd;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	LPITEMIDLIST idl = SHBrowseForFolder(&bi);

	if (idl)
	{
		SHGetPathFromIDList(idl, szBuffer);
		return szBuffer;
	}
		
	return nullptr;
}
