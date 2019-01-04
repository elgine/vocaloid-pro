#pragma once
#include <Windows.h>

wchar_t* StringToWString(const char *str){
	//CP_UTF8
	DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	wchar_t *pwText = new wchar_t[dwNum];
	MultiByteToWideChar(CP_ACP, 0, str, -1, pwText, dwNum);
	return pwText;
}

char* WStringToString(const wchar_t *str){
	int len = WideCharToMultiByte(CP_ACP, 0, str, -1, 0, 0, 0, 0);
	char* output = new char[len + 2];
	WideCharToMultiByte(CP_ACP, 0, str, -1, output, len + 1, 0, 0);
	return output;
}