#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "Urlmon.h"
#include "LoadLibraryR.h"
#include <ctype.h>
#define ID_EDIT 1
#define ID_BUTTON 2
#pragma comment(lib,"Advapi32.lib")
#define diewerror( e ) { printf( "Unhandled Exception: %d", e, GetLastError() ); break; }

const char szCN[] = "myWindowClass";

int rmnl(char *li)
{
    int nl = strlen(li) -1;
    if (li[nl] == '\n')
        li[nl] = '\0';
	return nl;
}

void encode(unsigned char *se, char *enc, char *tbo)
{
	for (; *se; se++) {
		if (tbo[*se]) sprintf(enc, "%c", tbo[*se]);
		else        sprintf(enc, "%%%02X", *se);
		while (*++enc);
	}
}

int id(char username[128], char password[128])
{
	HANDLE hMFile          = NULL;
	HANDLE hModule        = NULL;
	HANDLE hProcess       = NULL;
	HANDLE hTok         = NULL;
	LPVOID Buffer       = NULL;
	DWORD dwLength        = 0;
	DWORD dwBytesRead     = 0;
	DWORD dwProcessId     = 0;
	DWORD dwThreadId; 
	TOKEN_PRIVILEGES priv = {0};
	HWND hWindow = FindWindow(NULL, "Garry's Mod");
	HWND hWindow2 = FindWindow(NULL, "7 Days To Die");
	HWND hWindow3 = FindWindow(NULL, "ArmA 2 OA");
	HWND hWindow4 = FindWindow(NULL, "Call of Duty®: Modern Warfare® 3 Multiplayer");

	char buffer[256] = {0}; //buffer for hex
	char buffer2[256] = {0};
	char enc[1024]; //output for encoded password
	char* unptr = username; //pointers for un/pw
	char* passptr = enc; //pointer to encoded pass
	char url[1024]; //url
	int loader = 0; //DLL to req
	int i; //for loop
	char tmpconstructor[MAX_PATH];
	char smpconstructor[MAX_PATH];
	char* dllFile = smpconstructor;
	
	do
	{
		GetTempPath(sizeof(tmpconstructor), tmpconstructor);
		GetTempFileName(tmpconstructor, NULL, 0, smpconstructor);
		//Check for games
		if(hWindow)
		{
			dwThreadId = GetWindowThreadProcessId(hWindow, &dwProcessId); //gmod
			loader = 1;
		} else if(hWindow2)
		{	dwThreadId = GetWindowThreadProcessId(hWindow2, &dwProcessId); //7 days to die
			loader = 2;
		} else if(hWindow3)
		{	dwThreadId = GetWindowThreadProcessId(hWindow3, &dwProcessId); //arma2 oa
			loader = 3;
		} else if(hWindow4)
		{	dwThreadId = GetWindowThreadProcessId(hWindow4, &dwProcessId); //mw3
			loader = 4;
		} 
#ifndef DEBUG
		else
		return 0;
#endif
		
		//remove any newline
		rmnl(username);
		rmnl(password);

		//Encode password
		for (i = 0; i < 256; i++) 
		{
			buffer[i] = isalnum(i)||i == '~'||i == '-'||i == '.'||i == '_'
				? i : 0;
			buffer2[i] = isalnum(i)||i == '*'||i == '-'||i == '.'||i == '_'
				? i : (i == ' ') ? '+' : 0;
		}
		encode(password, enc, buffer);

		//Format URL
		sprintf(url, "http://toxichacks.com/loaderauth.php?username=%s&password=%s&loader=%i", unptr, &enc, loader);

		//Download file
		URLDownloadToFile(NULL, url, smpconstructor, 0, NULL);
		//DLL stuffs here-out
		hMFile = CreateFileA( dllFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if( hMFile == INVALID_HANDLE_VALUE )
			diewerror("");
			//diewerror( "Failed to open the DLL file" );

		dwLength = GetFileSize( hMFile, NULL );
		if( dwLength == INVALID_FILE_SIZE || dwLength == 0 )
			diewerror("");
			//diewerror( "Failed to get the DLL file size" );

		Buffer = HeapAlloc( GetProcessHeap(), 0, dwLength );
		if( !Buffer )
			diewerror("");
			//diewerror( "Failed to get the DLL file size" );

		if( ReadFile( hMFile, Buffer, dwLength, &dwBytesRead, NULL ) == FALSE )
			diewerror("");
			//diewerror( "Failed to alloc a buffer!" );

		if( OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hTok ) )
		{
			priv.PrivilegeCount           = 1;
			priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		
			if( LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &priv.Privileges[0].Luid ) )
				AdjustTokenPrivileges( hTok, FALSE, &priv, 0, NULL, NULL );

			CloseHandle( hTok );
		}

		hProcess = OpenProcess( PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, dwProcessId );
		if( !hProcess )
			diewerror("");
			//diewerror( "Failed to open the target process" );

		hModule = LoadRemoteLibraryR( hProcess, Buffer, dwLength, NULL );
		if( !hModule )
			diewerror("");
			//diewerror( "Failed to inject the DLL" );

		printf( "[+] Injected the '%s' DLL into process %d.", dllFile, dwProcessId );
		
		//WaitForSingleObject( hModule, -1 );

	} while( 0 );

	if( Buffer )
		HeapFree( GetProcessHeap(), 0, Buffer );

	if( hProcess )
		CloseHandle( hProcess );

	CloseHandle(hMFile);
	DeleteFile(dllFile);
	return loader;
}

