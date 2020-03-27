#pragma once
#include <string>
#include <Windows.h>

#define SUPPORTED_GAMES 6
#define SUPPORTED_PLATFORMS 4

enum eGames {
	GAME_GTA,
	GAME_GTA2,
	GAME_GTA3_VC,
	GAME_STORIES,
	GAME_MANHUNT,
	GAME_MH2
};

enum ePlatforms {
	PLATFORM_PC,
	PLATFORM_PS2,
	PLATFORM_PSP,
	PLATFORM_XBOX
};

enum eModes {
	MODE_EXTRACT,
	MODE_CREATE
};

enum eFSBVer {
	FSB_VER3 = 0xF3, FSB_VER4 = 0xF4
};

class SFXManager {
private:
	std::string rawPath, sdtPath, lstPath, cfgPath, outPath;
	int game, platform, mode;
	HWND* progressBar;
	HWND* numbers;
	HWND* filename;
	char  progressBuffer[256];
public:
	SFXManager();
	SFXManager(std::string r_path, std::string s_path, std::string l_path, std::string c_path,std::string o_path, int g, int p, int w);
	void AttachProgressBar(HWND* bar);
	void AttachFilenameText(HWND* txt);
	void AttachNumbersText(HWND* txt);
	void IncrementProgressBar(int value);
	const char* GetOperationMode();
	bool Process();
	bool ProcessGTA();
	bool ProcessGTA2();
	bool ProcessGTA3VC();
	bool ProcessGTA3VCXBOX();
	bool ProcessManhuntBuild();
	bool ProcessPS23VC();
	bool ProcessStoriesPS2();
	bool ProcessStoriesPSP();
	bool ProcessManhunt2PC();
	
};


const char* SetPathFromButton(const char* filter,const char* ext, HWND hWnd);
const char* SetSavePathFromButton(const char* filter, const char* ext, HWND hWnd);
const char* SetFolderFromButton(HWND hWnd);