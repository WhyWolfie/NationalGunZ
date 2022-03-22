#include "../Header/Acedia.h"
#include "../Header/md5wrapper.h"
#include "../MemoryManager/MemoryManagement.h"
md5wrapper md5;
FILE *logs;



///////////////////////////////////////////////////////////////////////////////////////////////////////
#define PROGRAMBLACKLIST                "Inject", "x1nject", "InjeX", "AC Tool", "Trainer", "Train3r", "AutoIt v3", "AutoIt"//
///////////////////////////////////////////////////////////////////////////////////////////////////////

HMODULE hBaseAddress;
HANDLE hProcess;

bool ProgramDetected = false;
unsigned long old2Protect;

BOOL CALLBACK EnumWindowsHandler(HWND hwnd, bool* retVal){
	char title[0x1000];
	GetWindowText(hwnd, title, _countof(title));
	static const char* titleBlacklist[] = { PROGRAMBLACKLIST };
	for(size_t i = 0; i < _countof(titleBlacklist); ++i){
		if(StrStrI(title, titleBlacklist[i])){
			*retVal = true;
			return FALSE;
		}
		return TRUE;
	}
	return TRUE;
}
bool IsAddressHooked(unsigned long address){
	BYTE* offsetValue = (BYTE*)address;
	return (*offsetValue == 0xE8 || *offsetValue == 0xE9 || *offsetValue == 0x7E || *offsetValue == 0x74 || *offsetValue == 0xFF);
}
int SetDebugPriv(){
    HANDLE TokenHandle;
    LUID lpLuid;
    TOKEN_PRIVILEGES NewState;
    if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &TokenHandle)){
        return 0;
    }
    if(!LookupPrivilegeValueA(NULL, "SeDebugPrivilege" , &lpLuid)){
        CloseHandle(TokenHandle);
        return 0;
    }
    NewState.PrivilegeCount = 1;
    NewState.Privileges[0].Luid = lpLuid;
    NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if(!AdjustTokenPrivileges(TokenHandle, FALSE, &NewState, sizeof(NewState), NULL, NULL)){
        CloseHandle(TokenHandle);
        return 0;
    }
    CloseHandle(TokenHandle);
    return 1;
}
void HACKING(){
	DWORD getTickCount = (DWORD)GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetTickCount");
	DWORD queryPerformanceCounter = (DWORD)GetProcAddress(GetModuleHandleA("kernel32.dll"), "QueryPerformanceCounter");
	  fopen_s(&logs, "Gunz.ini","a");
	  fputs("Anti-Hack Anti-Hacking loop started.\n",logs);
	  fclose(logs);
	while(1){
	  EnumWindows((WNDENUMPROC)EnumWindowsHandler, (LPARAM)(&ProgramDetected));
	  if(IsAddressHooked(getTickCount) || IsAddressHooked(queryPerformanceCounter)){
	  fopen_s(&logs, "Gunz.ini","a");
	  fputs("Anti-Hack Underclock Hack Detected!\n",logs);
	  fclose(logs);
	  ExitProcess(0);
	  fopen_s(&logs, "Gunz.ini","a");
	  fputs("Anti-Hack DLL Hack Detected!\n",logs);
	  fclose(logs);
	  ExitProcess(0);
	  }else if(ProgramDetected == true){
	  fopen_s(&logs, "Gunz.ini","a");
	  fputs("Anti-Hack 3RD Party Hack Detected!\n",logs);
	  fclose(logs);
	  ExitProcess(0);
	  }
	Sleep(100);
  }
}
//MD5 checker
void MD5(){
	if(_stricmp(md5.getHashFromFile("fmod.dll").c_str(), "F51A7DD4D040A9C079CF64D36F569673")){
	  fopen_s(&logs, "Gunz.ini","a");
	  fputs("Anti-Hack Fmod.dll MD5 is wrong!\n",logs);
	  fclose(logs);
		ExitProcess(0);
	}else if(_stricmp(md5.getHashFromFile("system.mrs").c_str(), "2DEA3E13520677C27A2B3CF989C6CD84")){
	  fopen_s(&logs, "Gunz.ini","a");
	  fputs("Anti-Hack System.mrs MD5 is wrong!\n",logs);
	  fclose(logs);
		ExitProcess(0);
	}
}
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID){
	if (dwReason == DLL_PROCESS_ATTACH){
	  fopen_s(&logs, "Gunz.ini","w");
	  fputs("Anti-Hack Has been injected!\n",logs);
	  fclose(logs);
	  DisableThreadLibraryCalls(hModule);
	  //remove("d3d9.dll");
	  //remove("msvcr71.dll"); remove("msvcp71.dll");
	if(SetDebugPriv() != 0){
	  fopen_s(&logs, "Gunz.ini","a");
	  fputs("Anti-Hack Debug priviledge available.\n",logs);
	  fclose(logs);
	  fopen_s(&logs, "Gunz.ini","a");
	  fputs("Anti-Hack Thread creation start.\n",logs);
	  fclose(logs);
	  	CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)&MD5,NULL,0,NULL);
	  	CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)&HACKING,NULL,0,NULL);
	  fopen_s(&logs, "Gunz.ini","a");
	  fputs("Anti-Hack Thread creation completed.\n",logs);
	  fclose(logs);
	}else{
	  fopen_s(&logs, "Gunz.ini","a");
	  fputs("Anti-Hack Debug priviledge failed!\n",logs);
	  fclose(logs);
      ExitProcess(0);
	}} 
  return TRUE;
}