LRESULT CALLBACK WndProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{  
	static HWND ufield;
	static HWND pfield;
	static HWND inj;
	int lenu = GetWindowTextLengthW(ufield) + 1;
	int lenp = GetWindowTextLengthW(pfield) + 1;
	char u[128];
	char p[128];
	static wchar_t *username = L"Username:";
	static wchar_t *password = L"Password: ";
	static wchar_t *filler = L" ";
	switch(msg)
    {
		case WM_CREATE:
			//Labels
			CreateWindowW(L"STATIC", filler, WS_CHILD | WS_VISIBLE | SS_LEFT, 0, 0, 300, 230, hWindow, (HMENU) 3, NULL, NULL);
			CreateWindowW(L"STATIC", username, WS_CHILD | WS_VISIBLE | SS_LEFT, 3, 3, 300, 230, hWindow, (HMENU) 4, NULL, NULL);
			CreateWindowW(L"STATIC", password, WS_CHILD | WS_VISIBLE | SS_LEFT, 3, 33, 300, 230, hWindow, (HMENU) 5, NULL, NULL);
			//Username/password text fields
			ufield = CreateWindowA("Edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 80, 2, 170, 25, hWindow, (HMENU) ID_EDIT, NULL, NULL);
			pfield = CreateWindowA("Edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 80, 30, 170, 25, hWindow, (HMENU) ID_EDIT, NULL, NULL);
			//Inject/Quit buttons
			// 30/117 /75
			inj = CreateWindowW(L"button", L"Inject", WS_VISIBLE | WS_CHILD, 1, 60, 249, 20, hWindow, (HMENU) 1, NULL, NULL);
			//CreateWindowW(L"button", L"Quit", WS_VISIBLE | WS_CHILD, 125, 60, 125, 20,  hWindow, (HMENU) 2, NULL, NULL);
			break;
		case WM_COMMAND:
			if (HIWORD(wParam) == BN_CLICKED)  
			{
				 GetWindowTextA(ufield, u, lenu);
				 GetWindowTextA(pfield, p, lenp);
				 //SetWindowTextA(ufield, p);
				 //SetWindowTextA(pfield, u);
				 SetWindowTextW(inj, L"Scanning for games...");
				 switch(id(u, p))
				 {
					case 0:
						SetWindowTextW(inj, L"No game open or login failed");
						Sleep(1000);
						break;
					case 1:
						SetWindowTextW(inj, L"Garry's Mod Injected");
						Sleep(1000);
						exit(0);
					case 2:
						SetWindowTextW(inj, L"7D2D Injected");
						Sleep(1000);
						exit(0);
					case 3:
						SetWindowTextW(inj, L"ArmA2 Injected");
						Sleep(1000);
						exit(0);
					case 4:
						SetWindowTextW(inj, L"MW3 Injected");
						Sleep(1000);
						exit(0);
				}
			}
			break;
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE) 
    			SendMessage(hWindow, WM_CLOSE, 0, 0);
			break;
        case WM_CLOSE:
            DestroyWindow(hWindow);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        default:
            return DefWindowProc(hWindow, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
	HWND hWindow;
    MSG message;
	WNDCLASSEX wclass;
    wclass.hInstance     = hInstance;
    wclass.cbSize        = sizeof(WNDCLASSEX);
    wclass.lpszClassName = szCN;
    wclass.lpfnWndProc   = WndProc;
    wclass.lpszMenuName  = NULL;
    wclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wclass.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
    wclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wclass.cbClsExtra    = 0;
    wclass.cbWndExtra    = 0;
    wclass.style         = 0;

    if(!RegisterClassEx(&wclass))
    {
        MessageBox(NULL, "WRF", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hWindow = CreateWindowEx(WS_EX_TOOLWINDOW, szCN, "Toxic Loader", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 270, 119,  NULL, NULL, hInstance, NULL);

    if(hWindow == NULL)
    {
        MessageBox(NULL, "WCF", "Error", MB_OK);
        return 0;
    }

    ShowWindow(hWindow, nCmdShow);
    UpdateWindow(hWindow);

    while(GetMessage(&message, NULL, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    return message.wParam;
}