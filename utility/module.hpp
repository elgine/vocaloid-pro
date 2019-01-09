#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
using namespace std;

static wchar_t origin_dll_path[512] = L"undefined";

HMODULE GetSelfModuleHandle() {
	MEMORY_BASIC_INFORMATION mbi;
	return ((::VirtualQuery(GetSelfModuleHandle, &mbi, sizeof(mbi)) != 0)
		? (HMODULE)mbi.AllocationBase : nullptr);
}

void SetCurrentModulePathAsDllPath() {
	if (lstrcmpW(origin_dll_path, L"undefined") == 0) {
		GetDllDirectory(512, origin_dll_path);
	}	
	GetModuleFileName(GetSelfModuleHandle(), origin_dll_path, 512);
	SetDllDirectory(origin_dll_path);
}

void ResetDllPath() {
	if (lstrcmpW(origin_dll_path, L"undefined") != 0)
		SetDllDirectory(origin_dll_path);
}