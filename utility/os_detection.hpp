#pragma once
#include <Windows.h>
bool IsWin7(){
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&osvi);

	// win7的系统版本为NT6.1  
	if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId &&
		osvi.dwMajorVersion == 6 &&
		osvi.dwMinorVersion == 1){
		return true;
	}
	else{
		return false;
	}
}