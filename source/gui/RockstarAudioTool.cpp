// RockstarAudioTool.cpp : Defines the entry point for the application.
//

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "framework.h"
#include "RockstarAudioTool.h"
#include <CommCtrl.h>
#include "SFXManager.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                               
WCHAR szTitle[MAX_LOADSTRING];                 
WCHAR szWindowClass[MAX_LOADSTRING];           


INT_PTR CALLBACK    Init(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AboutBox(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)

{
	DialogBox(hInst, MAKEINTRESOURCE(RSFX_DIALOG_MAIN), 0, Init);
}





// Message handler for about box.
INT_PTR CALLBACK Init(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND ProgressBar = GetDlgItem(hDlg, IDC_PROGRESS1);
	HWND ProgressNum = GetDlgItem(hDlg, ProgressNumbers);
	HWND ProgressName = GetDlgItem(hDlg, ProgressFile);

	HWND SDTPathBox = GetDlgItem(hDlg, IDC_EDIT1);
	HWND RAWPathBox = GetDlgItem(hDlg, IDC_EDIT2);
	HWND CFGPathBox = GetDlgItem(hDlg, IDC_EDIT4);
	HWND LSTPathBox = GetDlgItem(hDlg, IDC_EDIT3);
	HWND OutputBox = GetDlgItem(hDlg, IDC_EDIT5);
	HWND ExtractRadio = GetDlgItem(hDlg, mode_extract);
	HWND GameDropList = GetDlgItem(hDlg, GameList);
	LRESULT GameResult, PlatformResult;
	HWND PlatformDropList = GetDlgItem(hDlg, PlatformList);
	int iGameIdentifier = 0, iPlatformIdentifier = 0, iWorkMode = 0;

	const char* szGameNames[SUPPORTED_GAMES] = { "Grand Theft Auto 2","Grand Theft Auto III/Vice City","Grand Theft Auto Stories (LCS/VCS)","Manhunt" };
	const char* szPlatforms[SUPPORTED_PLATFORMS] = { "PC","Playstation 2","Playstation Portable","XBOX" };


    switch (message)
    {
    case WM_INITDIALOG:
		for (int i = 0; i < SUPPORTED_GAMES; i++)
			SendMessage(GameDropList, CB_ADDSTRING, 0, (LPARAM)szGameNames[i]);
		for (int i = 0; i < SUPPORTED_PLATFORMS; i++)
			SendMessage(PlatformDropList, CB_ADDSTRING, 0, (LPARAM)szPlatforms[i]);

		SendMessageA(GameDropList, CB_SETCURSEL, 0, 0);
		SendMessageA(PlatformDropList, CB_SETCURSEL, 0, 0);
		SendMessage(ExtractRadio, BM_SETCHECK, BST_CHECKED, 0);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
		if (!(wParam == IDCANCEL))
		{
			// process games
			GameResult = SendMessage(GameDropList, CB_GETCURSEL, 0, 0);
			iGameIdentifier = GameResult;
			PlatformResult = SendMessage(PlatformDropList, CB_GETCURSEL, 0, 0);
			iPlatformIdentifier = PlatformResult;
			
			if (IsDlgButtonChecked(hDlg, mode_extract))
				iWorkMode = MODE_EXTRACT;

			if (IsDlgButtonChecked(hDlg, mode_build))
				iWorkMode = MODE_CREATE;

			switch (iGameIdentifier)
			{
			case GAME_GTA2:
				SendMessageA(PlatformDropList, CB_SETCURSEL, PLATFORM_PC, 0);
				break;
			case GAME_MANHUNT:
			case GAME_GTA3_VC:
				if (iPlatformIdentifier == PLATFORM_PSP)
					SendMessageA(PlatformDropList, CB_SETCURSEL, PLATFORM_PC, 0);
				break;
			case GAME_STORIES:
				if (iPlatformIdentifier == PLATFORM_PC || iPlatformIdentifier == PLATFORM_XBOX)
					SendMessageA(PlatformDropList, CB_SETCURSEL, PLATFORM_PS2, 0);
				break;
			}


			if (iWorkMode == MODE_EXTRACT)
			{
				SetWindowText(GetDlgItem(hDlg, FolderText), "Output Folder");
			}
			else
			{
				SetWindowText(GetDlgItem(hDlg, FolderText), "Input Folder");
			}

			if (wParam == ButtonClear)
			{
				SetWindowTextA(SDTPathBox, 0);
				SetWindowTextA(RAWPathBox, 0);
				SetWindowTextA(CFGPathBox, 0);
				SetWindowTextA(LSTPathBox, 0);
				SetWindowTextA(OutputBox,  0);
			}

			if (iWorkMode == MODE_EXTRACT)
			{
				if (wParam == GetSDTPath)
					SetWindowTextA(SDTPathBox, SetPathFromButton("Sound Data Table (*.sdt)\0*.sdt\0All Files (*.*)\0*.*\0", "sdt", hDlg));
				if (wParam == GetRAWPath)
					SetWindowTextA(RAWPathBox, SetPathFromButton("Sound (*.raw)\0*.raw\0All Files (*.*)\0*.*\0", "raw", hDlg));
				if (wParam == GetCFGPath)
					SetWindowTextA(CFGPathBox, SetSavePathFromButton("Configuration (*.cfg)\0*.cfg\0All Files (*.*)\0*.*\0", "cfg", hDlg));
				if (wParam == GetNamesPath)
					SetWindowTextA(LSTPathBox, SetPathFromButton("List (*.lst)(*.sdf)\0*.lst;*.sdf\0All Files (*.*)\0*.*\0", "lst", hDlg));
				if (wParam == GetFolder)
					SetWindowTextA(OutputBox, SetFolderFromButton(hDlg));
				EnableWindow(GetDlgItem(hDlg, GetNamesPath), 1);
				EnableWindow(LSTPathBox, 1);

				if (iGameIdentifier == GAME_STORIES && iPlatformIdentifier == PLATFORM_PSP)
				{
					EnableWindow(GetDlgItem(hDlg, GetSDTPath), 0);
					EnableWindow(SDTPathBox, 0);
				}
				else
				{
					EnableWindow(GetDlgItem(hDlg, GetSDTPath), 1);
					EnableWindow(SDTPathBox, 1);
				}
			}
			else
			{
				if (wParam == GetSDTPath)
					SetWindowTextA(SDTPathBox, SetSavePathFromButton("Sound Data Table (*.sdt)\0*.sdt\0All Files (*.*)\0*.*\0", "sdt", hDlg));
				if (wParam == GetRAWPath)
					SetWindowTextA(RAWPathBox, SetSavePathFromButton("Sound (*.raw)\0*.raw\0All Files (*.*)\0*.*\0", "raw", hDlg));
				if (wParam == GetCFGPath)
					SetWindowTextA(CFGPathBox, SetPathFromButton("Configuration (*.cfg)\0*.cfg\0All Files (*.*)\0*.*\0", "cfg", hDlg));
				if (wParam == GetNamesPath)
					SetWindowTextA(LSTPathBox, SetPathFromButton("List (*.lst)(*.sdf)\0*.lst;*.sdf\0All Files (*.*)\0*.*\0", "lst", hDlg));
				if (wParam == GetFolder)
					SetWindowTextA(OutputBox, SetFolderFromButton(hDlg));
				EnableWindow(GetDlgItem(hDlg, GetNamesPath), 0);
				EnableWindow(LSTPathBox, 0);
				if (iGameIdentifier == GAME_STORIES && iPlatformIdentifier == PLATFORM_PSP)
				{
					EnableWindow(GetDlgItem(hDlg, GetSDTPath), 0);
					EnableWindow(SDTPathBox, 0);
				}
				else
				{
					EnableWindow(GetDlgItem(hDlg, GetSDTPath), 1);
					EnableWindow(SDTPathBox, 1);
				}
			}


			if (wParam == ButtonGO)
			{
				if (GetWindowTextLength(SDTPathBox) == 0 && !(iGameIdentifier == GAME_STORIES && iPlatformIdentifier == PLATFORM_PSP))
				{
					MessageBoxA(0, "No SDT path specified!", 0, 0);
					break;
				}
				if (GetWindowTextLength(RAWPathBox) == 0)
				{
					MessageBoxA(0, "No RAW path specified!", 0, 0);
					break;
				}
				if (GetWindowTextLength(OutputBox) == 0)
				{
					MessageBoxA(0, "No output path specified!", 0, 0);
					break;
				}

				if (iWorkMode == MODE_CREATE)
				{
					if (GetWindowTextLength(CFGPathBox) == 0)
					{
						MessageBoxA(0, "No CFG path specified!", 0, 0);
						break;
					}
				}

				char szRawPath[MAX_PATH] = {}, szSDTPath[MAX_PATH] = {}, szLstPath[MAX_PATH] = {}, szOutPath[MAX_PATH] = {}, szCfgPath[MAX_PATH] = {};
				// get raw
				GetWindowText(RAWPathBox, (LPSTR)szRawPath, GetWindowTextLength(RAWPathBox) + 1);
				// get sdt
				GetWindowText(SDTPathBox, (LPSTR)szSDTPath, GetWindowTextLength(SDTPathBox) + 1);
				// get lst
				if (GetWindowTextLength(LSTPathBox) > 0)
			    	GetWindowText(LSTPathBox, (LPSTR)szLstPath, GetWindowTextLength(LSTPathBox) + 1);
				// get cfg
				if (GetWindowTextLength(CFGPathBox) > 0)
			    	GetWindowText(CFGPathBox, (LPSTR)szCfgPath, GetWindowTextLength(CFGPathBox) + 1);
				// get out
				GetWindowText(OutputBox, (LPSTR)szOutPath, GetWindowTextLength(OutputBox) + 1);
				SFXManager* SFX = new SFXManager(szRawPath, szSDTPath, szLstPath, szCfgPath, szOutPath, iGameIdentifier, iPlatformIdentifier, iWorkMode);

				SetWindowText(GetDlgItem(hDlg, OperationStatus), (LPCSTR)SFX->GetOperationMode());
				SFX->AttachProgressBar(&ProgressBar);
				SFX->AttachNumbersText(&ProgressNum);
				SFX->AttachFilenameText(&ProgressName);

				if (SFX->Process())
				{
					MessageBoxA(0, "Finished!", "Information", MB_ICONINFORMATION);
					SetWindowText(GetDlgItem(hDlg, OperationStatus), "Idle");
				}
				else
				{
					SetWindowText(GetDlgItem(hDlg, OperationStatus), "Idle");
				}
			}
				
			if (wParam == IDM_ABOUT)
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), 0, AboutBox);
		}


        if (wParam == IDCANCEL || wParam == IDM_EXIT)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


INT_PTR CALLBACK AboutBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{


	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (wParam == IDCANCEL || wParam== IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}